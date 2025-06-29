#include "StreamDeckSurface.h"

#include <stdexcept>

#include <fstream>
#include <iostream>

#include <string>
#include <vector>

#include <SDL3/SDL.h>
#include <turbojpeg.h>
#include <FreeImage.h>

#include "Providers.h"

enum class ImageType
{
    UKN,
    JPG,
    BMP,
    PNG,
    GIF
};

struct StreamDeckFrame
{   
    int32_t Duration = -1;//-1 means infinite
    std::vector<uint8_t> JpgFileData;
    SDL_Surface* Surface;
    SDL_Texture* Texture;
};


struct StreamDeckSurfaceID
{
    std::string Filepath;
    SdlResourcesProvider* SdlProvider;
    TurboJpegResourcesProvider* TjProvider;

    std::vector<StreamDeckFrame> Frames;

    void GenerateSurfaceFromJpg(std::vector<uint8_t>& pRawFileData)
    {
        int32_t lWidth = -1;
        int32_t lHeight = -1;

        if ( 0 != tjDecompressHeader(TjProvider->GetDecompressor(), pRawFileData.data(), pRawFileData.size(), &lWidth, &lHeight) )
        {
            std::cout << "jpeg header extraction failed" << std::endl;
        }
        else
        {
            StreamDeckFrame lFrame;

            lFrame.Surface = SDL_CreateSurface(lWidth, lHeight, SDL_PIXELFORMAT_BGRA32);

            if( 0 != tjDecompress2(TjProvider->GetDecompressor(), pRawFileData.data(), pRawFileData.size(), 
                                   (uint8_t*)lFrame.Surface->pixels, lFrame.Surface->w, lFrame.Surface->pitch, lFrame.Surface->h,
                                   TJPF_BGRA, TJFLAG_ACCURATEDCT))
            {
                std::cout << "jpeg decompression failed" << std::endl;
                SDL_DestroySurface(lFrame.Surface);
            }
            else
            {
                Frames.push_back(lFrame);
            }
        }
    }

    void GenerateSurfaceFromBmp(std::vector<uint8_t>& pRawFileData)
    {
        FIMEMORY* lFiMemory = FreeImage_OpenMemory(pRawFileData.data(), pRawFileData.size());

        FIBITMAP* lBitmap = FreeImage_LoadFromMemory(FIF_BMP, lFiMemory);
        FIBITMAP* lBitmapBGRA = FreeImage_ConvertTo32Bits(lBitmap);
        FreeImage_FlipVertical(lBitmapBGRA);

        StreamDeckFrame lFrame;
        lFrame.Surface = SDL_CreateSurface(FreeImage_GetWidth(lBitmapBGRA), FreeImage_GetHeight(lBitmapBGRA), SDL_PIXELFORMAT_BGRA32);
        memcpy(lFrame.Surface->pixels, FreeImage_GetBits(lBitmapBGRA), lFrame.Surface->h * lFrame.Surface->pitch);

        Frames.push_back(lFrame);

        //Free memory
        FreeImage_Unload(lBitmapBGRA);
        FreeImage_Unload(lBitmap);
        FreeImage_CloseMemory(lFiMemory);

        //IsValid = true;
    }

    void GenerateSurfaceFromGif(std::vector<uint8_t>& pRawFileData)
    {
        FIMEMORY* lFiMemory = FreeImage_OpenMemory(pRawFileData.data(), pRawFileData.size());
        FIMULTIBITMAP* lGifImage = FreeImage_LoadMultiBitmapFromMemory(FIF_GIF, lFiMemory);

        int32_t lImageCount = FreeImage_GetPageCount(lGifImage);

        for ( uint32_t i = 0 ; i < lImageCount ; ++i )
        {
            FIBITMAP* lFrameImage = FreeImage_LockPage(lGifImage, i);
            FIBITMAP* lFrameBGRA = FreeImage_ConvertTo32Bits(lFrameImage);
            FreeImage_FlipVertical(lFrameBGRA);
            

            StreamDeckFrame lFrame;
            lFrame.Surface = SDL_CreateSurface(FreeImage_GetWidth(lFrameBGRA), FreeImage_GetHeight(lFrameBGRA), SDL_PIXELFORMAT_BGRA32);
            memcpy(lFrame.Surface->pixels, FreeImage_GetBits(lFrameBGRA), lFrame.Surface->h * lFrame.Surface->pitch);

            Frames.push_back(lFrame);
            
            FreeImage_Unload(lFrameBGRA);
            FreeImage_UnlockPage(lGifImage, lFrameImage, false);
        }

        FreeImage_CloseMultiBitmap(lGifImage);
        FreeImage_CloseMemory(lFiMemory);
    }

