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
    // process single image denoising
    void processImage(const std::filesystem::path& inputPath);
    // batch process images in directory
    void processDirectory();
    // create and setup render graph
    void setupRenderGraph();
    // image display
    void displayImage(Gui::Window& window, const ref<Texture>& tex, const std::string& label);


    // render graph
    ref<RenderGraph> mpGraph;
    // render context
    ref<RenderContext> mpRenderContext;

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
