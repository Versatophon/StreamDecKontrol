#pragma once

#include <string>
#include <unordered_map>
#include <queue>
#include <vector>
#include <cstdint>

#include "ManagedWindow.h"

#include "StreamDeckRawDevice.h"

class StreamDeckSurface;

class StreamDeckWindow:public ManagedWindow, public TurboJpegResourcesProvider
{
public:
    StreamDeckWindow();
    ~StreamDeckWindow();

    tjhandle GetCompressor() override;
    tjhandle GetDecompressor() override;
    tjhandle GetTransformer() override;

protected:
    int32_t Init() override;
    int32_t Event(SDL_Event *pEvent) override;
    int32_t Iterate() override;
    void Quit() override;

private:
    std::unordered_map<std::string, StreamDeckRawDevice*> mDevicesMap;

    void DisplayDeviceTab(StreamDeckRawDevice* pDevice);

    void SetImage(StreamDeckRawDevice* pDevice, uint8_t pButtonId, const char* pImagePath);
    
    /**
     * Display a button with an image
     * 
     * \param[in] pPressed Set it to true if hardware button is pressed
     * \return True if mouse cursor is above 
     */
    bool DisplayButton(bool pPressed, StreamDeckSurface* pImage);

    void EnumerateDevices();

    bool mIsDroppingSomething = false;
    std::queue<std::string> mDroppedFileQueue;
    float mDropPositionX;
    float mDropPositionY;

    //TurboJpeg stuff
    tjhandle mCompressorInstance;
    tjhandle mDecompressorInstance;
    tjhandle mTransformerInstance;

    std::vector<StreamDeckSurface*> mButtonImages;
};
