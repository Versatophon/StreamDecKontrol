#include "StreamDeckWindow.h"

#include <SDL3/SDL.h>

#include <iostream>

#include <unordered_set>
#include <list>

#include "externals/imgui/imgui.h"

StreamDeckWindow::StreamDeckWindow(): ManagedWindow(0, nullptr)
{
}
 
StreamDeckWindow::~StreamDeckWindow()
{
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
        for (size_t lRow = 0 ; lRow < 3 ; ++lRow )
        {
            for (size_t lCol = 0 ; lCol < 5 ; ++lCol )
            {
                bool lButtonPressed = pDevice->GetButtonPressed(lButtonIndex);
                //ImGui::Checkbox(("##" + std::to_string(lButtonIndex)).c_str(), &lButtonPressed);

                DisplayButton(lButtonPressed);

                ++lButtonIndex;

                if( (lButtonIndex % 5) != 0 )
                {
                    ImGui::SameLine();
                }
            }
        }
        #if 0
        if( ImGui::Button(("Set Img 0##" + lSerial).c_str()) )
        {
            pDevice->SetBlankImage(0);
        }

        //ImGui::SameLine();
    
        if( ImGui::Button(("Set Img 5##" + lSerial).c_str()) )
        {
            pDevice->SetBlankImage(5);
        }
        ImGui::SameLine();
        
        if( ImGui::Button(("Set Img 10##" + lSerial).c_str()) )
        {
            pDevice->SetBlankImage(10);
        }
        ImGui::SameLine();
        
        if( ImGui::Button(("Reset##" + lSerial).c_str()) )
        {
            pDevice->Reset();
        }
        #endif

        ImGui::EndTabItem();
    }
}

void StreamDeckWindow::DisplayButton(bool pPressed)
{
    uint32_t lMargin = 20;
    ImDrawList* lDrawList = ImGui::GetWindowDrawList();

    ImVec2 lButtonBeginPos = ImGui::GetCursorPos();
    ImVec2 lButtonEndPos = lButtonBeginPos;
    lButtonEndPos.x += 74;
    lButtonEndPos.y += 74;

    bool lMayDropSomething = mIsDroppingSomething;

    lMayDropSomething &= (mDropPositionX >= lButtonBeginPos.x);
    lMayDropSomething &= (mDropPositionX <= lButtonEndPos.x);

    lMayDropSomething &= (mDropPositionY >= lButtonBeginPos.y);
    lMayDropSomething &= (mDropPositionY <= lButtonEndPos.y);

    lDrawList->AddRect(lButtonBeginPos, lButtonEndPos, ImColor(ImVec4{0.75f, 0.75f, 0.75f, 1.f}), 10, 0, pPressed?2.f:1.f);

    if (lMayDropSomething)
    {
        lDrawList->AddLine(ImVec2(lButtonBeginPos.x + 37, lButtonBeginPos.y + 10), ImVec2(lButtonBeginPos.x + 37, lButtonBeginPos.y + 64), ImColor(ImVec4{0.75f, 0.75f, 0.75f, 1.f}), 5.f);
        lDrawList->AddLine(ImVec2(lButtonBeginPos.x + 10, lButtonBeginPos.y + 37), ImVec2(lButtonBeginPos.x + 64, lButtonBeginPos.y + 37), ImColor(ImVec4{0.75f, 0.75f, 0.75f, 1.f}), 5.f);
    }

    ImGui::Dummy(ImVec2(74 + lMargin, 74 + lMargin + 5));
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
