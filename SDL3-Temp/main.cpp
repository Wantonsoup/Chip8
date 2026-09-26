#include <SDL3/SDL.h>
// SDL3 main header.
// Gives us access to SDL_Window, SDL_Renderer, SDL_Event,
// SDL_Init(), SDL_CreateWindow(), SDL_CreateRenderer(), etc.
#include <SDL3/SDL.h>

int main()
{
    // SDL_Init()
    // Initialises SDL and the subsystems we want to use.
    //
    // SDL_INIT_VIDEO tells SDL:
    // "I want to use the video/windowing system."
    //
    // SDL_Init() returns true on success and false on failure in SDL3.
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        // SDL_Log() prints a message to the console.
        //
        // SDL_GetError() returns SDL's description of the
        // most recent error.
        SDL_Log("SDL_Init failed: %s", SDL_GetError());

        return 1;
    }


    // SDL_Window
    // Represents the actual OS window.
    //
    // SDL_Window* is a pointer because SDL creates and manages the window for us.
    //
    // SDL_CreateWindow() creates the window and returns a pointer to the newly created SDL_Window.
    SDL_Window* window = SDL_CreateWindow(
        "Blank",    // Window title
        640,        // Window width
        320,        // Window height
        0           // Window flags/options. 0 = default.
    );


    // If SDL_CreateWindow() fails, it returns nullptr.
    if (!window)
    {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());

        // SDL_Quit()
        // Shuts down SDL and cleans up SDL's global resources.
        SDL_Quit();

        return 1;
    }


    // SDL_Renderer
    // Represents the system SDL uses to draw things onto the window.
    //
    // Think of it as our drawing interface:
    //
    // SDL_Window
    //      ↑
    // SDL_Renderer
    //      ↑
    // Draw commands
    //
    // The renderer will eventually be used to draw our CHIP-8
    // 64x32 framebuffer.
    SDL_Renderer* renderer = SDL_CreateRenderer(window, 0);


    // If SDL_CreateRenderer() fails, it returns nullptr.
    if (!renderer)
    {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());

        // We already created the window, so destroy it
        // before shutting SDL down.
        SDL_DestroyWindow(window);

        SDL_Quit();

        return 1;
    }


    // This controls whether our main application loop
    // should continue running.
    bool running = true;

    // SDL_Event
    // A structure containing information about something
    // that happened in SDL.
    //
    // Examples:
    // - Window closed
    // - Keyboard key pressed
    // - Mouse movement
    // - Controller input
    //
    // For our CHIP-8 emulator, keyboard events will eventually
    // be used to control the CHIP-8 keypad.
    SDL_Event event;


    // Main application/game loop.
    //
    // This runs repeatedly while our program is running.
    while (running)
    {
        // SDL_PollEvent()
        // Checks SDL's event queue for events.
        //
        // &event gives SDL the address of our SDL_Event variable
        // so SDL can write the event information into it.
        //
        // It returns true while there are events available.
        while (SDL_PollEvent(&event))
        {
            // SDL_EVENT_QUIT
            // This event is generated when the user attempts
            // to close the window.
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
        }


        // SDL_SetRenderDrawColor()
        // Sets the colour that the renderer will use for
        // subsequent drawing operations.
        //
        // Parameters:
        // renderer = which renderer to use
        // 0 = Red
        // 0 = Green
        // 0 = Blue
        // 255 = Alpha (fully opaque)
        //
        // Therefore this gives us black.
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);


        // SDL_RenderClear()
        // Clears the entire rendering target using the colour
        // we set above.
        //
        // Currently this simply gives us a black screen.
        //
        // Later, we can use this when drawing the CHIP-8 display.
        SDL_RenderClear(renderer);


        // SDL_RenderPresent()
        // Displays everything we have drawn since the previous
        // frame.
        //
        // Think of rendering as:
        //
        // 1. Draw everything
        // 2. Present the finished frame
        //
        // Without SDL_RenderPresent(), our drawing would not
        // appear on the window.
        SDL_RenderPresent(renderer);
    }


    // The application has finished.
    //
    // SDL_DestroyRenderer()
    // Releases the renderer created by SDL_CreateRenderer().
    SDL_DestroyRenderer(renderer);


    // SDL_DestroyWindow()
    // Closes and releases the SDL_Window created earlier.
    SDL_DestroyWindow(window);


    // SDL_Quit()
    // Shuts down SDL and cleans up the SDL subsystems
    // that were initialised with SDL_Init().
    SDL_Quit();


    // Returning 0 tells the operating system that the program
    // finished successfully.
    return 0;
}
