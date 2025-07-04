#include "StreamDeckPhysicalDevice.h"

#include <iostream>
#include <cstring>
#include <fstream>
#include <filesystem>

#include "StreamDeckSurface.h"

std::wstring ConvertStringToWString(const std::string& pString)
{
    return std::wstring(pString.begin(), pString.end());
}

std::string ConvertWStringToString(const std::wstring& pString)
{
    return std::string(pString.begin(), pString.end());
}

StreamDeckPhysicalDevice::StreamDeckPhysicalDevice(uint16_t pVendorID, uint16_t pProductID, const char* pSerial):
    mVendorID(pVendorID),
    mProductID(pProductID),
    mSerial(pSerial)
{
}

bool StreamDeckPhysicalDevice::Update()
{
    bool lUpdated = false;

    if ( mDevice == nullptr )
    {
        mDevice = hid_open(mVendorID, mProductID, ConvertStringToWString(mSerial).c_str());

        if(mDevice != nullptr)
        {
            std::cout << "Connect to " << mSerial << std::endl;
            lUpdated = true;
        }
    }
    else
    {
        int32_t lReportLength = hid_read_timeout(mDevice, mBuffer.data(), mBuffer.size(), 10);

        if( lReportLength == -1 )
        {
            std::wcout << L"Error: " << hid_error(mDevice) << std::endl;
            std::cout << L"Disconnect to " << mSerial << std::endl;
            mDevice = nullptr;
            lUpdated = true;
        }
        else if (lReportLength > 0 )
        {
            uint8_t lReportID = mBuffer[0];

            uint16_t lMessageType = uint32_t((mBuffer[1]) << 8) + mBuffer[2];

            if( lMessageType == 0x000F )//Maybe buttons count
            {
                for ( size_t i = 0 ; i < 15 ; ++i )
                {
                    bool lCurrentState = (mBuffer[4 + i] != 0);
                    if( mButtons[i] != lCurrentState )
                    {
                        mButtons[i] = lCurrentState;

                        //TODO: Call listener here
                        //ButtonChanged(i, lCurrentState);
                    }
                }
                lUpdated = true;
            }
            else
            {
                std::wcout << L"Unknown message type" << std::endl;
            }
        }
    }

    return lUpdated;
}

void StreamDeckPhysicalDevice::Reset()
{
    if(mDevice != nullptr )
    {
        struct ResetReport
        {
            uint8_t ReportID = 0x03;
            uint8_t MessageID = 0x02;
        };

        ResetReport lResetReport;

        hid_send_feature_report(mDevice, (uint8_t*)&lResetReport, sizeof(ResetReport));
    }
}

/**
 * \param[in] pBrightness brightness to set [0-100]
 */
void StreamDeckPhysicalDevice::SetBrightness(uint8_t pBrightness)
{
    if( mDevice != nullptr && pBrightness <= 100)
    {
        mBrightness = pBrightness;

        struct BrightnessReport
        {
            uint8_t ReportID = 0x03;
            uint8_t MessageID = 0x08;
            uint8_t Brightness;
        };

        BrightnessReport lReport;
        lReport.Brightness = mBrightness;

        hid_send_feature_report(mDevice, (unsigned char*)&lReport, sizeof(BrightnessReport));
    }
}

uint8_t StreamDeckPhysicalDevice::GetBrightness() const
{
    return mBrightness;
}

void StreamDeckPhysicalDevice::SetScreenSaverTime(uint32_t pTiming)
{
    if( mDevice != nullptr )
    {
        #pragma pack(push,1)
        struct ScreenSaveTimeReport
        {
            uint8_t ReportID = 0x03;
            uint8_t MessageID = 0x0D;
            uint32_t Timing;
        };
        #pragma pack(pop)

        ScreenSaveTimeReport lReport;
        lReport.Timing = pTiming;

        //No need to have complete packet size for feature
        hid_send_feature_report(mDevice, (unsigned char*)&lReport, sizeof(ScreenSaveTimeReport));
    }
}

