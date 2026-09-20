# Compositor for Windows

This directory contains the native Windows port. It targets 64-bit Windows 10
and Windows 11 while preserving two-way compatibility with the macOS `.comp`
project package.

The first committed component is a platform-neutral implementation of the
current project-manifest contract (versions 1–7). UI and rendering code must
depend on this core rather than reproducing file-format behavior.

## Planned production stack

- C++20 and CMake
- Qt 6 for the native Windows shell, accessibility, input and windowing
- Skia with Direct3D through ANGLE for the tiled GPU canvas
- Little CMS for ICC transforms and OpenColorIO where appropriate
- ONNX Runtime with DirectML for subject segmentation
- A dedicated PSD/PSB codec boundary with lossless preservation of unsupported
  image-resource and additional-layer-info blocks
- WiX Toolset for MSI packaging

## Build the core tests

```powershell
cmake -S windows -B out/windows -G "Visual Studio 17 2022" -A x64
cmake --build out/windows --config Debug
ctest --test-dir out/windows -C Debug --output-on-failure
```

The complete editor targets Windows 10 version 1809 or later and Windows 11.

