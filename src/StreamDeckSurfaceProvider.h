#pragma once

class SdlResourcesProvider;

struct StreamDeckSurfaceProviderId;
struct StreamDeckSurface;

class StreamDeckSurfaceProvider
{
public:
    StreamDeckSurfaceProvider(SdlResourcesProvider* pSdlResourcesProvider);
    ~StreamDeckSurfaceProvider();

    StreamDeckSurface* GetSurface(const char* pFilepath);
    void ReleaseSurface(StreamDeckSurface* pStreamDeckSurface);

private:
    StreamDeckSurfaceProviderId* mId = nullptr;
};
