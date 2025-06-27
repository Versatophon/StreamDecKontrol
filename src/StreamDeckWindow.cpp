#include "StreamDeckWindow.h"

#include <SDL3/SDL.h>

#include <turbojpeg.h>

#include <iostream>
#include <fstream>

#include <unordered_set>
#include <list>

#include "externals/imgui/imgui.h"

#include "StreamDeckSurface.h"


StreamDeckWindow::StreamDeckWindow(): ManagedWindow(0, nullptr),
mCompressorInstance(tjInitCompress()),
mDecompressorInstance(tjInitDecompress())
{
    mButtonImages.resize(15, nullptr);
}
 
StreamDeckWindow::~StreamDeckWindow()
{
    tjDestroy(mCompressorInstance);
    tjDestroy(mDecompressorInstance);
}

tjhandle StreamDeckWindow::GetCompressor()
{
    return mCompressorInstance;
}

tjhandle StreamDeckWindow::GetDecompressor()
{
    return mDecompressorInstance;
}

int32_t StreamDeckWindow::Init()
{
    int32_t lRes = hid_init();


    return SDL_APP_CONTINUE;
}

int32_t StreamDeckWindow::Event(SDL_Event *pEvent)
{
    switch ( pEvent->type )
    {
        case SDL_EVENT_DROP_BEGIN:
            std::cout << "Begin: " << std::endl;
            mIsDroppingSomething = true;
        break;

        case SDL_EVENT_DROP_COMPLETE:
            std::cout << "Complete: " << std::endl;
            mIsDroppingSomething = false;
        break;

        case SDL_EVENT_DROP_FILE:
            std::cout << "File dropped: " << pEvent->drop.data << std::endl;
            mDroppedFileQueue.push(pEvent->drop.data);
        break;

        case SDL_EVENT_DROP_POSITION:
            mDropPositionX = pEvent->drop.x;
            mDropPositionY = pEvent->drop.y;
            //std::cout << "File moving: " << pEvent->drop.x << ";" << pEvent->drop.y << std::endl;

        
        //std::cout << "File data: " << pEvent->drop.data << std::endl;
        break;

    }

    return SDL_APP_CONTINUE;
}

int32_t StreamDeckWindow::Iterate()
{
    //Setup
    //TODO: may need to perform this less often
    EnumerateDevices();
    
    //mCounter++;

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove);

    if( ImGui::BeginTabBar("bar") )
    {
        for( std::pair<std::string, StreamDeckRawDevice*> mDevicePair :mDevicesMap)
        {
            mDevicePair.second->Update();
            std::string lDeviceSerial = mDevicePair.first;
            //ImGui::Text(lDeviceSerial.c_str());
            
            DisplayDeviceTab(mDevicePair.second);
            

        }
       
            ImGui::EndTabBar();
    }

    //ImGui::DragInt("Compteur", &mCounter);

    ImGui::End();

    return SDL_APP_CONTINUE;
}

void StreamDeckWindow::Quit()
{    
    int32_t lRes = hid_exit();
}

void StreamDeckWindow::DisplayDeviceTab(StreamDeckRawDevice* pDevice)
{
    std::string lSerial = pDevice->GetSerial();
    if ( ImGui::BeginTabItem(lSerial.c_str()))
    {
        if( ImGui::Button(("Reset##" + lSerial).c_str()) )
        {
            pDevice->Reset();
        }

        ImGui::SameLine();

        uint8_t lMin = 0;
        uint8_t lMax = 100;
        uint8_t lBrightness = pDevice->GetBrightness();

        if( ImGui::SliderScalar("Brightness", ImGuiDataType_U8, &lBrightness, &lMin, &lMax, nullptr) )
        {
            pDevice->SetBrightness(lBrightness);
        }

        uint8_t lButtonIndex = 0;
        for (size_t lRow = 0 ; lRow < 3 ; ++lRow)
        {
            for (size_t lCol = 0 ; lCol < 5 ; ++lCol)
            {
                bool lButtonPressed = pDevice->GetButtonPressed(lButtonIndex);

                if( DisplayButton(lButtonPressed, mButtonImages[lButtonIndex]) )
                {
                    while( !mDroppedFileQueue.empty())
                    {//if mouse is above this button and we have a file drop queue
                        std::string lFilePath = mDroppedFileQueue.front();mDroppedFileQueue.pop();
                        SetImage(pDevice, lButtonIndex, lFilePath.c_str());
                    }
                }

                ++lButtonIndex;

                if( (lButtonIndex % 5) != 0 )
                {
                    ImGui::SameLine();
                }
            }
        }

        //flush file queue in order to not stack filepathes for the next drop
        while(!mDroppedFileQueue.empty())
        {
            mDroppedFileQueue.pop();
        }

        ImGui::EndTabItem();
    }
}

