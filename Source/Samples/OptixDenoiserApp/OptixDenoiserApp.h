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
#pragma once
#include "Falcor.h"
#include "Core/SampleApp.h"
#include "RenderGraph/RenderGraph.h"

using namespace Falcor;

class OptixDenoiserApp : public SampleApp
{
public:
    OptixDenoiserApp(const SampleAppConfig& config);
    ~OptixDenoiserApp();

    void onLoad(RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;

private:
    // 处理单个图像的降噪
    void processImage(const std::filesystem::path& inputPath);
    // 批量处理文件夹中的图像
    void processDirectory();
    // 创建并设置渲染图
    void setupRenderGraph();
    // 显示预览图像
    void displayImage(Gui::Window& window, const ref<Texture>& tex, const std::string& label);

    ref<RenderGraph> mpGraph;

    struct {
        std::filesystem::path inputDir = "input";     // 输入目录
        std::filesystem::path outputDir = "output";   // 输出目录
        std::string filePattern = "*.exr";           // 文件匹配模式
        bool processing = false;                     // 当前是否正在处理
        uint32_t processedCount = 0;                // 已处理图像数量
        uint32_t totalCount = 0;                    // 总图像数量

        // 预览相关
        ref<Texture> pInputPreview;                 // 输入图像预览
        ref<Texture> pOutputPreview;                // 输出图像预览
        std::string currentFile;                    // 当前处理的文件名

        // Optix参数
        bool denoiseAlpha = false;                  // 是否降噪Alpha通道
        float blendFactor = 0.0f;                   // 混合因子(0=降噪,1=原图)
    } mConfig;
};
