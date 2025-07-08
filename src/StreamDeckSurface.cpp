#include "StreamDeckSurface.h"

#include <stdexcept>

#include <fstream>
#include <iostream>

#include <string>
#include <vector>

#include "StreamDeckImage.h"

struct StreamDeckSurfaceID
{
    StreamDeckImage& Image;
    size_t CurrentFrameIndex = 0;
    int32_t FrameStackedDisplayedDuration = 0;//milliseconds

    StreamDeckFrame* GetFrame(int32_t pFrameIndex)
    {
        return Image.GetFrame(pFrameIndex >= 0 ? pFrameIndex : CurrentFrameIndex);
    }
};

StreamDeckSurface::StreamDeckSurface(StreamDeckImage& pStreamDeckImage):
mID(new StreamDeckSurfaceID{
        pStreamDeckImage
    })
{
   ++mID->Image.ActiveInstances;
}

StreamDeckSurface::~StreamDeckSurface()
{
    --mID->Image.ActiveInstances;
    delete mID;
}

const StreamDeckImage& StreamDeckSurface::GetImage() const
{
    return mID->Image;
}

bool StreamDeckSurface::IsValid()
{
    return !mID->Image.Frames.empty();
}

bool StreamDeckSurface::Update(float pFrameDuration)
{
    bool lUpdated = false;

    StreamDeckFrame* lCurrentFrame = mID->Image.GetFrame(mID->CurrentFrameIndex);
    if(lCurrentFrame->Duration > 0 )
    {//OK we have an animated frame

        mID->FrameStackedDisplayedDuration += (pFrameDuration*1000);//Here we have millisecond resolution

        while (mID->FrameStackedDisplayedDuration > lCurrentFrame->Duration)
        {
            lUpdated = true;

            mID->FrameStackedDisplayedDuration -= lCurrentFrame->Duration;//Reset with sync frame timing

            //set next frame
            if( ++mID->CurrentFrameIndex >= mID->Image.Frames.size() )
            {
                mID->CurrentFrameIndex = 0;
            }

            lCurrentFrame = mID->Image.GetFrame(mID->CurrentFrameIndex);
        }
    }

    return lUpdated;
}

int32_t StreamDeckSurface::GetFrameCount()
{
    return int32_t(mID->Image.Frames.size());
}

size_t StreamDeckSurface::GetJpegSize(int32_t pFrameIndex)
{
    StreamDeckFrame* lFrame = mID->GetFrame(pFrameIndex);
    if( lFrame != nullptr )
    {
        return lFrame->JpgFileData.size();
    }

    return 0;
}

uint8_t* StreamDeckSurface::GetJpegData(int32_t pFrameIndex)
{
    StreamDeckFrame* lFrame = mID->GetFrame(pFrameIndex);
    if( lFrame != nullptr )
    {
        return lFrame->JpgFileData.data();
    }

    return nullptr;
}

size_t StreamDeckSurface::GetTexture(int32_t pFrameIndex)
{
    StreamDeckFrame* lFrame = mID->GetFrame(pFrameIndex);
    if( lFrame != nullptr )
    {
        return size_t(lFrame->Texture);
    }

    return 0;
}