void StreamDeckWindow::SetImage(StreamDeckRawDevice* pDevice, uint8_t pButtonId, const char* pImagePath)
{

    StreamDeckSurface* lStreamDeckSurface = new StreamDeckSurface(pImagePath, this, this);
    if( lStreamDeckSurface->IsValid() )
    {
        mButtonImages[pButtonId] = lStreamDeckSurface;
        //pDevice->SetImageFromPath(pButtonId, pImagePath);
        pDevice->SetImageFromSurface(pButtonId, lStreamDeckSurface);
    }
    else
    {
        delete lStreamDeckSurface;
    }

}

#define BORDER_WIDTH 1
#define IMG_SIZE 72
#define CROSS_MARGIN 10

bool StreamDeckWindow::DisplayButton(bool pPressed, StreamDeckSurface* pImage)
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

    bool lMayDropSomething = mIsDroppingSomething;
    bool lMouseIsAbove = true;

    lMouseIsAbove &= (mDropPositionX >= lButtonBeginPos.x);
    lMouseIsAbove &= (mDropPositionX <= lButtonEndPos.x);

    lMouseIsAbove &= (mDropPositionY >= lButtonBeginPos.y);
    lMouseIsAbove &= (mDropPositionY <= lButtonEndPos.y);

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

void StreamDeckWindow::EnumerateDevices()
{
    std::unordered_set<std::string> lConnectedSerials;

    hid_device_info* devs = hid_enumerate(0x0fd9, 0x006d);

    for (hid_device_info* cur_dev = devs; cur_dev; cur_dev = cur_dev->next) {
		if( cur_dev->vendor_id == 0x0fd9 && cur_dev->product_id ==0x006d )
        {
            std::wstring lWideString(cur_dev->serial_number);
            std::string lSerial = std::string( lWideString.begin(), lWideString.end() );
            lConnectedSerials.insert(lSerial);
            
            //mDevicesMap[lSerial] = new StreamDeckRawDevice(cur_dev->vendor_id, cur_dev->product_id, lSerial.c_str());
        }
	}

	hid_free_enumeration(devs);

    //check new devices
    for ( const std::string& lSerial : lConnectedSerials )
    {
        if( mDevicesMap.find(lSerial) == mDevicesMap.end() )
        {//new connection
            //TODO: find an efficient way to store Vendor and Device Ids
            mDevicesMap[lSerial] = new StreamDeckRawDevice(0x0fd9, 0x006d, lSerial.c_str());
            
            //need to update here in order to have connection to send the image
            mDevicesMap[lSerial]->Update();
            //mDevicesMap[lSerial]->SetImageFromPath(0, "../resources/gimp.jpg");
        }
    }

    std::list<std::string> lDevicesToRemove;

    //check for disconnected devices
    for ( const std::pair<std::string, StreamDeckRawDevice*>& lDevicePair : mDevicesMap )
    {
        if( lConnectedSerials.find(lDevicePair.first) == lConnectedSerials.end() )
        {
            lDevicesToRemove.push_back(lDevicePair.first);
        }
    }

    for ( const std::string& lSerial : lDevicesToRemove )
    {//delete disconnected devices
        delete mDevicesMap[lSerial];
        mDevicesMap.erase(lSerial);
    }
}
