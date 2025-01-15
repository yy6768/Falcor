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
#include "OptixDenoiserApp.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

OptixDenoiserApp::OptixDenoiserApp(const SampleAppConfig& config) : SampleApp(config)
{
}

OptixDenoiserApp::~OptixDenoiserApp()
{
}

void OptixDenoiserApp::onLoad(RenderContext* pRenderContext)
{
    // Create output directory
    if (!std::filesystem::exists(mConfig.outputDir))
    {
        std::filesystem::create_directories(mConfig.outputDir);
    }

    setupRenderGraph();
}

void OptixDenoiserApp::setupRenderGraph()
{
    auto pDevice = getDevice();
    mpGraph = RenderGraph::create(pDevice, "OptixDenoiser");

    // ColorLoader pass
    Properties colorProps;
    colorProps["mips"] = true;
    colorProps["srgb"] = false;
    colorProps["outputFormat"] = "RGBA32Float";
    auto pColorLoader = RenderPass::create("ImageLoader", pDevice, colorProps);
    mpGraph->addPass(pColorLoader, "ColorLoader");

    // AlbedoLoader pass
    Properties albedoProps;
    albedoProps["mips"] = true;
    albedoProps["srgb"] = false;
    albedoProps["outputFormat"] = "RGBA32Float";
    auto pAlbedoLoader = RenderPass::create("ImageLoader", pDevice, albedoProps);
    mpGraph->addPass(pAlbedoLoader, "AlbedoLoader");

    // NormalLoader pass
    Properties normalProps;
    normalProps["mips"] = true;
    normalProps["srgb"] = false;
    normalProps["outputFormat"] = "RGBA32Float";
    auto pNormalLoader = RenderPass::create("ImageLoader", pDevice, normalProps);
    mpGraph->addPass(pNormalLoader, "NormalLoader");

    // MotionLoader pass
    Properties motionProps;
    motionProps["mips"] = true;
    motionProps["srgb"] = false;
    motionProps["outputFormat"] = "RGBA32Float";
    auto pMotionLoader = RenderPass::create("ImageLoader", pDevice, motionProps);
    mpGraph->addPass(pMotionLoader, "MotionLoader");

    // OptixDenoiser pass
    Properties denoiserProps;
    denoiserProps["denoiseAlpha"] = mConfig.denoiseAlpha;
    denoiserProps["blend"] = mConfig.blendFactor;
    auto pOptixDenoiser = RenderPass::create("OptixDenoiser", pDevice, denoiserProps);
    mpGraph->addPass(pOptixDenoiser, "OptixDenoiser");

    // Connect passes
    mpGraph->addEdge("ColorLoader.dst", "OptixDenoiser.color");
    mpGraph->addEdge("AlbedoLoader.dst", "OptixDenoiser.albedo");
    mpGraph->addEdge("NormalLoader.dst", "OptixDenoiser.normal");
    mpGraph->addEdge("MotionLoader.dst", "OptixDenoiser.mvec");

    // Mark output
    mpGraph->markOutput("OptixDenoiser.output");

}

void OptixDenoiserApp::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    const float4 clearColor(0.38f, 0.52f, 0.10f, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
}

void OptixDenoiserApp::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Optix Denoiser", {400, 600});

    // 配置部分
    if (auto group = w.group("Configuration"))
    {
        group.text("Input Directory: " + mConfig.inputDir.string());
        if (group.button("Browse Input"))
        {
            // TODO: 添加文件浏览器功能
        }

        group.text("Output Directory: " + mConfig.outputDir.string());
        if (group.button("Browse Output"))
        {
            // TODO: 添加文件浏览器功能
        }

        group.checkbox("Denoise Alpha", mConfig.denoiseAlpha);
        group.slider("Blend Factor", mConfig.blendFactor, 0.0f, 1.0f);
    }

    // 处理控制
    if (auto group = w.group("Processing"))
    {
        if (!mConfig.processing)
        {
            if (group.button("Start Processing"))
            {
                processDirectory();
            }
        }
        else
        {
            group.text("Processing...");
            float progress = mConfig.processedCount / (float)mConfig.totalCount;

            group.text("Processed: " + std::to_string(mConfig.processedCount) + "/" +
                      std::to_string(mConfig.totalCount));
            group.text("Current: " + mConfig.currentFile);
        }
    }

    // 预览部分
    if (auto group = w.group("Preview"))
    {
        if (mConfig.pInputPreview)
        {
            displayImage(w, mConfig.pInputPreview, "Input");
        }
        if (mConfig.pOutputPreview)
        {
            displayImage(w, mConfig.pOutputPreview, "Output");
        }
    }
}

