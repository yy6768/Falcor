/***************************************************************************
 # Copyright (c) 2015-23, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#include "RealtimeDSPass.h"

namespace {
    const char kInputColor[] = "inputColor";
    const char kOutputColor[] = "outputColor";

    // Compute shader for Splat operation
    const char kSplatShader[] = R"(
        RWTexture2D<float4> gOutput;
        Texture2D<float4> gInput;
        Texture2D<float> gKernel;

        cbuffer CB {
            uint2 gImageSize;
            uint gKernelSize;
        };

        [numthreads(16, 16, 1)]
        void main(uint3 dispatchThreadId : SV_DispatchThreadID) {
            if (any(dispatchThreadId.xy >= gImageSize)) return;

            float4 total = float4(0, 0, 0, 0);
            int halfKernel = (int)(gKernelSize - 1) / 2;

            for (int i = 0; i < gKernelSize; i++) {
                for (int j = 0; j < gKernelSize; j++) {
                    int2 inputPos = int2(dispatchThreadId.xy) + int2(i - halfKernel, j - halfKernel);
                    if (any(inputPos < 0) || any(inputPos >= gImageSize)) continue;

                    float kernelValue = gKernel[int2(i, j)];
                    float4 inputValue = gInput[inputPos];
                    total += inputValue * kernelValue;
                }
            }

            gOutput[dispatchThreadId.xy] = total;
        }
    )";
}

RealtimeDSPass::RealtimeDSPass(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    // Initialize properties
    mModelPath = props.get<std::string>("modelPath", mModelPath);
    mEnableDenoising = props.get<bool>("enableDenoising", mEnableDenoising);

    // Create TensorRT denoiser
    mpDenoiser = std::make_unique<TensorRTInference>();
    initDenoiser();

    // Create compute program for splat operation
    Program::Desc desc;
    desc.addShaderLibrary(kSplatShader).csEntry("main");
    mpSplatProgram = ComputeProgram::create(mpDevice, desc);
    mpSplatVars = ComputeVars::create(mpDevice);
}

RenderPassReflection RealtimeDSPass::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    reflector.addInput(kInputColor, "Input color buffer").bindFlags(ResourceBindFlags::ShaderResource);
    reflector.addOutput(kOutputColor, "Denoised output color").bindFlags(ResourceBindFlags::UnorderedAccess | ResourceBindFlags::ShaderResource);
    return reflector;
}

void RealtimeDSPass::compile(RenderContext* pRenderContext, const CompileData& compileData)
{
    // Create staging buffers based on input dimensions
    uint32_t width = compileData.defaultTexDims.x;
    uint32_t height = compileData.defaultTexDims.y;

    mInputBuffer.resize(width * height * 4);  // RGBA
    mOutputBuffer.resize(width * height * 4);
}

void RealtimeDSPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // Get input and output textures
    mpInputColor = renderData[kInputColor]->asTexture();
    mpOutputColor = renderData[kOutputColor]->asTexture();

    if (!mEnableDenoising || !mpDenoiser)
    {
        // Copy input to output if denoising is disabled or denoiser isn't ready
        pRenderContext->copyResource(mpOutputColor.get(), mpInputColor.get());
        return;
    }

    processImage(pRenderContext, renderData);
}

bool RealtimeDSPass::initDenoiser()
{
    if (mModelPath.empty())
    {
        logWarning("Model path is empty. Denoising will be disabled.");
        return false;
    }

    return mpDenoiser->initializeFromONNX(mModelPath);
}

void RealtimeDSPass::processImage(RenderContext* pRenderContext, const RenderData& renderData)
{
    uint32_t width = mpInputColor->getWidth();
    uint32_t height = mpInputColor->getHeight();

    // Resize buffers if needed
    if (mInputBuffer.size() < width * height * 4)
    {
        mInputBuffer.resize(width * height * 4);  // RGBA
        mOutputBuffer.resize(width * height * 4);
    }

    // Create staging buffer for reading texture data
    Buffer::SharedPtr pStagingBuffer = Buffer::create(
        mpDevice,
        width * height * 4 * sizeof(float),
        ResourceBindFlags::None,
        Buffer::CpuAccess::Read
    );

    // Copy texture to staging buffer
    pRenderContext->copyResource(pStagingBuffer.get(), mpInputColor.get());

    // Map buffer and copy data to input buffer
    float* pData = static_cast<float*>(pStagingBuffer->map(Buffer::MapType::Read));
    std::memcpy(mInputBuffer.data(), pData, width * height * 4 * sizeof(float));
    pStagingBuffer->unmap();

    // Run inference
    if (mpDenoiser->infer(mInputBuffer.data(), mOutputBuffer.data()))
    {
        // Create staging buffer for writing texture data
        Buffer::SharedPtr pOutputBuffer = Buffer::create(
            mpDevice,
            width * height * 4 * sizeof(float),
            ResourceBindFlags::None,
            Buffer::CpuAccess::Write
        );

        // Copy output data to staging buffer
        float* pOutData = static_cast<float*>(pOutputBuffer->map(Buffer::MapType::Write));
        std::memcpy(pOutData, mOutputBuffer.data(), width * height * 4 * sizeof(float));
        pOutputBuffer->unmap();

        // Copy staging buffer to output texture
        pRenderContext->copyResource(mpOutputColor.get(), pOutputBuffer.get());
    }
    else
    {
        // Fallback to input on failure
        pRenderContext->copyResource(mpOutputColor.get(), mpInputColor.get());
    }
}

void RealtimeDSPass::renderUI(Gui::Widgets& widget)
{
    widget.checkbox("Enable Denoising", mEnableDenoising);
    if (widget.textbox("Model Path", mModelPath) && !mModelPath.empty())
    {
        initDenoiser();
    }
}

Properties RealtimeDSPass::getProperties() const
{
    Properties props;
    props["modelPath"] = mModelPath;
    props["enableDenoising"] = mEnableDenoising;
    return props;
}
