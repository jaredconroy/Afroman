#pragma once
namespace afroman {
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
