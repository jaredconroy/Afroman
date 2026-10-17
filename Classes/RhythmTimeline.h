#pragma once
#include <cmath>
#include <vector>

// Seconds, not frame counts: the original chart's 8.2 px/update at 60 Hz.
// A positive calibration offset makes notes arrive later relative to music.
namespace afroman {
struct RhythmNote { int column; bool hit = false; explicit RhythmNote(int c):column(c) {} };
struct RhythmTimeline {
    double originX = -15500.0;
    double targetX = 465.0;
    double tileWidth = 40.0;
    double pixelsPerSecond = 8.2 * 60.0;
    double hitWindowSeconds = 0.090;
    double position(double musicSeconds, double offsetSeconds) const {
        return originX + pixelsPerSecond * (musicSeconds - offsetSeconds);
    }
    double introRemaining(double musicSeconds, double offsetSeconds,
                          double halfWidth, double mapWidth) const {
        return (-halfWidth-mapWidth-originX)/pixelsPerSecond + offsetSeconds - musicSeconds;
    }
    double noteTime(int column, double offsetSeconds) const {
        return (targetX - originX - (column + 0.5) * tileWidth) / pixelsPerSecond + offsetSeconds;
    }
    bool takeHit(std::vector<RhythmNote>& notes, double musicSeconds, double offsetSeconds, int* hitColumn = nullptr) const {
        if (hitColumn) *hitColumn = -1;
        RhythmNote* best = nullptr;
        double nearest = hitWindowSeconds + 1e-9;
        for (auto& note : notes) {
            const double distance = std::abs(musicSeconds - noteTime(note.column, offsetSeconds));
            if (!note.hit && distance <= nearest) { best = &note; nearest = distance; }
        }
        if (!best) return false;
        best->hit = true;
        if (hitColumn) *hitColumn = best->column;
        return true;
    }
};
}
