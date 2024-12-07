#include "TensorRTInference.h"
#include "NvOnnxParser.h"
#include <fstream>

namespace Falcor {

void Logger::log(Severity severity, const char* msg) noexcept {
    // Map TensorRT severity to Falcor logging
    switch (severity) {
        case Severity::kINTERNAL_ERROR:
        case Severity::kERROR:
            FALCOR_ERROR("TensorRT: {}", msg);
            break;
        case Severity::kWARNING:
            FALCOR_WARN("TensorRT: {}", msg);
            break;
        case Severity::kINFO:
            FALCOR_INFO("TensorRT: {}", msg);
            break;
        default:
            break;
    }
}

TensorRTInference::TensorRTInference() {
    cudaStreamCreate(&mCudaStream);
}

TensorRTInference::~TensorRTInference() {
    if (mContext) mContext->destroy();
    if (mEngine) mEngine->destroy();
    if (mRuntime) mRuntime->destroy();
    
    for (void* buffer : mCudaBuffers) {
        if (buffer) cudaFree(buffer);
    }
    
    if (mCudaStream) cudaStreamDestroy(mCudaStream);
}

bool TensorRTInference::initializeFromONNX(const std::string& onnxPath) {
    // Create builder
    auto builder = std::unique_ptr<nvinfer1::IBuilder>(
        nvinfer1::createInferBuilder(mLogger));
    if (!builder) return false;

    // Create network
    const auto explicitBatch = 1U << static_cast<uint32_t>(
        nvinfer1::NetworkDefinitionCreationFlag::kEXPLICIT_BATCH);
    auto network = std::unique_ptr<nvinfer1::INetworkDefinition>(
        builder->createNetworkV2(explicitBatch));
    if (!network) return false;

    // Create ONNX parser
    auto parser = std::unique_ptr<nvonnxparser::IParser>(
        nvonnxparser::createParser(*network, mLogger));
    if (!parser) return false;

    // Parse ONNX file
    if (!parser->parseFromFile(onnxPath.c_str(), 
        static_cast<int>(nvinfer1::ILogger::Severity::kWARNING))) {
        return false;
    }

    // Create inference engine
    auto config = std::unique_ptr<nvinfer1::IBuilderConfig>(
        builder->createBuilderConfig());
    if (!config) return false;

    // Build serialized engine
    auto engineData = std::unique_ptr<nvinfer1::IHostMemory>(
        builder->buildSerializedNetwork(*network, *config));
    if (!engineData) return false;

    // Create runtime and engine
    mRuntime = nvinfer1::createInferRuntime(mLogger);
    if (!mRuntime) return false;

    mEngine = mRuntime->deserializeCudaEngine(
        engineData->data(), engineData->size());
    if (!mEngine) return false;

    // Create execution context
    mContext = mEngine->createExecutionContext();
    if (!mContext) return false;

    // Get dimensions and allocate buffers
    mInputDims = mEngine->getBindingDimensions(0);
    mOutputDims = mEngine->getBindingDimensions(1);

    size_t inputSize = volume(mInputDims) * sizeof(float);
    size_t outputSize = volume(mOutputDims) * sizeof(float);

    cudaMalloc(&mCudaBuffers[0], inputSize);
    cudaMalloc(&mCudaBuffers[1], outputSize);

    return true;
}

bool TensorRTInference::infer(const float* input, float* output) {
    // Copy input to GPU
    size_t inputSize = volume(mInputDims) * sizeof(float);
    cudaMemcpyAsync(mCudaBuffers[0], input, inputSize, 
        cudaMemcpyHostToDevice, mCudaStream);

    // Execute inference
    if (!mContext->enqueueV2(mCudaBuffers, mCudaStream, nullptr))
        return false;

    // Copy output back to CPU
    size_t outputSize = volume(mOutputDims) * sizeof(float);
    cudaMemcpyAsync(output, mCudaBuffers[1], outputSize,
        cudaMemcpyDeviceToHost, mCudaStream);

    cudaStreamSynchronize(mCudaStream);
    return true;
}

} // namespace Falcor 