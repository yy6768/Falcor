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
#include "ExrtoTexture.h"
#include <fstream>
#include <string>

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, ExrtoTexture>();
}

ExrtoTexture::ExrtoTexture(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice) {}

Properties ExrtoTexture::getProperties() const
{
    return {};
}

void ExrtoTexture::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    mpScene = pScene;
}

RenderPassReflection ExrtoTexture::reflect(const CompileData& compileData)
{
    // Define the required resources here
    RenderPassReflection reflector;
    reflector.addOutput("color", "color").format(ResourceFormat::RGBA32Float);
    reflector.addOutput("diffuse", "diffuse").format(ResourceFormat::RGBA32Float);
    reflector.addOutput("motion", "motion").format(ResourceFormat::RGBA32Float);
    reflector.addOutput("normal", "normal").format(ResourceFormat::RGBA32Float);
    // reflector.addInput("src");
    return reflector;
}

void ExrtoTexture::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    //auto pDevice = pRenderContext->getDevice();
    const auto& pDstTexc = renderData.getTexture("color");
    const auto& pDstTexa = renderData.getTexture("diffuse");
    const auto& pDstTexm = renderData.getTexture("motion");
    const auto& pDstTexn = renderData.getTexture("normal");
    //auto format = pDstTexc->getFormat();
    static int frame = 0;
    //if (frame== 0) std::cout << to_string(format);
    auto loadtotexture = [&](std::string name) //将exr文件绑定到纹理接口
    {
        std::ostringstream oss;
        if (name != "color")
            oss << "G:/data/bistro1/frame" << std::setw(4) << std::setfill('0') << frame << "/" << name << ".exr";
        else
            oss << "G:/data/bistro1_1spp/bistro1/frame" << std::setw(4) << std::setfill('0') << frame << "/" << name << ".exr";
        std::filesystem::path path = oss.str(); // data为Falcor的同级文件夹
        mpTex = Texture::createFromFile(mpDevice, path, false, false, ResourceBindFlags::ShaderResource, Bitmap::ImportFlags::None);
        /* if (!mpTex)
            std::cout << "Failed to load";
        else
            std::cout << "succeed to load";*/
        if (name == "color")
        {
            pRenderContext->blit(mpTex->getSRV(), pDstTexc->getRTV());
            /* auto format = pDstTexc->getFormat();
            std::cout << to_string(format);*/
        }
        else if (name == "diffuse")
        {
            pRenderContext->blit(mpTex->getSRV(), pDstTexa->getRTV());
        }
        else if (name == "motion")
        {
            pRenderContext->blit(mpTex->getSRV(), pDstTexm->getRTV());
        }
        else if (name == "normal")
        {
            pRenderContext->blit(mpTex->getSRV(), pDstTexn->getRTV());
        }
    };
    if (frame<=159)
    {
        std::cout << "....";
        loadtotexture("color");
        loadtotexture("diffuse");
        loadtotexture("motion");
        loadtotexture("normal");
        frame++;
    }
    /* if (mpScene)
        std::cout << "yyyyyyyes";
    else
        std::cout << "nooooooooo";*/
    //std::cout << frame << " ";
    //frame++;
}

void ExrtoTexture::renderUI(Gui::Widgets& widget) {}
