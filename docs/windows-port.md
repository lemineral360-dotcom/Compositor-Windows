# Windows port contract

## Product requirements

1. Native 64-bit desktop application for Windows 10 and Windows 11.
2. Two-way `.comp` compatibility with Compositor for macOS.
3. PSD and PSB import/export with the broadest possible Photoshop round trip.
4. Feature parity with the macOS editor, including layers, folders, masks,
   clipping masks, adjustments, transforms, selections, painting, retouching,
   filters, content-aware operations and project tabs.

## Compatibility rules

- The source implementation currently writes `.comp` version 7. The older
  `docs/project-format.md` heading is stale and must not be treated as the
  current version authority.
- A `.comp` remains a directory package containing `manifest.json` and
  `images/*.png`. Windows Explorer integration may present it as one document,
  but the on-disk layout cannot change.
- UUID spelling and UUID-derived asset filenames remain unchanged.
- Unknown valid metadata must survive load/save. Version-7 adjustment and
  shape payloads are retained before all editors for them are implemented.
- PNG assets remain embedded and transforms remain non-destructive.
- The same safety limits used by the Mac app apply on Windows.

## PSD definition of done

“Full PSD” means all publicly specified PSD/PSB structures are parsed and
written; raster layers, groups, masks, clipping, standard blend modes, opacity,
text metadata, common adjustment layers, ICC profiles and 8/16-bit channels
round-trip through automated fixtures. Unsupported Photoshop-private payloads
must be preserved byte-for-byte and surfaced as compatibility warnings rather
than silently discarded. Pixel-identical behavior for undocumented Photoshop
features cannot be promised by any independent editor.

## Architecture boundaries

| Component | Responsibility |
|---|---|
| `core` | Document model, commands, history, validation and `.comp` I/O |
| `raster` | Tiled images, masks, selections and CPU fallback kernels |
| `renderer` | GPU composition, transforms, blend modes and previews |
| `codec` | PNG/JPEG/TIFF/HEIC and PSD/PSB boundaries |
| `ml` | ONNX subject segmentation and model lifecycle |
| `ui` | Qt windows, panels, tools, shortcuts and accessibility |
| `platform/windows` | Shell registration, clipboard, drag/drop and installer |

## Delivery gates

1. Contract: `.comp` v1–v7 fixtures validate and round-trip on both platforms.
2. Canvas: tiled renderer matches reference composites for all blend modes.
3. Editing: every macOS tool passes shared visual and state-transition tests.
4. PSD: Adobe-spec fixture matrix passes without silent data loss.
5. Release: clean Windows 10/11 VMs install, update, uninstall and recover from
   interrupted saves without corrupting projects.

