#pragma once
#include "Falcor.h"
#include "NvInfer.h"
#include "cuda_runtime_api.h"
#include <memory>
#include <string>
#include <vector>

namespace Falcor {

// Logger for TensorRT info/warning/errors
class Logger : public nvinfer1::ILogger {
public:
    void log(Severity severity, const char* msg) noexcept override;
};

class TensorRTInference {
public:
    TensorRTInference();
    ~TensorRTInference();

    // Initialize engine from ONNX model
    bool initializeFromONNX(const std::string& onnxPath);

    // Run inference
    bool infer(const float* input, float* output);

    // Get input/output dimensions
    const nvinfer1::Dims& getInputDims() const { return mInputDims; }
    const nvinfer1::Dims& getOutputDims() const { return mOutputDims; }

private:
    Logger mLogger;
    nvinfer1::IRuntime* mRuntime{nullptr};
    nvinfer1::ICudaEngine* mEngine{nullptr};
    nvinfer1::IExecutionContext* mContext{nullptr};

    nvinfer1::Dims mInputDims;
    nvinfer1::Dims mOutputDims;

    void* mCudaBuffers[2]{nullptr, nullptr}; // Input and output buffers
    cudaStream_t mCudaStream{nullptr};

    // Helper function to calculate volume of dimensions
    size_t volume(const nvinfer1::Dims& dims) const;
};

} // namespace Falcor
