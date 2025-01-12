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
    auto pColorLoader = RenderPass::create("ImageLoader", pDevice, {});
    mpGraph->addPass(pColorLoader, "ColorLoader");
    // AlbedoLoader pass
    auto pAlbedoLoader = RenderPass::create("ImageLoader", pDevice, {});
    mpGraph->addPass(pAlbedoLoader, "AlbedoLoader");
    // NormalLoader pass
    auto pNormalLoader = RenderPass::create("ImageLoader", pDevice, {});
    mpGraph->addPass(pNormalLoader, "NormalLoader");
    // MotionLoader pass
    auto pMotionLoader = RenderPass::create("ImageLoader", pDevice, {});
    mpGraph->addPass(pMotionLoader, "MotionLoader");

    // OptixDenoiser pass
    auto pOptixDenoiser = RenderPass::create("OptixDenoiser", pDevice, {});
    mpGraph->addPass(pOptixDenoiser, "OptixDenoiser");

    // Connect passes
    mpGraph->addEdge("ColorLoader.dst", "OptixDenoiser.src");
    mpGraph->addEdge("AlbedoLoader.dst", "OptixDenoiser.albedo");
    mpGraph->addEdge("NormalLoader.dst", "OptixDenoiser.normal");
    mpGraph->addEdge("MotionLoader.dst", "OptixDenoiser.mvec");

    // Set parameters
    auto props = pOptixDenoiser->getProperties();
    props["denoiseAlpha"] = mConfig.denoiseAlpha;
    props["blend"] = mConfig.blendFactor;

    // Mark output
    mpGraph->markOutput("OptixDenoiser.dst");
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

void OptixDenoiserApp::processImage(const std::filesystem::path& inputPath)
{
    try
    {
        mConfig.currentFile = inputPath.filename().string();

        // 构建相关文件路径
        std::string colorPath = inputPath.string();
        std::string albedoPath = inputPath.parent_path().string() + "/" +
                                inputPath.stem().string() + "_albedo.exr";
        std::string normalPath = inputPath.parent_path().string() + "/" +
                                inputPath.stem().string() + "_normal.exr";
        std::string motionPath = inputPath.parent_path().string() + "/" +
                                inputPath.stem().string() + "_motion.exr";

        // 设置输入路径
        auto pColorLoader = mpGraph->getPass("ColorLoader");
        pColorLoader->setProperties({
            {"colorPath", colorPath},
        });
        auto pAlbedoLoader = mpGraph->getPass("AlbedoLoader");
        pAlbedoLoader->setProperties({
            {"albedoPath", albedoPath},
        });
        auto pNormalLoader = mpGraph->getPass("NormalLoader");
        pNormalLoader->setProperties({
            {"normalPath", normalPath},
        });
        auto pMotionLoader = mpGraph->getPass("MotionLoader");
        pMotionLoader->setProperties({
            {"motionPath", motionPath},
        });

        // Execute denoising
        auto pRenderContext = getRenderContext();
        mpGraph->execute(pRenderContext);

        // Get output
        ref<Texture> pOutputTex = mpGraph->getOutput("OptixDenoiser.dst")->asTexture();
        mConfig.pOutputPreview = pOutputTex;

        // Build output path
        std::filesystem::path outputPath = mConfig.outputDir / inputPath.filename();
        outputPath.replace_extension(".denoised.exr");

        // Save result
        pOutputTex->captureToFile(0, 0, outputPath.string());

        mConfig.processedCount++;
    }
    catch(const std::exception& e)
    {
        logError("Error processing image {}: {}", inputPath.string(), e.what());
    }
}

void OptixDenoiserApp::processDirectory()
{
    try
    {
        // 计算总文件数
        mConfig.totalCount = 0;
        mConfig.processedCount = 0;
        mConfig.processing = true;

        for(auto& p: std::filesystem::directory_iterator(mConfig.inputDir))
        {
            if(p.path().extension() == ".exr")
            {
                mConfig.totalCount++;
            }
        }

        // 处理文件
        for(auto& p: std::filesystem::directory_iterator(mConfig.inputDir))
        {
            if(p.path().extension() == ".exr")
            {
                processImage(p.path());
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
    config.windowDesc.title = "Falcor Project Template";
    config.windowDesc.resizableWindow = true;

    OptixDenoiserApp project(config);
    return project.run();
}

int main(int argc, char** argv)
{
    return catchAndReportAllExceptions([&]() { return runMain(argc, argv); });
}