void StreamDeckPhysicalDevice::SetImageFromPath(uint8_t pButtonIndex, const char* pFilepath)
{
    std::filesystem::path lFilepath(pFilepath);

    bool lExtensionValid = false;
    lExtensionValid |=  (lFilepath.extension() == ".jpg");
    lExtensionValid |=  (lFilepath.extension() == ".jpeg");
    lExtensionValid |=  (lFilepath.extension() == ".bmp");

    //TODO: need to check size here when the lib will be available
    if( lExtensionValid && mDevice != nullptr )
    {
        size_t lReportSize = 1024;
        std::vector<uint8_t> lReport;
        lReport.resize(lReportSize);

        memset(lReport.data(), 0, lReportSize);

        std::ifstream lFile(pFilepath, std::ios::binary | std::ios::ate);
        size_t lFileSize = lFile.tellg();
        lFile.seekg(0, std::ios_base::beg);

        size_t lRemainingBytesCount = lFileSize;

        struct PacketHeader
        {
            uint8_t ReportID;
            uint8_t MessageID;
            uint8_t KeyIndex;
            uint8_t LastPacket;
            uint16_t PacketSize;
            uint16_t PacketIndex;
        };

        PacketHeader& lHeader = *(PacketHeader*)lReport.data();

        lHeader.ReportID = 0x02;
        lHeader.MessageID = 0x07;
        lHeader.KeyIndex = pButtonIndex;
        lHeader.PacketIndex = 0;
        

        while (lRemainingBytesCount > 0)
        {
            lHeader.PacketSize = std::min(lReportSize-sizeof(PacketHeader), lRemainingBytesCount);

            lFile.read((char*)(lReport.data()+sizeof(PacketHeader)), lHeader.PacketSize);

            lRemainingBytesCount -= lHeader.PacketSize;

            lHeader.LastPacket = lRemainingBytesCount > 0 ? 0x00 : 0x01;

            //Need to have complete report size
            hid_write(mDevice, lReport.data(), lReportSize);

            lHeader.PacketIndex++;
        }

    }
}

void StreamDeckPhysicalDevice::SetImageFromSurface(uint8_t pButtonIndex, StreamDeckSurface* pSurface)
{
    size_t lDataSize = pSurface->GetJpegSize();

    if( lDataSize == 0 )
    {
        return;
    }

    uint8_t* lDataContent = pSurface->GetJpegData();

    size_t lReportSize = 1024;
    std::vector<uint8_t> lReport;
    lReport.resize(lReportSize);

    size_t lRemainingBytesCount = lDataSize;

    struct PacketHeader
    {
        uint8_t ReportID;
        uint8_t MessageID;
        uint8_t KeyIndex;
        uint8_t LastPacket;
        uint16_t PacketSize;
        uint16_t PacketIndex;
    };

    PacketHeader& lHeader = *(PacketHeader*)lReport.data();

    lHeader.ReportID = 0x02;
    lHeader.MessageID = 0x07;
    lHeader.KeyIndex = pButtonIndex;
    lHeader.PacketIndex = 0;

    char* lPacketDataPointer = (char*)(lReport.data()+sizeof(PacketHeader));
    size_t lMaxPacketCapacity = lReportSize-sizeof(PacketHeader);

    while (lRemainingBytesCount > 0)
    {
        lHeader.PacketSize = std::min(lMaxPacketCapacity, lRemainingBytesCount);

        memcpy(lPacketDataPointer, lDataContent + (lHeader.PacketIndex*lMaxPacketCapacity), lHeader.PacketSize);

        lRemainingBytesCount -= lHeader.PacketSize;

        lHeader.LastPacket = lRemainingBytesCount > 0 ? 0x00 : 0x01;

        //Need to have complete report size
        hid_write(mDevice, lReport.data(), lReportSize);

        lHeader.PacketIndex++;
    }
}

const char* StreamDeckPhysicalDevice::GetSerial() const
{
    return mSerial.c_str();
}

bool StreamDeckPhysicalDevice::GetButtonPressed(uint8_t pButtonIndex) const
{
    if( pButtonIndex < 15)
    {
        return mButtons[pButtonIndex];
    }
    return false;
}

void StreamDeckPhysicalDevice::ButtonChanged(uint32_t pIndex, bool pPressed)
{
    if (pIndex == 14 && !pPressed)
    {
        Reset();
    }

    if (pIndex == 0 && !pPressed)
    {
        //SetBlankImage(pIndex);
        SetBrightness(20);
    }

    if (pIndex == 1 && !pPressed)
    {
        //SetBlankImage(pIndex);
        SetBrightness(40);
    }

    if (pIndex == 2 && !pPressed)
    {
        //SetBlankImage(pIndex);
        SetBrightness(60);
    }

    if (pIndex == 3 && !pPressed)
    {
        //SetBlankImage(pIndex);
        SetBrightness(80);
    }

    if (pIndex == 4 && !pPressed)
    {
        //SetBlankImage(pIndex);
        SetBrightness(100);
    }

    if (pIndex == 5 && !pPressed)
    {
        //SetBlankImage(pIndex);
        SetScreenSaverTime(10);
    }

    if (pIndex == 9 && !pPressed)
    {
        //SetBlankImage(pIndex);
        SetScreenSaverTime(0);
    }
}
