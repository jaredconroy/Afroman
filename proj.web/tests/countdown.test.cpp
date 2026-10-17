#include "GameCountdown.h"
#include <cassert>
#include <cmath>
int main() {
    for (int hz : {15,30,60,144}) {
        afroman::GameCountdown clock;
        int seconds = 25;
        for (int i=0; i<15*hz; ++i) clock.advance(seconds,1.0/hz);
        assert(seconds==10 && clock.scale(seconds)==1);
        for (int i=0; i<5*hz; ++i) clock.advance(seconds,1.0/hz);
        assert(seconds==5 && std::abs(clock.scale(seconds)-1.2f)<1e-5);
        clock.advance(seconds,.35);
        assert(clock.scale(seconds)==1);
        clock.advance(seconds,20);
        assert(seconds==0 && clock.scale(seconds)==1);
    }
    afroman::GameCountdown clock;
    int seconds=60;
    clock.advance(seconds,2.5);
    assert(seconds==58 && std::abs(clock.fraction-.5)<1e-9);
}