    void GenerateInternalJpegData()
    {
        uint8_t* lBuffer = nullptr; 
        size_t lSize = 0;

        uint8_t* lTransformedBuffer[1] = {nullptr}; 
        size_t lTrasformedSize[1] = {0};
        tjtransform lTransform[1] = {
            tjtransform
            {
                {0,0,0,0},
                TJXOP_ROT180,
                TJXOPT_PERFECT,
                nullptr,
                nullptr
            }
        };

        for (StreamDeckFrame& lFrame: Frames)
        {
            tjCompress2(TjProvider->GetCompressor(), (uint8_t*)lFrame.Surface->pixels, lFrame.Surface->w, lFrame.Surface->pitch, lFrame.Surface->h, 
                        TJPF_BGRA, &lBuffer, &lSize, TJSAMP_444, 95, TJFLAG_ACCURATEDCT);

            tjTransform(TjProvider->GetTransformer(), lBuffer, lSize, 1, lTransformedBuffer, lTrasformedSize, lTransform, TJFLAG_ACCURATEDCT);

            lFrame.JpgFileData = std::vector<uint8_t>(lTransformedBuffer[0], lTransformedBuffer[0]+lTrasformedSize[0]);
        }

        tjFree(lBuffer);
        tjFree(lTransformedBuffer[0]);
    }
};

ImageType GetImageType(std::vector<uint8_t>& pFileData)
{
    size_t lFileSize = pFileData.size();
    if( lFileSize > 6)
    {//we check BMP here
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

    std::vector<uint8_t> lRawFileData;
    ImageType lImageType = ImageType::UKN;

    //TODO: check here if file is jpeg and valid
    lRawFileData.resize(lFileSize);
    lFile.read((char*)lRawFileData.data(), lFileSize);

    try 
    {
        lImageType = GetImageType(lRawFileData);
    }
    catch (const std::domain_error& pException)
    {
        std::cout << "Error loading " << pFilepath << ": " << pException.what() << std::endl;
    }

    switch (lImageType)
    {
        case ImageType::JPG:
            mID->GenerateSurfaceFromJpg(lRawFileData);
            break;

        case ImageType::BMP:
            mID->GenerateSurfaceFromBmp(lRawFileData);
            break;

        case ImageType::GIF:
            mID->GenerateSurfaceFromGif(lRawFileData);
            break;
    }

    mID->GenerateInternalJpegData();
    //TODO: move to Internal data method
    for (StreamDeckFrame& lFrame: mID->Frames)
    {
        lFrame.Texture = SDL_CreateTextureFromSurface(mID->SdlProvider->GetSdlRenderer(), lFrame.Surface);
    }
}

StreamDeckSurface::~StreamDeckSurface()
{
    for (StreamDeckFrame& lFrame: mID->Frames)
    {
        SDL_DestroyTexture(lFrame.Texture);
        SDL_DestroySurface(lFrame.Surface);
    }
    
    delete mID;
}

size_t StreamDeckSurface::GetTexture()
{
    if ( mID->Frames.empty() )
    {
        return 0;
    }

    return (size_t)mID->Frames[0].Texture;
}

bool StreamDeckSurface::IsValid()
{
    return !mID->Frames.empty();
}

size_t StreamDeckSurface::GetFrameCount()
{
    return mID->Frames.size();
}

size_t StreamDeckSurface::GetJpegSize(size_t pFrameIndex)
{
    if( pFrameIndex < mID->Frames.size() )
    {
        return mID->Frames[pFrameIndex].JpgFileData.size();
    }
    return 0;
}

uint8_t* StreamDeckSurface::GetJpegData(size_t pFrameIndex)
{
    if( pFrameIndex < mID->Frames.size() )
    {
        return mID->Frames[pFrameIndex].JpgFileData.data();
    }
    return nullptr;
}
