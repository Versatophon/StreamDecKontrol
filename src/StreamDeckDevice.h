#pragma once

#include <vector>

class StreamDeckPhysicalDevice;
class FileDropProvider;
class TurboJpegResourcesProvider;
class SdlResourcesProvider;

class StreamDeckSurface;

class StreamDeckDevice
{
public:
    StreamDeckDevice(const char* pDeviceSerial, SdlResourcesProvider* pSdlResourcesProvider, TurboJpegResourcesProvider* pTurboJpegResourcesProvider, FileDropProvider* pFileDropProvider);
    ~StreamDeckDevice();

    bool Update(float pFrameDuration);

    void DisplayDeviceTab();

private:
    StreamDeckPhysicalDevice* mPhysicalDevice = nullptr;
    SdlResourcesProvider* mSdlResourcesProvider;
    TurboJpegResourcesProvider* mTurboJpegResourcesProvider;
    FileDropProvider* mFileDropProvider = nullptr;


    std::vector<StreamDeckSurface*> mStreamDeckSurfaces;
    void ReplaceSurface(size_t pButtonIndex, StreamDeckSurface* pStreamDeckSurface);

    /**
     * Display a button with an image
     * 
     * \param[in] pPressed Set it to true if hardware button is pressed
     * \return True if mouse cursor is above 
     */
    bool DisplayButton(bool pPressed, StreamDeckSurface* pImage);
};
