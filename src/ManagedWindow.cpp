#include "ManagedWindow.h"

#define SDL_MAIN_USE_CALLBACKS 1/* use the callbacks instead of main() */
#define SDL_MAIN_HANDLED 1
    
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_sdl3.h"
#include "externals/imgui/imgui_impl_sdlrenderer3.h"

#include <cstdlib>
#include <GL/gl.h>

#include <iostream>

SDL_AppResult AppInit(void **pAppstate, int32_t pArgC, char** pArgV);
SDL_AppResult AppEvent(void *pAppstate, SDL_Event *pEvent);
SDL_AppResult AppIterate(void *pAppState);
void AppQuit(void *pAppstate, SDL_AppResult pResult);

struct ManagedWindowID
{
    ManagedWindow* Window = nullptr;
    SDL_Window *SdlWindow = nullptr;
    SDL_Renderer *SdlRenderer = nullptr;

    ManagedWindowID(ManagedWindow* pWindow):
    Window(pWindow)
    {
    }

    int32_t Init()
    {
        return Window->InternalInit();
    }

    int32_t Event(SDL_Event *pEvent)
    {
        return Window->InternalEvent(pEvent);
    }

    int32_t Iterate()
    {
        return Window->InternalIterate();
    }

    void Quit()
    {
        Window->InternalQuit();
    }
};


ManagedWindow::ManagedWindow(int32_t pArgC, char** pArgV):
mArgC(pArgC),
mID(new ManagedWindowID(this))
{
    if( mArgC > 0)
    {
        mArgV = new char*[mArgC];
        for (int32_t i = 0 ; i < mArgC ; ++i)
        {
            int32_t lStringLength = strlen(pArgV[i])+1;//copy with trailing '\0'
            mArgV[i] = new char[lStringLength];
            memcpy(mArgV[i], pArgV[i], lStringLength);
        }
    }
}

ManagedWindow::~ManagedWindow()
{
    for (int32_t i = 0 ; i < mArgC ; ++i)
    {
        delete mArgV[i];
    }
    delete mArgV;

    delete mID;
}

int32_t ManagedWindow::Execute()
{
    char* lArgV [1] = {(char*)this->mID};
    return SDL_EnterAppMainCallbacks(1, lArgV, AppInit, AppIterate, AppEvent, AppQuit);
}

int32_t ManagedWindow::Init()
{
    return SDL_APP_CONTINUE;
}

int32_t ManagedWindow::Event(SDL_Event *pEvent)
{
    return SDL_APP_CONTINUE;
}

int32_t ManagedWindow::Iterate()
{
    return SDL_APP_CONTINUE;
}

void ManagedWindow::Quit()
{
}

int32_t ManagedWindow::InternalInit()
{
    SDL_SetAppMetadata("My Linux Stream Deck", "0.0.1", "com.versatophon.streamdeck");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_WindowFlags lFlags = SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIGH_PIXEL_DENSITY;

    /* we don't _need_ a window for audio-only things but it's good policy to have one. */
    if (!SDL_CreateWindowAndRenderer("Stream Deck", 640, 480, lFlags, &mID->SdlWindow, &mID->SdlRenderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderVSync(mID->SdlRenderer, 1);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(mID->SdlWindow, mID->SdlRenderer);
    ImGui_ImplSDLRenderer3_Init(mID->SdlRenderer);

    return Init();
}

int32_t ManagedWindow::InternalEvent(SDL_Event *pEvent)
{
    if (pEvent->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }

    if( !ImGui_ImplSDL3_ProcessEvent(pEvent) )
    {
        return Event(pEvent);
    }

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

int32_t ManagedWindow::InternalIterate()
{
    /* we're not doing anything with the renderer, so just blank it out. */
    SDL_RenderClear(mID->SdlRenderer);

    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    int32_t lReturnValue = Iterate();

    //Render
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), mID->SdlRenderer);

    SDL_RenderPresent(mID->SdlRenderer);

    return lReturnValue;
}

void ManagedWindow::InternalQuit()
{
    Quit();
}

/* This function runs once at startup. */
SDL_AppResult AppInit(void** pAppState, int pArgC, char** pArgV)
{
    *pAppState = pArgV[0];
    return (SDL_AppResult)((ManagedWindowID*)(*pAppState))->Init();
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult AppEvent(void* pAppState, SDL_Event *pEvent)
{
     return (SDL_AppResult)((ManagedWindowID*)pAppState)->Event(pEvent);
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult AppIterate(void *pAppState)
{
    return (SDL_AppResult)((ManagedWindowID*)pAppState)->Iterate();
}

/* This function runs once at shutdown. */
void AppQuit(void *pAppState, SDL_AppResult pResult)
{
    ((ManagedWindowID*)pAppState)->Quit();
}
