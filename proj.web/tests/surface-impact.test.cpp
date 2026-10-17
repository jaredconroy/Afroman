#include "SurfaceImpact.h"
#include <cassert>
int main() {
    assert(afroman::binTopSupports(5600,1216,-1));
    assert(afroman::binTopSupports(5759,1216,-1));
    assert(afroman::binTopSupports(5680,1230,-20));
    assert(!afroman::binTopSupports(5599,1216,-1));
    assert(!afroman::binTopSupports(5760,1216,-1));
    assert(!afroman::binTopSupports(5680,1210,10));
    assert(!afroman::binTopSupports(5680,1215,-1));
    afroman::SurfaceImpact bin;
    assert(bin.update(true,true,1216)); // First gentle landing.
    for (int i=0;i<100;++i) {
        assert(!bin.update(false,true,1216)); // Tile-seam support flicker.
        assert(!bin.update(true,true,1216)); // Walking over the top.
    }
    assert(!bin.update(false,true,1220)); // Jump separates.
    assert(bin.update(true,true,1216)); // Land again.
    assert(!bin.update(false,false,1215)); // Walk off.
    assert(!bin.update(false,false,1200)); // Fall away, no impact.
    assert(bin.update(true,true,1216)); // Return and land.
}
