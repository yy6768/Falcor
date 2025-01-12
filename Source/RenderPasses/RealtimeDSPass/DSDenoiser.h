#pragma once
#include "Falcor.h"
#include "Utils/TensorRT/TensorRTInference.h"

using namespace Falcor;

class DSDenoiser
{
public:
    struct Desc
    {
        uint2 resolution;
        std::string modelPath;
    };

    DSDenoiser(ref<Device> pDevice);
    ~DSDenoiser();

    bool init(const Desc& desc);

    bool denoise(
        RenderContext* pRenderContext,
        const ref<Texture>& pNoisy,
        const ref<Texture>& pAlbedo,
        const ref<Texture>& pNormal,
        const ref<Texture>& pOutput
    );

private:
    ref<Device> mpDevice;
    TensorRTInference mInference;

    ref<Buffer> mpInputBuffer;
    ref<Buffer> mpOutputBuffer;

    uint2 mResolution = {0, 0};
    bool mIsInitialized = false;

    // 辅助函数
    bool createBuffers(uint2 resolution);
    bool copyTextureToBuffer(RenderContext* pRenderContext, const ref<Texture>& tex, ref<Buffer>& buf, uint32_t offset);
};
