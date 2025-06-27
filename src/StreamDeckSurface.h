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

    size_t GetJpegSize();
    uint8_t* GetJpegData();
    
private:
    StreamDeckSurfaceID* mID = nullptr;
};
