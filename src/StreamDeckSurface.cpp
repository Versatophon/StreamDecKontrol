#include "StreamDeckSurface.h"

#include <stdexcept>

#include <fstream>
#include <iostream>

#include <string>
#include <vector>

#include <SDL3/SDL.h>
#include <turbojpeg.h>

#include "Providers.h"

enum class ImageType
{
    UKN,
    JPG,
    BMP,
    PNG,
    GIF
};

struct StreamDeckSurfaceID
{
    std::string Filepath;
    SdlResourcesProvider* SdlProvider;
    TurboJpegResourcesProvider* TjProvider;
    ImageType RawType = ImageType::UKN;
    std::vector<uint8_t> RawFileData;

    bool IsValid = false;

    SDL_Surface* Surface;
    SDL_Texture* Texture;
};

ImageType GetImageType(std::vector<uint8_t>& pFileData)
{
    size_t lFileSize = pFileData.size();
    if( lFileSize > 6)
    {//we check BMP here
        //if( pFileData[0] == 'B' && pFileData[1] == 'M')
        if( *(uint16_t*)pFileData.data() == 0x4D42/*'BM'*/ )
        {
            size_t lSizeFromHeader = size_t(*(uint32_t*)(pFileData.data()+2));
            if( lSizeFromHeader != lFileSize)
            {
                throw std::domain_error("BMP size is not consistent!");
            }
            else
            {
                return ImageType::BMP;
            }
        }

        //GIF89a
        if(*(uint32_t*)pFileData.data() == 0x38464947/*GIF8*/)
        {
            if(pFileData[5] == 'a' && (pFileData[4] == '7' || pFileData[4] == '9'))
            {
                return ImageType::GIF;
            }
            else
            {
                throw std::domain_error("Invalid GIF signature!");
            }
        }
    }

    if (lFileSize > 8)
    {
        if (*(uint64_t*)pFileData.data() == 0x0A1A0A0D474E5089 /*PNG signature*/ )
        {
            return ImageType::PNG;
        }
    }

    if (lFileSize > 10)
    {
        if ( *(uint32_t*)pFileData.data() == 0xE0FFD8FF /*JPEG 1st block bytes*/ && *(uint32_t*)(pFileData.data()+6) == 0x4649464A /* JFIF Signature */)
        {
            return ImageType::JPG;
        }
    }
    

    throw std::domain_error("Unsupported File!");
}

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
    mID->RawFileData.resize(lFileSize);
    lFile.read((char*)mID->RawFileData.data(), lFileSize);

    try 
    {
        mID->RawType = GetImageType(mID->RawFileData);
    }
    catch (const std::domain_error& pException)
    {
        std::cout << "Error loading " << pFilepath << ": " << pException.what() << std::endl;
    }

    if( mID->RawType == ImageType::JPG )
    {
        int32_t lWidth = -1;
        int32_t lHeight = -1;

        if ( 0 != tjDecompressHeader(mID->TjProvider->GetDecompressor(), mID->RawFileData.data(), mID->RawFileData.size(), &lWidth, &lHeight) )
        {
            std::cout << "jpeg header extraction failed" << std::endl;
        }
        else
        {
            mID->Surface = SDL_CreateSurface(lWidth, lHeight, SDL_PIXELFORMAT_RGBA32);
            //lImage.Content.resize(lImage.Width * lImage.Height * 4);

            if( 0 != tjDecompress2(mID->TjProvider->GetDecompressor(), mID->RawFileData.data(), mID->RawFileData.size(), (uint8_t*)mID->Surface->pixels, 
                                    mID->Surface->w, mID->Surface->pitch, mID->Surface->h, TJPF_RGBA, TJFLAG_ACCURATEDCT))
            {
                std::cout << "jpeg decompression failed" << std::endl;
            }
        }

        mID->Texture = SDL_CreateTextureFromSurface(mID->SdlProvider->GetSdlRenderer(), mID->Surface);

        mID->IsValid = true;
    }
    
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

bool StreamDeckSurface::IsValid()
{
    return mID->IsValid;
}
  