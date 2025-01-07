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
#include <fstream>
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
    //reflector.addInput("albedo", "albedo");
    reflector.addInput("color", "color");
    //reflector.addInput("depth", "depth");
    //reflector.addInput("normal", "normal");
    //reflector.addInput("posw", "posw");
    //reflector.addInput("mov", "mov");
    
    return reflector;
}

void capturetofile::setScene(RenderContext* pRenderContext, const ref<Scene>& pScene)
{
    mpScene = pScene;
}

void capturetofile::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    static int frame = 0;
    auto save_gbuffer = [&](std::string name)
    {
        auto pTexture = renderData.getTexture(name);
        auto ext = Bitmap::getFileExtFromResourceFormat(pTexture->getFormat());
        //std::filesystem::path path = "G:/data/falcor/" + name + "/" + name + std::to_string(frame) + "." + ext;
        std::ostringstream oss;
        oss << "G:/data/color/frame" << std::setw(4) << std::setfill('0') << frame << ".exr";
        if (name != "albedo")
        {
           pTexture->captureToFile(0, 0, oss.str(), Bitmap::FileFormat::ExrFile, Bitmap::ExportFlags::None, true);
        }
        else
        {
            pTexture->captureToFile(0, 0, oss.str(), Bitmap::FileFormat::ExrFile, Bitmap::ExportFlags::None, true);
        }
        //std::cout << ext;
    };
    /* auto save_camera = [&](std::string name)
    {
        std::filesystem::path path = "G:/data/falcor/" + name + "/" + name + std::to_string(frame) + ".txt";
        std::ofstream outFile(path);
        const auto& camera = mpScene->getCamera();
        float3 camera_position = camera->getPosition();
        float3 camera_up = camera->getUpVector();
        float3 camera_tagert = camera->getTarget();
        float4x4 camera_view = camera->getViewMatrix();
        // std::cout << camera_position.x << " " << camera_position.y << " " << camera_position.z;
        if (outFile.is_open())
        {
            if (name == "camera_position")
                outFile << camera_position.x << " " << camera_position.y << " " << camera_position.z << std::endl;
            else if (name == "camera_up" )
                outFile << camera_up.x << " " << camera_up.y << " " << camera_up.z << std::endl;
            else if (name == "camera_tagert")
                outFile << camera_tagert.x << " " << camera_tagert.y << " " << camera_tagert.z << std::endl;
            else if (name == "camera_view")
            {
                for (int i = 0; i < 4; i++)
                {
                    outFile << camera_view.getRow(i).x << " " << camera_view.getRow(i).y << " " << camera_view.getRow(i).z << " "
                            << camera_view.getRow(i).w
                            << std::endl;
                }
            }

            outFile.close();
        }
    };*/
    //std::cout << pTextureb << "   "<<pTexturea  << std::endl;
    if (frame<=159)
    {
        //save_gbuffer("albedo");
        save_gbuffer("color");
        //save_gbuffer("depth");
        //save_gbuffer("normal");
        //save_gbuffer("posw");
        //save_gbuffer("mov");
        //std::cout << "---------------    " << frame << "  ";
        //save_camera("camera_position");
        //save_camera("camera_up");
        //save_camera("camera_tagert");
        //save_camera("camera_view");
        frame++;
    }
    //std::cout << "---------------    " << frame << "  ";
    //frame++;
}
void capturetofile::renderUI(Gui::Widgets& widget) {}
