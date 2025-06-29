#pragma once

#include <cstdint>

#include "Providers.h"

union SDL_Event;

struct ManagedWindowID;

class ManagedWindow:public SdlResourcesProvider
{
public:
    ManagedWindow(int32_t pArgC, char** pArgV);
    virtual ~ManagedWindow();

    int32_t Execute();

    SDL_Renderer* GetSdlRenderer() override;
    SDL_Window* GetSdlWindow() override;

protected:
    virtual int32_t Init();
    virtual int32_t Event(SDL_Event *pEvent);
    virtual int32_t Iterate();
    virtual void Quit();

    float GetLastFrameDuration();
    
    int32_t mArgC = 0;
    char** mArgV = nullptr;

private:
    ManagedWindowID* mID = nullptr;

    int32_t InternalInit();
    int32_t InternalEvent(SDL_Event *pEvent);
    int32_t InternalIterate();
    void InternalQuit();

    friend struct ManagedWindowID;
};
