#include "StreamDeckSurfaceProvider.h"

#include "Providers.h"
#include "StreamDeckSurface.h"
#include "StreamDeckImage.h"

#include <SDL3/SDL.h>
#include <turbojpeg.h>
#include <FreeImage.h>

#include <string>
#include <unordered_map>

#include <fstream>
#include <iostream>

enum class ImageType
{
    UKN,
    JPG,
    BMP,
    PNG,
    GIF
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

struct StreamDeckSurfaceProviderId
{
    SdlResourcesProvider* SdlProvider = nullptr;
    tjhandle CompressorInstance = nullptr;
    tjhandle DecompressorInstance = nullptr;
    tjhandle TransformerInstance = nullptr;

    std::unordered_map<std::string, StreamDeckImage> Images;

    //TODO: create remove file
    void AddFile(const char* pFilepath)
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

        StreamDeckImage lImage;

        switch (lImageType)
        {
            case ImageType::JPG:
                lImage = GenerateImageFromJpg(lRawFileData);
                break;

            case ImageType::BMP:
                lImage = GenerateImageFromBmp(lRawFileData);
                break;

            case ImageType::PNG:
                lImage = GenerateImageFromPng(lRawFileData);
                break;

            case ImageType::GIF:
                lImage = GenerateImageFromGif(lRawFileData);
                break;

            default:
            return;
        }

        GenerateInternalJpegData(lImage);
        //TODO: move to Internal data method
        for (StreamDeckFrame& lFrame: lImage.Frames)
        {
            lFrame.Texture = SDL_CreateTextureFromSurface(SdlProvider->GetSdlRenderer(), lFrame.Surface);
        }

        Images[pFilepath] = lImage;
    }

    StreamDeckImage GenerateImageFromJpg(std::vector<uint8_t>& pRawFileData)
    {
        StreamDeckImage lImage;

        int32_t lWidth = -1;
        int32_t lHeight = -1;

        if ( 0 != tjDecompressHeader(DecompressorInstance, pRawFileData.data(), pRawFileData.size(), &lWidth, &lHeight) )
        {
            std::cout << "jpeg header extraction failed" << std::endl;
        }
        else
        {
            StreamDeckFrame lFrame;

            lFrame.Surface = SDL_CreateSurface(lWidth, lHeight, SDL_PIXELFORMAT_BGRA32);

            if( 0 != tjDecompress2(DecompressorInstance, pRawFileData.data(), pRawFileData.size(), 
                                   (uint8_t*)lFrame.Surface->pixels, lFrame.Surface->w, lFrame.Surface->pitch, lFrame.Surface->h,
                                   TJPF_BGRA, TJFLAG_ACCURATEDCT))
            {
                std::cout << "jpeg decompression failed" << std::endl;
                SDL_DestroySurface(lFrame.Surface);
            }
            else
            {
                lImage.Frames.push_back(lFrame);
            }
        }
        return lImage;
    }

    StreamDeckImage GenerateImageFromBmp(std::vector<uint8_t>& pRawFileData)
    {
        StreamDeckImage lImage;
        FIMEMORY* lFiMemory = FreeImage_OpenMemory(pRawFileData.data(), pRawFileData.size());

        FIBITMAP* lBitmap = FreeImage_LoadFromMemory(FIF_BMP, lFiMemory);
        FIBITMAP* lBitmapBGRA = FreeImage_ConvertTo32Bits(lBitmap);
        FreeImage_FlipVertical(lBitmapBGRA);

        StreamDeckFrame lFrame;
        lFrame.Surface = SDL_CreateSurface(FreeImage_GetWidth(lBitmapBGRA), FreeImage_GetHeight(lBitmapBGRA), SDL_PIXELFORMAT_BGRA32);
        memcpy(lFrame.Surface->pixels, FreeImage_GetBits(lBitmapBGRA), lFrame.Surface->h * lFrame.Surface->pitch);

        lImage.Frames.push_back(lFrame);

        //Free memory
        FreeImage_Unload(lBitmapBGRA);
        FreeImage_Unload(lBitmap);
        FreeImage_CloseMemory(lFiMemory);
        return lImage;
    }

    StreamDeckImage GenerateImageFromPng(std::vector<uint8_t>& pRawFileData)
    {
        StreamDeckImage lImage;
        FIMEMORY* lFiMemory = FreeImage_OpenMemory(pRawFileData.data(), pRawFileData.size());

        FIBITMAP* lBitmap = FreeImage_LoadFromMemory(FIF_PNG, lFiMemory);
        FIBITMAP* lBitmapBGRA = FreeImage_ConvertTo32Bits(lBitmap);
        FreeImage_FlipVertical(lBitmapBGRA);

        StreamDeckFrame lFrame;
        lFrame.Surface = SDL_CreateSurface(FreeImage_GetWidth(lBitmapBGRA), FreeImage_GetHeight(lBitmapBGRA), SDL_PIXELFORMAT_BGRA32);
        memcpy(lFrame.Surface->pixels, FreeImage_GetBits(lBitmapBGRA), lFrame.Surface->h * lFrame.Surface->pitch);

        lImage.Frames.push_back(lFrame);

        //Free memory
        FreeImage_Unload(lBitmapBGRA);
        FreeImage_Unload(lBitmap);
        FreeImage_CloseMemory(lFiMemory);
        return lImage;
    }

