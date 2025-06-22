#pragma once

#include <string>
#include <unordered_map>
#include <queue>
#include <vector>
#include <cstdint>

#include "ManagedWindow.h"

#include "StreamDeckRawDevice.h"

typedef void *tjhandle;

#if 1
struct Image
{
    std::string Filepath;
    int32_t Width;
    int32_t Height;
    std::vector<uint8_t> FileData;
    std::vector<uint8_t> Content;
};
#endif

class StreamDeckWindow:public ManagedWindow
{
public:
    StreamDeckWindow();
    ~StreamDeckWindow();

protected:
    int32_t Init() override;
    int32_t Event(SDL_Event *pEvent) override;
    int32_t Iterate() override;
    void Quit() override;

private:
    //int32_t mCounter = 0; 
    std::unordered_map<std::string, StreamDeckRawDevice*> mDevicesMap;

    void DisplayDeviceTab(StreamDeckRawDevice* pDevice);

    void SetImage(StreamDeckRawDevice* pDevice, uint8_t pButtonId, const char* pImagePath);
    
    /**
     * Display a button with an image
     * 
     * \param[in] pPressed Set it to true if hardware button is pressed
     * \return True if mouse cursor is above 
     */
    bool DisplayButton(bool pPressed);

    void EnumerateDevices();

    bool mIsDroppingSomething = false;
    //bool mHasFileDropped = false;
    std::queue<std::string> mDroppedFileQueue;
    float mDropPositionX;
    float mDropPositionY;

    //TurboJpeg stuff
    tjhandle mCompressorInstance;
    tjhandle mDecompressorInstance;
};
