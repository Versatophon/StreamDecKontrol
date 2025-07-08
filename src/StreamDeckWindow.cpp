#include "StreamDeckWindow.h"

#include <SDL3/SDL.h>

#include <iostream>
#include <fstream>

#include <unordered_set>
#include <list>

#include "externals/imgui/imgui.h"

#include "StreamDeckDevice.h"

//TODO: check if we need this header
#include "StreamDeckSurface.h"
#include "StreamDeckSurfaceProvider.h"


StreamDeckWindow::StreamDeckWindow(): ManagedWindow(0, nullptr),
mStreamDeckSurfaceProvider(new StreamDeckSurfaceProvider(this))
{
}
 
StreamDeckWindow::~StreamDeckWindow()
{
    delete mStreamDeckSurfaceProvider;
}

const char* StreamDeckWindow::GetQueuedFilepath()
{
    if( mDroppedFileQueue.empty() )
    {
        return nullptr;
    }

    mLastDroppedFilepath = mDroppedFileQueue.front();mDroppedFileQueue.pop();
    return mLastDroppedFilepath.c_str();
}

bool StreamDeckWindow::DropPending()
{
    return mIsDroppingSomething;
}

float StreamDeckWindow::DropPositionX()
{
    return mDropPositionX;
}

float StreamDeckWindow::DropPositionY()
{
    return mDropPositionY;
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
        break;

    }

    return SDL_APP_CONTINUE;
}

int32_t StreamDeckWindow::Iterate()
{
    //Setup
    //TODO: may need to perform this less often
    EnumerateDevices();


    for( std::pair<std::string, StreamDeckDevice*> mDevicePair :mDevicesMap)
    {
        mDevicePair.second->Update(GetLastFrameDuration());
    }
    
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove);

    if( ImGui::BeginTabBar("bar") )
    {
        for( std::pair<std::string, StreamDeckDevice*> mDevicePair :mDevicesMap)
        {
            mDevicePair.second->DisplayDeviceTab();
        }
       
        ImGui::EndTabBar();
    }

    ImGui::End();

    //TODO: Find a better solution for this event 
    //Clean drop queue
    if( !mDroppedFileQueue.empty())
    {
        {//Special case for testing
            const char* lFilePath = mDroppedFileQueue.front().c_str();

            for( std::pair<std::string, StreamDeckDevice*> mDevicePair :mDevicesMap)
            {
                for ( size_t i = 0 ; i < 15 ; ++i)
                {
                    mDevicePair.second->SetButtonImage(i, lFilePath);
                }
            }
        }
        

        while(!mDroppedFileQueue.empty())
        {
            mDroppedFileQueue.pop();
        }
    }
    

    return SDL_APP_CONTINUE;
}

void StreamDeckWindow::Quit()
{
    for (std::pair<std::string, StreamDeckDevice*> mDevicePair : mDevicesMap)
    {
        mDevicePair.second->Reset();
    }
    int32_t lRes = hid_exit();
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
        }
	}

	hid_free_enumeration(devs);

    //check new devices
    for ( const std::string& lSerial : lConnectedSerials )
    {
        if( mDevicesMap.find(lSerial) == mDevicesMap.end() )
        {//new connection
            //TODO: find an efficient way to store Vendor and Device Ids
            mDevicesMap[lSerial] = new StreamDeckDevice(lSerial.c_str(), mStreamDeckSurfaceProvider, this);
            
            //need to update here in order to have connection to send the image
            mDevicesMap[lSerial]->Update(0.f);
            //mDevicesMap[lSerial]->SetImageFromPath(0, "/home/versatophon/Images/smiley72.gif");
            //mDevicesMap[lSerial]->SetImageFromPath(1, "/home/versatophon/Images/gimp.jpg");
            //mDevicesMap[lSerial]->SetImageFromPath(2, "/home/versatophon/Images/gimp.bmp");
            //mDevicesMap[lSerial]->SetImageFromPath(3, "/home/versatophon/Images/27.jpg");
            //mDevicesMap[lSerial]->SetImageFromPath(4, "/home/versatophon/Images/27.2.jpg");
        }
    }

    std::list<std::string> lDevicesToRemove;

    //check for disconnected devices
    for ( const std::pair<std::string, StreamDeckDevice*>& lDevicePair : mDevicesMap )
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
