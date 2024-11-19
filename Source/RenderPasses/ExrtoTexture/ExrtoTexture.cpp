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
    std::ifstream posFile("G:/data/frame0000/camera_position.txt");
    float x, y, z;
    posFile >> x >> y >> z;
    float3 eyePosition = float3(x, y, z);
    posFile.close();

    // 读取目标位置
    std::ifstream targetFile("G:/data/frame0000/camera_target.txt");
    targetFile >> x >> y >> z;
    float3 targetPosition = float3(x, y, z);
    targetFile.close();

    // 读取上向量
    std::ifstream upFile("G:/data/frame0000/camera_up.txt");
    upFile >> x >> y >> z;
    float3 upVector = float3(x, y, z);
    upFile.close();

    // 设置Falcor相机
    pScene->getCamera()->setPosition(eyePosition);
    pScene->getCamera()->setTarget(targetPosition);
    pScene->getCamera()->setUpVector(upVector);

    mpScene = pScene;
}

RenderPassReflection ExrtoTexture::reflect(const CompileData& compileData)
{
    // Define the required resources here
    RenderPassReflection reflector;
    reflector.addOutput("color", "color");
    reflector.addOutput("diffuse", "diffuse");
    reflector.addOutput("motion", "motion");
    reflector.addOutput("normal", "normal");
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
    static int frame = 0;
    auto loadtotexture = [&](std::string name) //将exr文件绑定到纹理接口
    {
        std::ostringstream oss;
        //oss << "G:/data/bistro1png/" << name << "/frame" << std::setw(4) << std::setfill('0') << frame << ".png";
        //std::filesystem::path path = oss.str(); // data为Falcor的同级文件夹
        std::filesystem::path path;
        if (name != "albedo")
            path = "G:/data/frame0000/" + name + ".exr";
        else
            path = "G:/data/frame0000/diffuse.exr";
        //std::cout << "------------------------";
        mpTex = Texture::createFromFile(mpDevice, path, false, true);
        /* if (!mpTex)
            std::cout << "Failed to load";
        else
            std::cout << "succeed to load";*/
        if (name == "color")
        {
            pRenderContext->blit(mpTex->getSRV(), pDstTexc->getRTV());
        }
        else if (name == "albedo")
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
    if (frame==0)
    {
        loadtotexture("color");
        loadtotexture("albedo");
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
