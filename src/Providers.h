#pragma once

struct SDL_Window;
struct SDL_Renderer;

typedef void* tjhandle;

class SdlResourcesProvider
{
public:
    virtual SDL_Window* GetSdlWindow() = 0;
    virtual SDL_Renderer* GetSdlRenderer() = 0;
};

class TurboJpegResourcesProvider
{
public:
    virtual tjhandle GetCompressor() = 0;
    virtual tjhandle GetDecompressor() = 0;
    virtual tjhandle GetTransformer() = 0;
};
