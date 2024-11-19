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
#include "OptixDenoiserReference.h"

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, OptixDenoiserReference>();
}

OptixDenoiserReference::OptixDenoiserReference(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice) {}

Properties OptixDenoiserReference::getProperties() const
{
    return {};
}

RenderPassReflection OptixDenoiserReference::reflect(const CompileData& compileData)
{
    // Define the required resources here
    RenderPassReflection reflector;
    reflector.addInput("color", "color");
    return reflector;
}

void OptixDenoiserReference::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    static int frame = 0;
    auto pTexture = renderData.getTexture("color");
    std::ostringstream oss;
    oss << "G:/data/bistro1png/denoiser/frame" << std::setw(4) << std::setfill('0') << frame << ".exr";
    //std::filesystem::path path = "G:/data/falcor/reference/reference" + std::to_string(frame) + ".exr";
    if (frame == 0 || frame == 127)
    {
        pTexture->captureToFile(0, 0, oss.str(), Bitmap::FileFormat::ExrFile, Bitmap::ExportFlags::None, true);
        //frame++;
    }
    frame++;
    // renderData holds the requested resources
    // auto& pTexture = renderData.getTexture("src");
}

void OptixDenoiserReference::renderUI(Gui::Widgets& widget) {}
