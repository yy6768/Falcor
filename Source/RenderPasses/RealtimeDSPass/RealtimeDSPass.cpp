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
// #include "RealtimeDSPass.h"

// extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
// {
//     registry.registerClass<RenderPass, RealtimeDSPass>();
// }

// RealtimeDSPass::RealtimeDSPass(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
// {
//     mpDenoiser = std::make_unique<TensorRTDenoiser>(pDevice);
// }

// Properties RealtimeDSPass::getProperties() const
// {
//     return {};
// }

// RenderPassReflection RealtimeDSPass::reflect(const CompileData& compileData)
// {
//     RenderPassReflection reflector;

//     // 定义输入输出
//     reflector.addInput("noisy", "Noisy input image").bindFlags(ResourceBindFlags::ShaderResource);
//     reflector.addInput("albedo", "Surface albedo").bindFlags(ResourceBindFlags::ShaderResource);
//     reflector.addInput("normal", "Surface normal").bindFlags(ResourceBindFlags::ShaderResource);

//     reflector.addOutput("output", "Denoised output image").bindFlags(ResourceBindFlags::UnorderedAccess | ResourceBindFlags::ShaderResource);

//     return reflector;
// }

// void RealtimeDSPass::compile(RenderContext* pRenderContext, const CompileData& compileData)
// {
//     // 初始化降噪器
//     TensorRTDenoiser::Desc desc;
//     desc.resolution = {compileData.defaultTexDims.x, compileData.defaultTexDims.y};
//     desc.modelPath = mModelPath;

//     if (!mpDenoiser->init(desc))
//     {
//         FALCOR_THROW("Failed to initialize denoiser");
//     }
// }

// void RealtimeDSPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
// {
//     if (!mEnableDenoising) return;

//     // 获取输入输出纹理
//     const auto& pNoisy = renderData.getTexture("noisy");
//     const auto& pAlbedo = renderData.getTexture("albedo");
//     const auto& pNormal = renderData.getTexture("normal");
//     const auto& pOutput = renderData.getTexture("output");

//     // 执行降噪
//     mpDenoiser->denoise(pRenderContext, pNoisy, pAlbedo, pNormal, pOutput);
// }

// void RealtimeDSPass::renderUI(Gui::Widgets& widget)
// {
//     widget.checkbox("Enable Denoising", mEnableDenoising);
//     if (widget.textbox("Model Path", mModelPath))
//     {
//         // 重新初始化降噪器
//         TensorRTDenoiser::Desc desc;
//         desc.resolution = {0, 0}; // 将在compile时更新
//         desc.modelPath = mModelPath;
//         mpDenoiser->init(desc);
//     }
// }
