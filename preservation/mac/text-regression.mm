#import <Cocoa/Cocoa.h>
#include "platform/CCDevice.h"
#include "base/ccTypes.h"
#include <cstdio>

int main() {
    @autoreleasepool {
        [NSApplication sharedApplication];
        if (![NSFont fontWithName:@"8-Bit-Madness" size:40]) return 2;
        const char *texts[] = {"TIME 1:00", "TIME 0:59", "SCORE: 0", "12345", "1:00"};
        const int sizes[] = {40, 40, 40, 60, 50};
        for (int t = 0; t < 5; ++t) {
            cocos2d::FontDefinition def;
            def._fontName = "8-Bit Madness";
            def._fontSize = sizes[t];
            def._dimensions = cocos2d::Size(t == 3 ? 500 : 245, t == 3 ? 40 : 32);
            int width = 0, height = 0;
            bool premultiplied = false;
            cocos2d::Data data = cocos2d::Device::getTextureDataForText(texts[t], def,
                cocos2d::Device::TextAlign::CENTER, width, height, premultiplied);
            if (data.getSize() != width * height * 4 || !data.getSize() || !premultiplied) return 3;
            size_t visible = 0, transparent = 0;
            const unsigned char *p = data.getBytes();
            for (ssize_t i = 0; i < data.getSize(); i += 4) {
                if (p[i] != p[i+3] || p[i+1] != p[i+3] || p[i+2] != p[i+3]) return 4;
                if (p[i+3] != 0 && p[i+3] != 255) return 5;
                visible += p[i+3] == 255;
                transparent += p[i+3] == 0;
            }
            if (!visible || !transparent) return 6;
            printf("PASS %s size=%d %dx%d white=%zu transparent=%zu\n",
                texts[t], sizes[t], width, height, visible, transparent);
        }
    }
}
