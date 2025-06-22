#include "StreamDeckSurface.h"

#include <fstream>
#include <iostream>

#include <string>
#include <vector>

#include <SDL3/SDL.h>
#include <turbojpeg.h>

#include "Providers.h"

struct StreamDeckSurfaceID
{
    std::string Filepath;
    SdlResourcesProvider* SdlProvider;
    TurboJpegResourcesProvider* TjProvider;
    std::vector<uint8_t> JpegData;
    SDL_Surface* Surface;
    SDL_Texture* Texture;
};

StreamDeckSurface::StreamDeckSurface(const char* pFilepath, SdlResourcesProvider* pSdlResourcesProvider, TurboJpegResourcesProvider* pTurboJpegResourcesProvider):
    mID(new StreamDeckSurfaceID{
        pFilepath,
        pSdlResourcesProvider,
        pTurboJpegResourcesProvider
    })
{
    std::ifstream lFile(pFilepath, std::ios::binary | std::ios::ate);
    size_t lFileSize = lFile.tellg();
    lFile.seekg(0, std::ios_base::beg);

    //TODO: check here if file is jpeg and valid
    mID->JpegData.resize(lFileSize);

    lFile.read((char*)mID->JpegData.data(), lFileSize);

    int32_t lWidth = -1;
    int32_t lHeight = -1;

    if ( 0 != tjDecompressHeader(mID->TjProvider->GetDecompressor(), mID->JpegData.data(), mID->JpegData.size(), &lWidth, &lHeight) )
    {
        std::cout << "jpeg header extraction failed" << std::endl;
    }
    else
    {
        mID->Surface = SDL_CreateSurface(lWidth, lHeight, SDL_PIXELFORMAT_RGBA32);
        //lImage.Content.resize(lImage.Width * lImage.Height * 4);

        if( 0 != tjDecompress2(mID->TjProvider->GetDecompressor(), mID->JpegData.data(), mID->JpegData.size(), (uint8_t*)mID->Surface->pixels, 
                                mID->Surface->w, mID->Surface->pitch, mID->Surface->h, TJPF_RGBA, TJFLAG_ACCURATEDCT))
        {
            std::cout << "jpeg decompression failed" << std::endl;
        }
    }

    mID->Texture = SDL_CreateTextureFromSurface(mID->SdlProvider->GetSdlRenderer(), mID->Surface);
}

StreamDeckSurface::~StreamDeckSurface()
{
    SDL_DestroyTexture(mID->Texture);
    SDL_DestroySurface(mID->Surface);

    delete mID;
}

size_t StreamDeckSurface::GetTexture()
{
    return (size_t)mID->Texture;
}
  