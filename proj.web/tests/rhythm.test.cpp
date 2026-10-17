#include "RhythmTimeline.h"
#include <cassert>
#include <iostream>
int main() {
    afroman::RhythmTimeline timeline;
    const double time = timeline.noteTime(299, 0);
    const double center = timeline.position(time, 0) + 299.5*40;
    assert(std::abs(center - timeline.targetX) < 1e-8);
    assert(std::abs(timeline.noteTime(299, 0.15)-time-0.15)<1e-9);
    for (int fps : {15, 30, 60, 120, 144}) {
        double last = 0;
        for (int i=0;i<=fps*20;++i) last=timeline.position(double(i)/fps,0);
        assert(std::abs(last-timeline.position(20,0))<1e-8);
    }
    std::vector<afroman::RhythmNote> lane{afroman::RhythmNote(299)};
    assert(!timeline.takeHit(lane,time-0.1,0));
    int matchedColumn = -1;
    assert(timeline.takeHit(lane,time+0.08,0,&matchedColumn));
    assert(matchedColumn == 299); // visual feedback must identify the scored note
    assert(!timeline.takeHit(lane,time+0.08,0,&matchedColumn));
    assert(matchedColumn == -1); // one score per note
    std::vector<afroman::RhythmNote> otherLane{afroman::RhythmNote(299)};
    assert(timeline.takeHit(otherLane,time,0)); // simultaneous drums are independent
    std::vector<afroman::RhythmNote> delayedFrame{afroman::RhythmNote(299)};
    timeline.position(time-0.3,0); // stale last render cannot affect hit judgment
    assert(timeline.takeHit(delayedFrame,time,0));
    std::vector<afroman::RhythmNote> calibrated{afroman::RhythmNote(299)};
    assert(timeline.takeHit(calibrated,time+0.4,0.4));
    std::cout << "PASS: chart crossing, 15-144 Hz invariance, timestamp scoring, offsets, one hit per note\n";
}
