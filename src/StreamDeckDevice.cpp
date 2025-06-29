#include "StreamDeckDevice.h"

#include "StreamDeckSurface.h"
#include "StreamDeckPhysicalDevice.h"
#include "FileDropProvider.h"

#include "externals/imgui/imgui.h"

StreamDeckDevice::StreamDeckDevice(const char* pDeviceSerial, SdlResourcesProvider* pSdlResourcesProvider, TurboJpegResourcesProvider* pTurboJpegResourcesProvider, FileDropProvider* pFileDropProvider):
    mPhysicalDevice(new StreamDeckPhysicalDevice(0x0fd9, 0x006d, pDeviceSerial)),
    mSdlResourcesProvider(pSdlResourcesProvider),
    mTurboJpegResourcesProvider(pTurboJpegResourcesProvider),
    mFileDropProvider(pFileDropProvider)
{
    mStreamDeckSurfaces = std::vector<StreamDeckSurface*>(15, nullptr);
}
    
StreamDeckDevice::~StreamDeckDevice()
{
    delete mPhysicalDevice;
}

bool StreamDeckDevice::Update(float pFrameDuration)
{
    bool lUpdated = false;
    for(size_t i = 0 ; i < mStreamDeckSurfaces.size() ; ++i)
    {
        StreamDeckSurface* lSurface = mStreamDeckSurfaces[i];

        if( lSurface!= nullptr && lSurface->Update(pFrameDuration) )
        {
            mPhysicalDevice->SetImageFromSurface(i, lSurface);
            lUpdated = true;
        }
    }

    lUpdated |= mPhysicalDevice->Update();

    return lUpdated;
}

void StreamDeckDevice::DisplayDeviceTab()
{
    std::string lSerial = mPhysicalDevice->GetSerial();
    if ( ImGui::BeginTabItem(lSerial.c_str()))
    {
        if( ImGui::Button(("Reset##" + lSerial).c_str()) )
        {
            mPhysicalDevice->Reset();
        }

        ImGui::SameLine();

        uint8_t lMin = 0;
        uint8_t lMax = 100;
        uint8_t lBrightness = mPhysicalDevice->GetBrightness();

        if( ImGui::SliderScalar("Brightness", ImGuiDataType_U8, &lBrightness, &lMin, &lMax, nullptr) )
        {
            mPhysicalDevice->SetBrightness(lBrightness);
        }

        uint8_t lButtonIndex = 0;
        for (size_t lRow = 0 ; lRow < 3 ; ++lRow)
        {
            for (size_t lCol = 0 ; lCol < 5 ; ++lCol)
            {
                bool lButtonPressed = mPhysicalDevice->GetButtonPressed(lButtonIndex);

                if( DisplayButton(lButtonPressed, mStreamDeckSurfaces[lButtonIndex]) )
                {

                    const char* lFilepath;
                    while( (lFilepath = mFileDropProvider->GetQueuedFilepath()) != nullptr )
                    {//if mouse is above this button and we have a file drop queue
                        StreamDeckSurface* lStreamDeckSurface = new StreamDeckSurface(lFilepath, mSdlResourcesProvider, mTurboJpegResourcesProvider);
                        if( lStreamDeckSurface->IsValid() )
                        {
                            ReplaceSurface(lButtonIndex, lStreamDeckSurface);
                        }
                        else
                        {
                            delete lStreamDeckSurface;
                        }
                    }
                }

                ++lButtonIndex;

                if( (lButtonIndex % 5) != 0 )
                {
                    ImGui::SameLine();
                }
            }
        }

        ImGui::EndTabItem();
    }
}

void StreamDeckDevice::ReplaceSurface(size_t pButtonIndex, StreamDeckSurface* pStreamDeckSurface)
{
    if( mStreamDeckSurfaces[pButtonIndex] != nullptr)
    {
        delete mStreamDeckSurfaces[pButtonIndex];
    }

    mStreamDeckSurfaces[pButtonIndex] = pStreamDeckSurface;

    mPhysicalDevice->SetImageFromSurface(pButtonIndex, pStreamDeckSurface);
}

#define BORDER_WIDTH 1
#define IMG_SIZE 72
#define CROSS_MARGIN 10

bool StreamDeckDevice::DisplayButton(bool pPressed, StreamDeckSurface* pImage)
{
    uint32_t lMargin = 20;
    ImDrawList* lDrawList = ImGui::GetWindowDrawList();

    ImVec2 lItemBeginPos = ImGui::GetCursorPos();

    ImVec2 lButtonBeginPos = lItemBeginPos;
    lButtonBeginPos.x += BORDER_WIDTH;
    lButtonBeginPos.y += BORDER_WIDTH;

    ImVec2 lButtonEndPos = lButtonBeginPos;
    lButtonEndPos.x += IMG_SIZE;
    lButtonEndPos.y += IMG_SIZE;

    ImVec2 lItemEndPos = lButtonEndPos;
    lItemEndPos.x += BORDER_WIDTH;
    lItemEndPos.y += BORDER_WIDTH;

    bool lMayDropSomething = mFileDropProvider->DropPending();
    bool lMouseIsAbove = true;

    lMouseIsAbove &= (mFileDropProvider->DropPositionX() >= lButtonBeginPos.x);
    lMouseIsAbove &= (mFileDropProvider->DropPositionX() <= lButtonEndPos.x);

    lMouseIsAbove &= (mFileDropProvider->DropPositionY() >= lButtonBeginPos.y);
    lMouseIsAbove &= (mFileDropProvider->DropPositionY() <= lButtonEndPos.y);

    lMayDropSomething &= lMouseIsAbove;

    if( pImage != nullptr)
    {
        ImTextureRef lTextureRef((ImTextureID)(intptr_t)pImage->GetTexture());

        lDrawList->AddImageRounded(lTextureRef, lButtonBeginPos, lButtonEndPos, {0,0}, {1,1}, ImColor(ImVec4{1.f, 1.f, 1.f, 1.f}), 10);
    }
    
    lDrawList->AddRect(lItemBeginPos, lItemEndPos, ImColor(pPressed?ImVec4{0.21f, 0.75f, 0.21f, 1.f}:ImVec4{0.75f, 0.75f, 0.75f, 1.f}),
                       10, 0, pPressed?2.f:1.f);


    if (lMayDropSomething)
    {
        lDrawList->AddLine(ImVec2(lButtonBeginPos.x + IMG_SIZE/2, lButtonBeginPos.y + CROSS_MARGIN), ImVec2(lButtonBeginPos.x + IMG_SIZE/2, lButtonBeginPos.y + IMG_SIZE - CROSS_MARGIN), ImColor(ImVec4{0.75f, 0.75f, 0.75f, 1.f}), 5.f);
        lDrawList->AddLine(ImVec2(lButtonBeginPos.x + CROSS_MARGIN, lButtonBeginPos.y + IMG_SIZE/2), ImVec2(lButtonBeginPos.x + IMG_SIZE - CROSS_MARGIN, lButtonBeginPos.y + IMG_SIZE/2), ImColor(ImVec4{0.75f, 0.75f, 0.75f, 1.f}), 5.f);
    }

    ImGui::Dummy(ImVec2(IMG_SIZE+BORDER_WIDTH*2 + lMargin, IMG_SIZE+BORDER_WIDTH*2 + lMargin + 5));

    return lMouseIsAbove;
}
