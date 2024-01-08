#include "Window.h"

#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "imgui.h"

#include "Theme.h"
#include "Core/Defines.h"
#include <mutex>

namespace App
{
    Window::Window(int screenWidth, int screenHeight, const char* windowTitle, Core::GameBoy* gb) : gameboy(gb)
    {
        #if !SDL_VERSION_ATLEAST(2,0,17)
            #error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
        #endif

        // Setup SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
        {
            printf("Error: %s\n", SDL_GetError());
            return;
        }

        // From 2.0.18: Enable native IME.
        #ifdef SDL_HINT_IME_SHOW_UI
            SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
        #endif

        // Setup audio
        if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
            // Handle SDL initialization failure
            return;
        }

        SDL_AudioSpec want, have;
        SDL_memset(&want, 0, sizeof(want));
        want.freq = 44100;
        want.format = AUDIO_S16SYS;   // Signed 16-bit samples in little-endian byte order
        want.channels = 1;
        want.samples = 512;          // Buffer size (adjust as needed)
        want.callback = StaticAudioCallback;
        want.userdata = this;

        sdlAudioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

        // Create window with SDL_Renderer graphics context
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        sdlWindow = SDL_CreateWindow( windowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, window_flags);
        sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
        if (sdlRenderer == nullptr)
        {
            SDL_Log("Error creating SDL_Renderer!");
            return;
        }

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; 

        // Setup Dear ImGui style
        ImGui::StyleColorsLight();
        setTheme();

        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForSDLRenderer(sdlWindow, sdlRenderer);
        ImGui_ImplSDLRenderer2_Init(sdlRenderer);

        startTime = SDL_GetTicks();
    }

    Window::~Window()
    {
        // Clean up imgui resources
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        // Clean up SDL resources
        SDL_CloseAudioDevice(sdlAudioDevice);
        SDL_DestroyRenderer(sdlRenderer);
        SDL_DestroyWindow(sdlWindow);
        SDL_Quit();
    }

    bool Window::Initialize()
    {
        // Check if SDL initialization was successful
        SDL_PauseAudioDevice(sdlAudioDevice, 0);  // Start audio playback
        return sdlWindow != nullptr && sdlRenderer != nullptr;
        
    }

    void Window::Update(bool& isRunning) 
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) 
        {
            ImGui_ImplSDL2_ProcessEvent(&event);       

            // handle quit
            if (event.type == SDL_QUIT ||
                    (event.type == SDL_WINDOWEVENT &&
                    event.window.event == SDL_WINDOWEVENT_CLOSE &&
                    event.window.windowID == SDL_GetWindowID(sdlWindow)))
            {
                isRunning = false;
            }             

            // handle input
            if (event.type == SDL_KEYDOWN)
            {
                SDL_Keycode pressedKey = event.key.keysym.sym;
                switch (pressedKey)
                {
                    case SDLK_p:
                    {
                        if (gameboy->IsPaused())
                        {
                            gameboy->Unpause();
                        }
                        else
                        {
                            gameboy->Pause();
                        }
                        break;
                    }
                    case SDLK_SPACE:
                    {
                        if (gameboy->IsPaused())
                        {
                            gameboy->StepCPU();
                        }
                        break;
                    }
                    // game controls
                    case SDLK_UP:
                        gameboy->input.SetDPADState(Core::Joypad_DPAD::UP, true);
                        break;
                    case SDLK_DOWN:
                        gameboy->input.SetDPADState(Core::Joypad_DPAD::DOWN, true);
                        break;
                    case SDLK_LEFT:
                        gameboy->input.SetDPADState(Core::Joypad_DPAD::LEFT, true);
                        break;
                    case SDLK_RIGHT:
                        gameboy->input.SetDPADState(Core::Joypad_DPAD::RIGHT, true);
                        break;
                    case SDLK_TAB:
                        gameboy->input.SetButtonState(Core::Joypad_Button::SELECT, true);
                        break;
                    case SDLK_RETURN:
                        gameboy->input.SetButtonState(Core::Joypad_Button::START, true);
                        break;
                    case SDLK_s:
                        gameboy->input.SetButtonState(Core::Joypad_Button::B, true);
                        break;
                    case SDLK_a:
                        gameboy->input.SetButtonState(Core::Joypad_Button::A, true);
                        break;
                }
            }
            else if (event.type == SDL_KEYUP)
            {
                SDL_Keycode releasedKey = event.key.keysym.sym;
                switch (releasedKey)
                {
                    // game controls
                    case SDLK_UP:
                        gameboy->input.SetDPADState(Core::Joypad_DPAD::UP, false);
                        break;
                    case SDLK_DOWN:
                        gameboy->input.SetDPADState(Core::Joypad_DPAD::DOWN, false);
                        break;
                    case SDLK_LEFT:
                        gameboy->input.SetDPADState(Core::Joypad_DPAD::LEFT, false);
                        break;
                    case SDLK_RIGHT:
                        gameboy->input.SetDPADState(Core::Joypad_DPAD::RIGHT, false);
                        break;
                    case SDLK_TAB:
                        gameboy->input.SetButtonState(Core::Joypad_Button::SELECT, false);
                        break;
                    case SDLK_RETURN:
                        gameboy->input.SetButtonState(Core::Joypad_Button::START, false);
                        break;
                    case SDLK_s:
                        gameboy->input.SetButtonState(Core::Joypad_Button::B, false);
                        break;
                    case SDLK_a:
                        gameboy->input.SetButtonState(Core::Joypad_Button::A, false);
                        break;
                }
            }
        }

        Uint32 currentTime = SDL_GetTicks();
        elapsedTime = currentTime - startTime;
        startTime = currentTime;
    }

    void Window::BeginRender()
    {
        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // add dockspace to entire window
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::DockSpaceOverViewport(viewport);
    }

    void Window::EndRender()
    {
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        ImGui::Render();

        // Update and render ImGui multi-viewports
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        SDL_RenderSetScale(sdlRenderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(sdlRenderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(sdlRenderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(sdlRenderer);
    }

    SDL_Renderer* Window::GetRenderer()
    {
        return sdlRenderer;
    }

    Uint32 Window::GetElapsedTime()
    {
        return elapsedTime;
    }

    void Window::StaticAudioCallback(void* userdata, Uint8* stream, int len)
    {
        Window* window = static_cast<Window*>(userdata);
        std::lock_guard<std::mutex> lock(window->mutex);  // Ensure proper synchronization
        if (window && window->gameboy) {
            window->gameboy->FeedAudioBuffer(stream, len);
        }
    }
}