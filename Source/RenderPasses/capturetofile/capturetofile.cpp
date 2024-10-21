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
#include "capturetofile.h"
#include "../../Falcor/Falcor.h"
#include "../../Falcor/Core/API/Texture.h"
#include <Utils/Timing/Clock.h>
using namespace Falcor;


extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, capturetofile>();
}

capturetofile::capturetofile(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice) {}

Properties capturetofile::getProperties() const
{
    return {};
}

RenderPassReflection capturetofile::reflect(const CompileData& compileData)
{
    // Define the required resources here
    //RenderPassReflection reflector;
    RenderPassReflection reflector;
    //reflector.addInput("input", "the source texture");
    reflector.addInput("albedo", "albedo");
    reflector.addInput("color", "color");
    reflector.addInput("depth", "depth");
    reflector.addInput("normal", "normal");
    //reflector.addInput("target", "target");
    
    return reflector;
}

void capturetofile::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    static int frame = 0;
    auto save_gbuffer = [&](std::string name)
    {
        auto pTexture = renderData.getTexture(name);
        auto ext = Bitmap::getFileExtFromResourceFormat(pTexture->getFormat());
        std::filesystem::path path = "G:/data/falcor/" + name + "/" + name + std::to_string(frame) + "." + ext;
        if (name != "albedo")
            pTexture->captureToFile(0, 0, path, Bitmap::FileFormat::ExrFile, Bitmap::ExportFlags::None, true);
        else
            pTexture->captureToFile(0, 0, path, Bitmap::FileFormat::PngFile, Bitmap::ExportFlags::None, true);
    };
    if(frame < 256)
    {
        save_gbuffer("albedo");
        save_gbuffer("color");
        save_gbuffer("depth");
        save_gbuffer("normal");
        frame++;
    }
    //save_gbuffer("target");
    //pRenderContext->blit(pSrcTex->getSRV(), pDstTex->getRTV());
}
void capturetofile::renderUI(Gui::Widgets& widget) {}
