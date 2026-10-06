#include <chrono>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "Chip8.h"
#include "Platform.h"

int main(int argc, char** argv)
{
	// Check command line arguments.
    if (argc != 4)
    {
        std::cerr
            << "Usage: "
            << argv[0]
            << " <Scale> <Cycle Delay> <ROM>\n";

        return EXIT_FAILURE;
    }
    
    

    try
    {
		// Parse command line arguments.
		// Scale: The scale factor for the video output.
		// Cycle Delay: The delay in milliseconds between each CPU cycle.
        const int videoScale = std::stoi(argv[1]);
        const int cycleDelay = std::stoi(argv[2]);

        const char* romFilename = argv[3];

        if (videoScale <= 0)
        {
            throw std::invalid_argument(
                "Video scale must be greater than zero.");
        }

        if (cycleDelay <= 0)
        {
            throw std::invalid_argument(
                "Cycle delay must be greater than zero.");
        }

        Platform platform(
            "CHIP-8 Emulator",
            static_cast<int>(VIDEO_WIDTH) * videoScale,
            static_cast<int>(VIDEO_HEIGHT) * videoScale,
            static_cast<int>(VIDEO_WIDTH),
            static_cast<int>(VIDEO_HEIGHT));

        Chip8 chip8;

        chip8.LoadROM("Tetris.ch8");
        
		// Pitch: Number of bytes in a row of the video buffer. Each pixel is 4 bytes (RGBA).
        const int videoPitch =
            sizeof(uint32_t) * static_cast<int>(VIDEO_WIDTH);

        using Clock = std::chrono::steady_clock;

        auto lastCycleTime = Clock::now();
        auto lastTimerTime = Clock::now();

        constexpr auto timerInterval =
            std::chrono::milliseconds(1000 / 60);

        bool quit = false;

        while (!quit)
        {
            quit = platform.ProcessInput(
                chip8.GetKeypad());

            const auto currentTime = Clock::now();

            // CPU timing.
            const auto cycleElapsed =
                std::chrono::duration_cast<
                std::chrono::milliseconds>(
                    currentTime - lastCycleTime);

            if (cycleElapsed.count() >= cycleDelay)
            {
                lastCycleTime = currentTime;

                chip8.Cycle();

                platform.Update(
                    chip8.GetVideo(),
                    videoPitch);
            }

            // chip 8 timers run at 60 Hz.
            if (currentTime - lastTimerTime >= timerInterval)
            {
                lastTimerTime = currentTime;

                chip8.UpdateTimers();
            }
        }
    }
    // fail if file is not found
    catch (const std::exception& exception)
    {
        std::cerr
            << "Error: "
            << exception.what()
            << '\n';

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}