#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace wing {
constexpr float marker_angle = -2.792526803190927f;
constexpr unsigned slots = 64;
constexpr double default_ms = 250, min_ms = 0, max_ms = 500, step_ms = 50;
struct State {
    uintptr_t model{}, aero{};
    uint64_t epoch{};
    double last{};
    float shown{};
    bool used{};
};

// The caller owns this state. No simulation pointer is written here.
inline float advance(State& s, uintptr_t model, uintptr_t aero, uint64_t epoch,
                     double now, float target, double duration) {
    if (!std::isfinite(target) || target < 0 || target > 1 || !std::isfinite(now))
        return target;
    if (duration <= 0) {
        s = {model, aero, epoch, now, target, true};
        return target; // Zero means no added smoothing; never divide by zero.
    }
    if (!s.used || s.model != model || s.aero != aero || s.epoch != epoch ||
        now < s.last || now - s.last > 0.25) {
        s = {model, aero, epoch, now, target, true};
        return target;
    }
    const double dt = now - s.last;
    s.last = now;
    const float limit = static_cast<float>(dt / std::clamp(duration, 0.05, max_ms / 1000));
    s.shown += std::clamp(target - s.shown, -limit, limit);
    s.shown = std::clamp(s.shown, 0.0f, 1.0f);
    return s.shown;
}
}
