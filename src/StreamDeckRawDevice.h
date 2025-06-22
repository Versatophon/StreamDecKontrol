#pragma once

#include <cstdint>
#include <string>
#include <vector>

extern "C"
{
    #include <hidapi/hidapi.h>
}

class StreamDeckRawDevice
{
public:
    StreamDeckRawDevice(uint16_t pVendorID, uint16_t pProductID, const char* pSerial);
    
    bool Update();

    void Reset();

    /**
     * \param[in] pBrightness brightness to set [0-100]
     */
    void SetBrightness(uint8_t pBrightness);

    uint8_t GetBrightness() const;

    void SetScreenSaverTime(uint32_t pTiming);

    void SetImageFromPath(uint8_t pButtonIndex, const char* pFilepath);

    const char* GetSerial() const;

    bool GetButtonPressed(uint8_t pButtonIndex) const;

private:
    uint16_t mVendorID;
    uint16_t mProductID;
    std::string mSerial;
    hid_device* mDevice = nullptr;
    std::vector<bool> mButtons = std::vector<bool>(15, false);
    std::vector<uint8_t> mBuffer = std::vector<uint8_t>(512, 0);
    uint8_t mBrightness = 50;

    //TODO: create a listener instead
    void ButtonChanged(uint32_t pIndex, bool pPressed);
};
