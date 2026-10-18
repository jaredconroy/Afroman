#include "ConcertStage.h"
#include "GameCountdown.h"
#include <cassert>
int main() {
    afroman::ConcertStage stage;
    assert(!stage.arrive(14943,1280,30));
    assert(!stage.arrive(14944,640,30));
    assert(stage.arrive(14944,1280,30));
    assert(!stage.arrive(15000,1280,29));
    assert(!stage.won(1));
    assert(stage.won(0));
    assert(!stage.lost(0));
    afroman::ConcertStage late;
    assert(!late.arrive(14944,1280,0));
    assert(late.lost(0));
    afroman::GameCountdown clock;
    int time=5;
    clock.advance(time,.5);
    time+=30; // Flush preserves the half-second already elapsed.
    clock.advance(time,.5);
    assert(time==34);
    clock.advance(time,34);
    assert(time==0 && stage.won(time));
}
