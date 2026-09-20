# Compositor for Windows

Native Windows 10/11 port of
[robbietilton/Compositor](https://github.com/robbietilton/Compositor), the free
and open-source image editor.

## Status

The port is under active development. The first milestone provides the Windows
application shell and a cross-platform implementation of the `.comp` v1–v7
manifest contract. Editing, GPU rendering, full tool parity, and PSD/PSB support
are tracked as subsequent milestones.

Downloadable Windows builds are produced by the **Windows build** GitHub Actions
workflow. Test builds are unsigned and may show a Microsoft SmartScreen warning.

## Compatibility goals

- Windows 10 version 1809 or newer and Windows 11, x64
- Two-way `.comp` compatibility with the macOS application
- PSD and PSB import/export with lossless retention of unsupported private data
- Feature parity with the macOS editor

See [the Windows port contract](docs/windows-port.md) and
[Windows build instructions](windows/README.md).

## License

MIT, matching the upstream project. Copyright notices from the original project
are retained in `LICENSE`.
