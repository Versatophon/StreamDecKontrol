#pragma once

#include <string>
#include <unordered_map>

#include "ManagedWindow.h"

#include "StreamDeckRawDevice.h"

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
    void DisplayButton(bool pPressed);

    void EnumerateDevices();

    bool mIsDroppingSomething = false;
    float mDropPositionX;
    float mDropPositionY; 
};
