#pragma once
#include <algorithm>
#include <cmath>
namespace afroman {
struct GameCountdown {
    double fraction = 0;
    void advance(int& seconds, double delta) {
        fraction += std::max(0.0, delta);
        while (fraction + 1e-9 >= 1.0 && seconds > 0) { fraction = std::max(0.0, fraction - 1.0); --seconds; }
        if (seconds == 0) fraction = 0;
    }
    float scale(int seconds) const {
        return seconds > 0 && seconds <= 5 ?
            1.0f + 0.20f * std::max(0.0, 1.0 - fraction / 0.35) : 1.0f;
    }
};
}
