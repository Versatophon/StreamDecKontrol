#pragma once

#include <string>
#include <vector>

#include <cstdint>

typedef struct SDL_Surface SDL_Surface;
typedef struct SDL_Texture SDL_Texture;

struct StreamDeckFrame
{   
    int32_t Duration = -1;//-1 means infinite
    int32_t StackedDisplayedDuration = 0;
    std::vector<uint8_t> JpgFileData;
    SDL_Surface* Surface;
    SDL_Texture* Texture;
};

struct StreamDeckImage
{
    std::string Filepath;
    std::vector<StreamDeckFrame> Frames;
    size_t ActiveInstances = 0;

    inline StreamDeckFrame* GetFrame(uint32_t pFrameIndex)
    {
        if( pFrameIndex < Frames.size() )
        {
            return &Frames[pFrameIndex];
        }

        return nullptr;
    }
};

