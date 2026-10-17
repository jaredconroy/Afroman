#pragma once
namespace afroman {
inline bool binTopSupports(float footX, float feet, float verticalStep) {
    return footX >= 5600 && footX < 5760 && verticalStep <= 0 &&
        feet >= 1216 && feet+verticalStep <= 1216;
}
// Keep support latched through tile seams; only separation rearms an impact.
class SurfaceImpact {
    bool occupied = false;
public:
    bool update(bool supported, bool overlaps, float feet) {
        if (!overlaps || feet > 1218) occupied = false;
        const bool impact = supported && !occupied;
        if (supported) occupied = true;
        return impact;
    }
};
}
