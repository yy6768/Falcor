#include "TensorRTInference.h"
#include "Utils/Logger.h"
#include <fstream>
#include <numeric>

namespace Falcor {

void TRTLogger::log(nvinfer1::ILogger::Severity severity, const nvinfer1::AsciiChar* msg) noexcept {
    // Map TensorRT severity to Falcor logging
    switch (severity) {
        case nvinfer1::ILogger::Severity::kINTERNAL_ERROR:
        case nvinfer1::ILogger::Severity::kERROR:
            logError("TensorRT: {}", msg);
            break;
        case nvinfer1::ILogger::Severity::kWARNING:
            logWarning("TensorRT: {}", msg);
            break;
        case nvinfer1::ILogger::Severity::kINFO:
            logInfo("TensorRT: {}", msg);
            break;
        default:
            break;
    }
}

TensorRTInference::TensorRTInference(bool enableFP16) : mEnableFP16(enableFP16) {
    cudaStreamCreate(&mStream);
}

TensorRTInference::~TensorRTInference() {
    cudaStreamDestroy(mStream);
    for (auto& buffer : mDeviceBuffers) {
        if (buffer) cudaFree(buffer);
    }
}

bool TensorRTInference::buildEngineFromONNX(const std::string& onnxPath) {
    auto builder = std::unique_ptr<nvinfer1::IBuilder>(nvinfer1::createInferBuilder(mLogger));
    if (!builder) return false;

    const auto explicitBatch = 1U << static_cast<uint32_t>(nvinfer1::NetworkDefinitionCreationFlag::kEXPLICIT_BATCH);
    auto network = std::unique_ptr<nvinfer1::INetworkDefinition>(builder->createNetworkV2(explicitBatch));
    auto config = std::unique_ptr<nvinfer1::IBuilderConfig>(builder->createBuilderConfig());
    auto parser = std::unique_ptr<nvonnxparser::IParser>(nvonnxparser::createParser(*network, mLogger));

    if (!parser->parseFromFile(onnxPath.c_str(), static_cast<int>(nvinfer1::ILogger::Severity::kWARNING))) {
        return false;
    }

    // 现代配置方式
    // config->setMaxWorkspaceSize(1 << 30);
    if (mEnableFP16) config->setFlag(nvinfer1::BuilderFlag::kFP16);

    // 处理动态形状
    auto profile = builder->createOptimizationProfile();
    auto input = network->getInput(0);
    auto inputDims = input->getDimensions();

    profile->setDimensions(input->getName(),
        nvinfer1::OptProfileSelector::kMIN, inputDims);
    profile->setDimensions(input->getName(),
        nvinfer1::OptProfileSelector::kOPT, inputDims);
    profile->setDimensions(input->getName(),
        nvinfer1::OptProfileSelector::kMAX, inputDims);

    config->addOptimizationProfile(profile);

    mEngine = std::unique_ptr<nvinfer1::ICudaEngine>(
        builder->buildEngineWithConfig(*network, *config)
    );

    if (!mEngine) return false;

    mContext = std::unique_ptr<nvinfer1::IExecutionContext>(mEngine->createExecutionContext());
    if (!mContext) return false;

    // 设置绑定维度
    // mInputDims = mEngine->getBindingDimensions(0);
    // mOutputDims = mEngine->getBindingDimensions(1);

    // 分配设备内存
    const auto inputSize = std::accumulate(mInputDims.d, mInputDims.d + mInputDims.nbDims, 1, std::multiplies<>());
    const auto outputSize = std::accumulate(mOutputDims.d, mOutputDims.d + mOutputDims.nbDims, 1, std::multiplies<>());

    cudaMalloc(&mDeviceBuffers[0], inputSize * sizeof(float));
    cudaMalloc(&mDeviceBuffers[1], outputSize * sizeof(float));

    return true;
}

bool TensorRTInference::execute(const std::vector<float>& input, std::vector<float>& output) {
    const auto inputSize = input.size() * sizeof(float);
    const auto outputSize = output.size() * sizeof(float);

    cudaMemcpyAsync(mDeviceBuffers[0], input.data(), inputSize, cudaMemcpyHostToDevice, mStream);

    if (!mContext->executeV2(mDeviceBuffers)) {
        return false;
    }

    cudaMemcpyAsync(output.data(), mDeviceBuffers[1], outputSize, cudaMemcpyDeviceToHost, mStream);
    cudaStreamSynchronize(mStream);

    return true;
}

} // namespace Falcor
