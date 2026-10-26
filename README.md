# Grit60 by Tapework – MPC-style 12-bit sampler crunch (VST3)

## Build
Requirements: CMake 3.22+, a C++17 compiler (Visual Studio 2022 / Xcode / gcc), internet (fetches JUCE).

    cmake -B build
    cmake --build build --config Release

The VST3 is copied to your system plug-in folder automatically.
A Standalone app is also built so you can test without a DAW.

## Controls
- BITS: quantisation depth (12 = MPC60 vibe, lower = nastier)
- SAMPLE RATE: sample-and-hold rate (40000 = MPC60; try 26000 / 22050 for SP-1200 style grit)
- COMPAND: mu-law style non-linear quantisation (the "warm" low-level noise of old 12-bit samplers)
- DRIVE: soft-clip input saturation
- NOISE: low-level hiss, also acts like dither
- TONE: output low-pass, mimics the analog filter stage
- ALIAS MODE: removes the anti-alias filter so frequencies fold back (crunchier, more lo-fi)

## Install locations
- Windows VST3: C:\Program Files\Common Files\VST3
- Mac VST3: /Library/Audio/Plug-Ins/VST3
- Mac Audio Unit (Grit60.component): /Library/Audio/Plug-Ins/Components
- Mac unsigned plugins may need: xattr -cr <path to plugin>
- Ableton Live 11/12 loads VST3 on Windows and Mac. Mac AU shows up too.

## Tape section (v2)
- TAPE DRIVE: gentle tape-style saturation and peak compression
- WOBBLE: slow wow and fast flutter pitch drift
- HEAD BUMP: low-end lift around 90 Hz, like a tape machine
- Presets: Default 12-bit, Clean Glue, MPC60 Classic, Dusty Sampler, Tape Warmth, Crushed Drums
- The plugin reports about 4 ms of latency to the DAW because of the wobble delay line.
