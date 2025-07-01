#pragma once

#include <cstdint>
#include <cstddef>

struct StreamDeckSurfaceID;

class StreamDeckImage;

class StreamDeckSurface
{
public:
    StreamDeckSurface(StreamDeckImage& pStreamDeckImage);
    ~StreamDeckSurface();

    const StreamDeckImage& GetImage() const;
        
    bool IsValid();
    
    /**
     * @brief Updates surface if content changed (animation)
     * 
     * @param pFrameDuration duration since last call
     * @return true if a change occured
     * @return false otherwise
     */
    bool Update(float pFrameDuration);
    
    int32_t GetFrameCount();
    size_t GetJpegSize(int32_t pFrameIndex = -1);
    uint8_t* GetJpegData(int32_t pFrameIndex = -1);
    size_t GetTexture(int32_t pFrameIndex = -1);
    
private:
    StreamDeckSurfaceID* mID = nullptr;
};
