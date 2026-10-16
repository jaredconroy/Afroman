# Afroman Mac preservation build

Preservation date: 2026-10-16. Game baseline: 68ffa44097ec2c998013ac95ce5fa9691bbdcb77.
Engine: Cocos2d-x 3.2, commit 2210d1fe87c3dd7d2ce79d01945b85a0de24862d, with local compatibility patches.

## Layout and toolchain

Game: ~/Documents/Afroman. Engine: ~/Documents/cocos2d-x-3.2.
The game symlink cocos2d points to ../cocos2d-x-3.2. Preserve this sibling layout when restoring.
Validated prior build log: Xcode Debug, HelloCpp Mac scheme, Intel x86_64.
Current tools: Xcode 27.1 (27A9275), macOS 26.6.2 (25G83).
Deployment target is macOS 12.0. Bundled dependency libraries require x86_64;
Apple Silicon requires Rosetta. The current project does not automate asset copying.

## Build and package

```sh
cd ~/Documents/Afroman
xcodebuild -project proj.ios_mac/Afroman.xcodeproj \
  -scheme 'HelloCpp Mac' -configuration Debug \
  -destination 'platform=macOS,arch=x86_64' \
  -derivedDataPath /tmp/afroman-mac-derived \
  ARCHS=x86_64 ONLY_ACTIVE_ARCH=YES \
  OTHER_LDFLAGS='$(inherited) /Users/conroyj2/Documents/cocos2d-x-3.2/external/freetype2/prebuilt/mac/libfreetype.a /Users/conroyj2/Documents/cocos2d-x-3.2/external/png/prebuilt/mac/libpng.a /Users/conroyj2/Documents/cocos2d-x-3.2/external/jpeg/prebuilt/mac/libjpeg.a /Users/conroyj2/Documents/cocos2d-x-3.2/external/tiff/prebuilt/mac/libtiff.a /Users/conroyj2/Documents/cocos2d-x-3.2/external/webp/prebuilt/mac/libwebp.a /Users/conroyj2/Documents/cocos2d-x-3.2/external/glfw3/prebuilt/mac/libglfw3.a' build
APP='/tmp/afroman-mac-derived/Build/Products/Debug/HelloCpp Mac.app'
rsync -a Resources/ "$APP/Contents/Resources/"
open "$APP"
# Direct executable, if console diagnostics are needed:
"$APP/Contents/MacOS/HelloCpp Mac"
```

Adjust absolute library paths if restoring elsewhere. Authoritative sources are
AppDelegate, GameScene, GameScene2, GameScene3, Level, Player, and StartScreen.
The root historical CMakeLists.txt is not a working Mac/web build recipe.

## Engine changes

- Python 3 downloader conversion and current archived Lua dependency repository.
- iOS headers moved to ios.disabled to avoid modern Xcode header collisions.
- Explicit platform-header selection in CCPlatformMacros.h and ZipUtils.h.
- Deployment targets raised to 12.0; Xcode scheme metadata updated.
- Removed fake AudioSession declarations/stub; Mac mute/other-audio queries return NO.
- Mac text uses explicit premultiplied RGBA8 bitmap drawing, with antialiasing and
  font smoothing disabled, instead of interpreting display-dependent capture bytes.

preservation/mac/engine-tracked.patch contains tracked changes except the iOS
folder rename. To reconstruct from the exact baseline: apply with git apply,
then move cocos/platform/ios to cocos/platform/ios.disabled. Downloaded dependencies,
untracked backup files, and Git metadata are retained in the local full archive.

## Font and gameplay status

HUD source requests the family "8-Bit Madness". This Mac implementation requires
the bundled Resources/fonts/8-Bit-Madness.ttf to be installed in macOS Font Book.
The existing MAC_TEXT_FIX.md records previous visual/pixel verification and a
successful build. This preservation session found that the saved regression
executable exits at its font-availability check (exit 2); fresh pixel verification
is therefore not claimed. Test source is retained in preservation/mac/.
No additional text renderer change was made during preservation.

The user reports level 2 timing is already inaccurate on this computer: it was
calibrated to an old Windows laptop. It is reference code, not an authoritative
audio timing specification. The browser port should recalibrate against music.

## Preservation payload

Local .preservation/Afroman-mac-archive-2026-10-16.tar.gz contains both complete
working repositories (including engine dependencies and Git history) plus the
existing built app and temporary verification evidence. It is intentionally
excluded from Git. A SHA-256 checksum accompanies it. Copy both files to an
independent backup for protection against disk failure.

Extract into an empty directory; game and engine are siblings under Documents/.
The game symlink remains valid. The engine snapshot contains an existing
self-referencing symlink; do not recursively dereference symlinks when copying.
The source checkpoint and tag on GitHub complement this local payload; GitHub
does not contain the full dependency snapshot or compiled app.
