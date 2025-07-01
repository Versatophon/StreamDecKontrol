#pragma once

#include <vector>

class StreamDeckPhysicalDevice;
class FileDropProvider;
class StreamDeckSurfaceProvider;

class StreamDeckSurface;

class StreamDeckDevice
{
public:
    StreamDeckDevice(const char* pDeviceSerial, StreamDeckSurfaceProvider* pStreamDeckSurfaceProvider, FileDropProvider* pFileDropProvider);
    ~StreamDeckDevice();

    void SetButtonImage(size_t pButtonIndex, const char* pFilepath);

    bool Update(float pFrameDuration);

    void DisplayDeviceTab();

private:
    StreamDeckPhysicalDevice* mPhysicalDevice = nullptr;

    StreamDeckSurfaceProvider* mStreamDeckSurfaceProvider = nullptr;

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
