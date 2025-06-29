#pragma once

#include <string>
#include <unordered_map>
#include <queue>
#include <vector>
#include <cstdint>

#include "ManagedWindow.h"
#include "FileDropProvider.h"

//#include "StreamDeckPhysicalDevice.h"

extern "C"
{//TODO: need to move this
    #include <hidapi/hidapi.h>
}

class StreamDeckDevice;

class StreamDeckWindow:public ManagedWindow, public TurboJpegResourcesProvider, public FileDropProvider
{
public:
    StreamDeckWindow();
    ~StreamDeckWindow();

    //TurboJpegResourcesProvider
    tjhandle GetCompressor() override;
    tjhandle GetDecompressor() override;
    tjhandle GetTransformer() override;

    //FileDropProvider
    const char* GetQueuedFilepath() override;
    bool DropPending() override;
    float DropPositionX() override;
    float DropPositionY() override;

protected:
    int32_t Init() override;
    int32_t Event(SDL_Event *pEvent) override;
    int32_t Iterate() override;
    void Quit() override;

private:
    std::unordered_map<std::string, StreamDeckDevice*> mDevicesMap;

    void EnumerateDevices();

    bool mIsDroppingSomething = false;
    std::queue<std::string> mDroppedFileQueue;
    float mDropPositionX;
    float mDropPositionY;

    //TurboJpeg stuff
    tjhandle mCompressorInstance;
    tjhandle mDecompressorInstance;
    tjhandle mTransformerInstance;

    //DnD path storage
    std::string mLastDroppedFilepath;
};