    StreamDeckImage GenerateImageFromGif(std::vector<uint8_t>& pRawFileData)
    {
        StreamDeckImage lImage;

        FIMEMORY* lFiMemory = FreeImage_OpenMemory(pRawFileData.data(), pRawFileData.size());
        FIMULTIBITMAP* lGifImage = FreeImage_LoadMultiBitmapFromMemory(FIF_GIF, lFiMemory, GIF_PLAYBACK);

        int32_t lImageCount = FreeImage_GetPageCount(lGifImage);

        for ( uint32_t i = 0 ; i < lImageCount ; ++i )
        {
            FIBITMAP* lFrameImage = FreeImage_LockPage(lGifImage, i);
            FIBITMAP* lFrameBGRA = FreeImage_ConvertTo32Bits(lFrameImage);
            FreeImage_FlipVertical(lFrameBGRA);

            FITAG* lTag = nullptr;
            StreamDeckFrame lFrame;

            if( FreeImage_GetMetadata(FIMD_ANIMATION, lFrameImage, "FrameTime", &lTag) )
            {
                lFrame.Duration = *(uint32_t*)FreeImage_GetTagValue(lTag);
            }
            else
            {//Animation time is missing on frame metadata
                lFrame.Duration = 100;//100 ms appears to be the default duration
            }

            lFrame.Surface = SDL_CreateSurface(FreeImage_GetWidth(lFrameBGRA), FreeImage_GetHeight(lFrameBGRA), SDL_PIXELFORMAT_BGRA32);
            memcpy(lFrame.Surface->pixels, FreeImage_GetBits(lFrameBGRA), lFrame.Surface->h * lFrame.Surface->pitch);

            lImage.Frames.push_back(lFrame);
            
            FreeImage_Unload(lFrameBGRA);
            FreeImage_UnlockPage(lGifImage, lFrameImage, false);
        }

        FreeImage_CloseMultiBitmap(lGifImage);
        FreeImage_CloseMemory(lFiMemory);
        return lImage;
    }

    void GenerateInternalJpegData(StreamDeckImage& pStreamDeckImage)
    {
        uint8_t* lBuffer = nullptr; 
        unsigned long lSize = 0;

        uint8_t* lTransformedBuffer[1] = {nullptr}; 
        unsigned long lTransformedSize[1] = {0};
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

        for (StreamDeckFrame& lFrame: pStreamDeckImage.Frames)
        {
            tjCompress2(CompressorInstance, (uint8_t*)lFrame.Surface->pixels, lFrame.Surface->w, lFrame.Surface->pitch, lFrame.Surface->h, 
                        TJPF_BGRA, &lBuffer, &lSize, TJSAMP_444, 95, TJFLAG_ACCURATEDCT);

            tjTransform(TransformerInstance, lBuffer, lSize, 1, lTransformedBuffer, lTransformedSize, lTransform, TJFLAG_ACCURATEDCT);

            lFrame.JpgFileData = std::vector<uint8_t>(lTransformedBuffer[0], lTransformedBuffer[0]+ lTransformedSize[0]);
        }

        tjFree(lBuffer);
        tjFree(lTransformedBuffer[0]);
    }
};

StreamDeckSurfaceProvider::StreamDeckSurfaceProvider(SdlResourcesProvider* pSdlResourcesProvider):
mId(new StreamDeckSurfaceProviderId
    {
        pSdlResourcesProvider,
        tjInitCompress(),
        tjInitDecompress(),
        tjInitTransform()
    })
{
}

StreamDeckSurfaceProvider::~StreamDeckSurfaceProvider()
{
    delete mId;
}

StreamDeckSurface* StreamDeckSurfaceProvider::GetSurface(const char* pFilepath)
{
    std::unordered_map<std::string, StreamDeckImage>::iterator lImagesIterator = mId->Images.find(pFilepath);
    if (lImagesIterator == mId->Images.end())
    {//The image doesn't exists
        mId->AddFile(pFilepath);
        lImagesIterator = mId->Images.find(pFilepath);
    }
        
    if (lImagesIterator != mId->Images.end())
    {
        return new StreamDeckSurface(lImagesIterator->second);
    }

    return nullptr;
}

void StreamDeckSurfaceProvider::ReleaseSurface(StreamDeckSurface* pStreamDeckSurface)
{
    std::string pFilepath = pStreamDeckSurface->GetImage().Filepath;
    delete pStreamDeckSurface;

    if ( mId->Images[pFilepath].ActiveInstances == 0 )
    {
        for (StreamDeckFrame& lFrame: mId->Images[pFilepath].Frames )
        {
            SDL_DestroyTexture(lFrame.Texture);
            SDL_DestroySurface(lFrame.Surface);
        }

        mId->Images.erase(pFilepath);
    }
}
