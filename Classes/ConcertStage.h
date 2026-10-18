#pragma once
namespace afroman {
struct ConcertStage {
    bool entered = false;
    bool arrive(float playerRight, float cameraY, int remaining) {
        if (entered || remaining <= 0 || cameraY != 1280 || playerRight < 14944) return false;
        entered = true;
        return true;
    }
    bool won(int remaining) const { return entered && remaining == 0; }
    bool lost(int remaining) const { return !entered && remaining == 0; }
};
}
