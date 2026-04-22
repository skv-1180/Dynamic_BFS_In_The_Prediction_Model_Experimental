#pragma once
#include <chrono>

class Timer
{
   private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    TimePoint start_time;
    double accumulated_us = 0.0;  // total paused time
    bool running = false;

   public:
    // Constructor (optional auto-start)
    explicit Timer(bool auto_start = false)
    {
        if (auto_start) play();
    }

    // Start / Resume (same as play)
    void play()
    {
        if (!running)
        {
            start_time = Clock::now();
            running = true;
        }
    }

    // Pause (accumulate time)
    void pause()
    {
        if (running)
        {
            accumulated_us += std::chrono::duration<double, std::micro>(
                                  Clock::now() - start_time)
                                  .count();
            running = false;
        }
    }

    // Stop (same as pause, but semantic clarity)
    void stop()
    {
        pause();
    }

    // Reset everything
    void reset()
    {
        accumulated_us = 0.0;
        running = false;
    }

    // Elapsed time in microseconds
    double elapsed_us() const
    {
        if (running)
        {
            return accumulated_us +
                   std::chrono::duration<double, std::micro>(
                       Clock::now() - start_time)
                       .count();
        }
        return accumulated_us;
    }

    double elapsed_ms() const
    {
        return elapsed_us() / 1000.0;
    }
};