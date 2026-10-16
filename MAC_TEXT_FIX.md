# Mac LabelTTF RGBA8 fix

The fix is applied to `../cocos2d-x-3.2/cocos/platform/mac/CCDevice.mm`.
`mac-label-rgba8.patch` preserves the change in this repository; it is already
applied to the engine and should not be applied a second time.

## Diagnosis

On this Mac, the old NSImage lockFocus / initWithFocusedViewRect path returns
four-channel, 16-bit floating-point pixels (64 bits per pixel). For a 245-pixel
row, bytesPerRow is 1960, not the 980 bytes expected by Cocos RGBA8888 textures.
White has half-float component bytes `00 3c`, which the old memcpy misreads as
8-bit channels. It also copies only half the bitmap, corrupting the layout.
This is a sample-format mismatch, not a red/blue channel swap.

## Change

Allocate NSBitmapImageRep with 8 bits per sample, four interleaved samples,
alpha last, premultiplied alpha, and width * 4 bytes per row. Clear its pixels,
draw directly into its graphics context, then copy that explicitly defined
RGBA8 buffer to Cocos. Save and restore AppKit graphics state. Keep both
antialiasing and font smoothing disabled for crisp pixel-font edges.

Font selection, label measurements/alignment, game logic, and the existing
x86_64/Rosetta build configuration are unchanged. The installed 8-Bit Madness
font is still required by the existing font lookup.

## Verification

- Xcode Debug build for HelloCpp Mac with ARCHS=x86_64 succeeded, using the
  six bundled Intel libraries explicitly in OTHER_LDFLAGS.
- Copied Resources into the rebuilt app's Contents/Resources with ditto.
- Launched the game and visually verified the upright white level-one timer,
  including a live countdown update and its appearance behind the pause menu.
- Called Device::getTextureDataForText from an x86_64 regression executable
  linked against the rebuilt engine. All five samples passed: TIME 1:00 and
  TIME 0:59 at 40 pt, SCORE: 0 at 40 pt, 12345 at 60 pt, and 1:00 at 50 pt.
  Verified buffer size, premultiplied-alpha flag, nonempty glyph/background,
  R = G = B = A for every pixel, and alpha strictly 0 or 255.

Build log: `/private/tmp/afroman-rgba8-build.log` (temporary).
Regression source: `/private/tmp/afroman-text-regression.mm` (temporary).

To apply the saved patch to an engine with the same prior rasterizer:

```sh
cd ~/Documents/cocos2d-x-3.2
patch --dry-run -p0 -i ../Afroman/mac-label-rgba8.patch
patch -p0 -i ../Afroman/mac-label-rgba8.patch
```
