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
    // 创建输出目录
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

    // 创建多个ImageLoader pass
    auto pColorLoader = RenderPass::create("ImageLoader", pDevice);
    auto pAlbedoLoader = RenderPass::create("ImageLoader", pDevice);
    auto pNormalLoader = RenderPass::create("ImageLoader", pDevice);
    auto pMotionLoader = RenderPass::create("ImageLoader", pDevice);

    mpGraph->addPass(pColorLoader, "ColorLoader");
    mpGraph->addPass(pAlbedoLoader, "AlbedoLoader");
    mpGraph->addPass(pNormalLoader, "NormalLoader");
    mpGraph->addPass(pMotionLoader, "MotionLoader");

    // 创建OptixDenoiser pass
    auto pOptixDenoiser = RenderPass::create("OptixDenoiser", mpDevice);
    mpGraph->addPass(pOptixDenoiser, "OptixDenoiser");

    // 连接passes
    mpGraph->addEdge("ColorLoader.dst", "OptixDenoiser.color");
    mpGraph->addEdge("AlbedoLoader.dst", "OptixDenoiser.albedo");
    mpGraph->addEdge("NormalLoader.dst", "OptixDenoiser.normal");
    mpGraph->addEdge("MotionLoader.dst", "OptixDenoiser.mvec");

    // 设置参数
    auto& props = pOptixDenoiser->getProperties();
    props["denoiseAlpha"] = mConfig.denoiseAlpha;
    props["blend"] = mConfig.blendFactor;

    // 标记输出
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
            // group.progressBar(progress);
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

        // 设置各个Loader的属性
        auto setLoaderProperties = [this](const std::string& passName,
                                        const std::string& path,
                                        bool srgb = false) {
            auto pass = mpGraph->getPass(passName);
            Properties props;
            props["filename"] = path;
            props["srgb"] = srgb;
            props["mips"] = false;
            pass->setProperties(props);
        };

        // 设置各个Loader的输入文件
        setLoaderProperties("ColorLoader", colorPath, true);  // color通常需要sRGB
        setLoaderProperties("AlbedoLoader", albedoPath);
        setLoaderProperties("NormalLoader", normalPath);
        setLoaderProperties("MotionLoader", motionPath);

        RenderContext* pRenderContext = getRenderContext();
        // Execute
        mpGraph->execute(pRenderContext);

        // 获取输出并保存
        ref<Texture> pOutputTex = mpGraph->getOutput("OptixDenoiser.output")->asTexture();

        // 构建输出路径
        std::filesystem::path outputPath = mConfig.outputDir / inputPath.filename();
        // 保存结果
        pOutputTex->captureToFile(0,
                                  0,
                                  outputPath.string(),
                                  Bitmap::FileFormat::ExrFile);
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
        uint32_t previewHeight = (uint32_t)(previewWidth * aspectRatio);

        group.image(label + " Preview", tex, {previewWidth, previewHeight});
    }
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
    config.windowDesc.title = "Falcor Optix Denoiser";
    config.windowDesc.resizableWindow = true;

    OptixDenoiserApp project(config);
    return project.run();
}

int main(int argc, char** argv)
{
    return catchAndReportAllExceptions([&]() { return runMain(argc, argv); });
}
