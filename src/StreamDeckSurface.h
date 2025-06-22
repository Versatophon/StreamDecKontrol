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

private:
    StreamDeckSurfaceID* mID = nullptr;
};