void OptixDenoiserApp::processImage(const std::filesystem::path& framePath)
{
    try
    {
        mConfig.currentFile = framePath.filename().string();

        // 构建相关文件路径
        std::string colorPath = (framePath / "color.exr").string();
        std::string albedoPath = (framePath / "albedo.exr").string();
        std::string normalPath = (framePath / "normal.exr").string();
        std::string motionPath = (framePath / "motion.exr").string();

        // 首先加载颜色图像以获取尺寸
        auto pColorImage = Bitmap::createFromFile(colorPath, true);
        if (!pColorImage)
        {
            throw std::runtime_error("Failed to load color image: " + colorPath);
        }

        uint2 imageSize = uint2(pColorImage->getWidth(), pColorImage->getHeight());
        if (imageSize.x == 0 || imageSize.y == 0)
        {
            throw std::runtime_error("Invalid image dimensions");
        }

        // 创建一个临时的 FBO 来设置正确的尺寸
        ref<Fbo> pResizeFbo = Fbo::create(getDevice());
        ref<Texture> pResizeTex = getDevice()->createTexture2D(
            imageSize.x, imageSize.y,
            ResourceFormat::RGBA32Float,
            1, 1, nullptr,
            ResourceBindFlags::RenderTarget
        );
        pResizeFbo->attachColorTarget(pResizeTex, 0);

        // 通过 onResize 更新渲染图尺寸
        mpGraph->onResize(pResizeFbo.get());

        // 设置输入路径
        auto pColorLoader = mpGraph->getPass("ColorLoader");
        Properties colorProps;
        colorProps["filename"] = colorPath;
        colorProps["mips"] = true;
        colorProps["srgb"] = false;
        pColorLoader->setProperties(colorProps);

        auto pAlbedoLoader = mpGraph->getPass("AlbedoLoader");
        Properties albedoProps;
        albedoProps["filename"] = albedoPath;
        albedoProps["mips"] = true;
        albedoProps["srgb"] = false;
        pAlbedoLoader->setProperties(albedoProps);

        auto pNormalLoader = mpGraph->getPass("NormalLoader");
        Properties normalProps;
        normalProps["filename"] = normalPath;
        normalProps["mips"] = true;
        normalProps["srgb"] = false;
        pNormalLoader->setProperties(normalProps);

        auto pMotionLoader = mpGraph->getPass("MotionLoader");
        Properties motionProps;
        motionProps["filename"] = motionPath;
        motionProps["mips"] = true;
        motionProps["srgb"] = false;
        pMotionLoader->setProperties(motionProps);

        // 编译和执行渲染图
        mpGraph->compile(getRenderContext());
        mpGraph->execute(getRenderContext());

        // Get output
        ref<Texture> pOutputTex = mpGraph->getOutput("OptixDenoiser.output")->asTexture();
        mConfig.pOutputPreview = pOutputTex;

        // Build output path
        std::filesystem::path outputPath = mConfig.outputDir / (framePath.filename().string() + ".exr");
        // Create output directory if it doesn't exist
        std::filesystem::create_directories(mConfig.outputDir);
        // Save result
        pOutputTex->captureToFile(0, 0, outputPath.string());

        mConfig.processedCount++;
    }
    catch(const std::exception& e)
    {
        logError("Error processing frame {}: {}", framePath.string(), e.what());
    }
}

void OptixDenoiserApp::processDirectory()
{
    try
    {
        mConfig.totalCount = 0;
        mConfig.processedCount = 0;
        mConfig.processing = true;

        // 遍历input目录下的所有子文件夹
        for(auto& p: std::filesystem::directory_iterator(mConfig.inputDir))
        {
            if(std::filesystem::is_directory(p) &&
               std::filesystem::exists(p.path() / "color.exr"))
            {
                mConfig.totalCount++;
            }
        }

        // 按帧处理
        for(auto& p: std::filesystem::directory_iterator(mConfig.inputDir))
        {
            if(std::filesystem::is_directory(p))
            {
                auto framePath = p.path();
                // 检查必要的文件是否都存在
                if(std::filesystem::exists(framePath / "color.exr"))
                {processImage(framePath);
                }
            }
        }

        mConfig.processing = false;
    }
    catch(const std::exception& e)
    {
        logError("Error processing directory {}: {}", mConfig.inputDir.string(), e.what());
        mConfig.processing = false;
    }
}

void OptixDenoiserApp::displayImage(Gui::Window& window, const ref<Texture>& tex, const std::string& label)
{
    if (auto group = window.group(label))
    {
        // 计算预览尺寸
        float aspectRatio = (float)tex->getHeight() / tex->getWidth();
        uint32_t previewWidth = std::min(tex->getWidth(), 300u);
        uint32_t previewHeight = static_cast<uint32_t>(previewWidth * aspectRatio);

        group.image((label + " Preview").c_str(), tex.get(), {previewWidth, previewHeight});
    }
}

void OptixDenoiserApp::onShutdown()
{
    //
}

void OptixDenoiserApp::onResize(uint32_t width, uint32_t height)
{
    //
}

bool OptixDenoiserApp::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return false;
}

bool OptixDenoiserApp::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

void OptixDenoiserApp::onHotReload(HotReloadFlags reloaded)
{
    //
}

int runMain(int argc, char** argv)
{
    SampleAppConfig config;
    config.windowDesc.title = "Optix Denoiser App";
    config.windowDesc.resizableWindow = true;

    OptixDenoiserApp project(config);
    return project.run();
}

int main(int argc, char** argv)
{
    return catchAndReportAllExceptions([&]()
    {
        return runMain(argc, argv);
    });

}
