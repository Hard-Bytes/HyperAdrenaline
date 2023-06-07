#pragma once

#include <chrono>
using namespace std::chrono_literals;
using Chrono = std::chrono::high_resolution_clock;
using Nanoseconds = std::chrono::nanoseconds;
using Milliseconds = std::chrono::milliseconds;
using Time_point = std::chrono::_V2::system_clock::time_point;

constexpr Nanoseconds timestep(16ms); // 60 FPS

struct GameTimer {
    GameTimer()
    : time_start(Chrono::now())
    , lag(0ns)
    {}

    constexpr Nanoseconds getDeltaAsNanoseconds() const noexcept
        { return timestep; }

    constexpr Milliseconds getDeltaAsMilliseconds() const noexcept
        { return std::chrono::duration_cast<Milliseconds>(timestep); }

    constexpr double getDeltaAsSeconds() const noexcept
        { return std::chrono::duration_cast<Milliseconds>(timestep).count() / 1000.0; }


    void updateDelta() noexcept
    {
        auto delta_time = Chrono::now() - time_start;
        time_start = Chrono::now();
        lag += std::chrono::duration_cast<Nanoseconds>(delta_time);
    }

    constexpr Nanoseconds getLag() const noexcept
        { return lag; }

    void setLag(Nanoseconds newLag) noexcept
        { lag = newLag; }

private:
    Time_point time_start;
    Nanoseconds lag;
};
