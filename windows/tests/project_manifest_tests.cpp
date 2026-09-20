#include "core/project_manifest.h"

#include <cstdlib>
#include <iostream>

namespace {

void expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "FAILED: " << message << '\n';
    std::exit(1);
  }
}

compositor::ProjectManifest validManifest() {
  compositor::LayerRecord layer;
  layer.id = "123E4567-E89B-42D3-A456-426614174001";
  layer.name = "Layer 1";
  layer.transform.origin = {0, 0};
  layer.transform.size = {1920, 1080};
  layer.imageFile = layer.id + ".png";

  compositor::ProjectManifest manifest;
  manifest.documentID = "123E4567-E89B-42D3-A456-426614174000";
  manifest.width = 1920;
  manifest.height = 1080;
  manifest.activeLayerID = layer.id;
  manifest.layers.push_back(layer);
  return manifest;
}

}  // namespace

int main() {
  auto manifest = validManifest();
  expect(compositor::validate(manifest).empty(), "a valid v7 manifest must pass");

  manifest.layers.front().imageFile = "unsafe.png";
  expect(!compositor::validate(manifest).empty(), "asset names must be UUID-derived");

  manifest = validManifest();
  manifest.version = 1;
  manifest.layers.front().isGroup = true;
  manifest.layers.front().imageFile.reset();
  expect(!compositor::validate(manifest).empty(), "groups must be rejected before v2");

  manifest = validManifest();
  manifest.layers.front().opacity = 1.2;
  expect(!compositor::validate(manifest).empty(), "opacity above one must be rejected");

  std::cout << "project_manifest_tests: OK\n";
}

