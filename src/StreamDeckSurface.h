#pragma once

#include <cstdint>
#include <cstddef>

struct StreamDeckSurfaceID;
class SdlResourcesProvider;
class TurboJpegResourcesProvider;

class StreamDeckSurface
{
public:
    StreamDeckSurface(const char* pFilepath, SdlResourcesProvider* pSdlResourcesProvider, TurboJpegResourcesProvider* pTurboJpegResourcesProvider);
    ~StreamDeckSurface();

    size_t GetTexture();
    
    bool IsValid();

    size_t GetFrameCount();
    size_t GetJpegSize(size_t pFrameIndex = 0);
    uint8_t* GetJpegData(size_t pFrameIndex = 0);
    
private:
    StreamDeckSurfaceID* mID = nullptr;
};
