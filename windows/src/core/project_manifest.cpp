#include "core/project_manifest.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include <nlohmann/json.hpp>

namespace compositor {
namespace {

using json = nlohmann::json;

const std::regex kUuidPattern{
    "^[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}$"};

bool validUuid(const std::string& value) { return std::regex_match(value, kUuidPattern); }
bool finite(double value) { return std::isfinite(value); }

template <typename T>
void optionalFrom(const json& value, std::string_view key, std::optional<T>& target) {
  if (const auto found = value.find(key); found != value.end() && !found->is_null()) {
    target = found->get<T>();
  }
}

template <typename T>
void optionalTo(json& value, std::string_view key, const std::optional<T>& source) {
  if (source) value[std::string(key)] = *source;
}

ValidationIssue issue(ValidationCode code, std::string message,
                      std::optional<std::size_t> layer = std::nullopt) {
  return {code, std::move(message), layer};
}

}  // namespace

void from_json(const json& value, Point& point) {
  value.at("x").get_to(point.x);
  value.at("y").get_to(point.y);
}

void to_json(json& value, const Point& point) { value = json{{"x", point.x}, {"y", point.y}}; }

void from_json(const json& value, Size& size) {
  value.at("width").get_to(size.width);
  value.at("height").get_to(size.height);
}

void to_json(json& value, const Size& size) {
  value = json{{"width", size.width}, {"height", size.height}};
}

void from_json(const json& value, LayerTransform& transform) {
  value.at("origin").get_to(transform.origin);
  value.at("size").get_to(transform.size);
  transform.rotation = value.value("rotation", 0.0);
  value.at("flipX").get_to(transform.flipX);
  value.at("flipY").get_to(transform.flipY);
  value.at("sampling").get_to(transform.sampling);
}

void to_json(json& value, const LayerTransform& transform) {
  value = json{{"origin", transform.origin},
               {"size", transform.size},
               {"rotation", transform.rotation},
               {"flipX", transform.flipX},
               {"flipY", transform.flipY},
               {"sampling", transform.sampling}};
}

void from_json(const json& value, LayerRecord& layer) {
  value.at("id").get_to(layer.id);
  value.at("name").get_to(layer.name);
  layer.isVisible = value.value("isVisible", true);
  value.at("transform").get_to(layer.transform);
  optionalFrom(value, "imageFile", layer.imageFile);
  optionalFrom(value, "parentID", layer.parentID);
  optionalFrom(value, "isGroup", layer.isGroup);
  optionalFrom(value, "opacity", layer.opacity);
  optionalFrom(value, "blendMode", layer.blendMode);
  optionalFrom(value, "maskFile", layer.maskFile);
  optionalFrom(value, "maskEnabled", layer.maskEnabled);
  optionalFrom(value, "maskSourceID", layer.maskSourceID);
  optionalFrom(value, "adjustment", layer.adjustment);
  optionalFrom(value, "maskPlacement", layer.maskPlacement);
  optionalFrom(value, "maskLinked", layer.maskLinked);
  optionalFrom(value, "shape", layer.shape);
}

void to_json(json& value, const LayerRecord& layer) {
  value = json{{"id", layer.id},
               {"name", layer.name},
               {"isVisible", layer.isVisible},
               {"transform", layer.transform}};
  optionalTo(value, "imageFile", layer.imageFile);
  optionalTo(value, "parentID", layer.parentID);
  optionalTo(value, "isGroup", layer.isGroup);
  optionalTo(value, "opacity", layer.opacity);
  optionalTo(value, "blendMode", layer.blendMode);
  optionalTo(value, "maskFile", layer.maskFile);
  optionalTo(value, "maskEnabled", layer.maskEnabled);
  optionalTo(value, "maskSourceID", layer.maskSourceID);
  optionalTo(value, "adjustment", layer.adjustment);
  optionalTo(value, "maskPlacement", layer.maskPlacement);
  optionalTo(value, "maskLinked", layer.maskLinked);
  optionalTo(value, "shape", layer.shape);
}

void from_json(const json& value, ProjectManifest& manifest) {
  value.at("format").get_to(manifest.format);
  value.at("version").get_to(manifest.version);
  value.at("colorSpace").get_to(manifest.colorSpace);
  optionalFrom(value, "resolution", manifest.resolution);
  value.at("documentID").get_to(manifest.documentID);
  value.at("width").get_to(manifest.width);
  value.at("height").get_to(manifest.height);
  optionalFrom(value, "activeLayerID", manifest.activeLayerID);
  value.at("layers").get_to(manifest.layers);
}

void to_json(json& value, const ProjectManifest& manifest) {
  value = json{{"format", manifest.format},
               {"version", manifest.version},
               {"colorSpace", manifest.colorSpace},
               {"documentID", manifest.documentID},
               {"width", manifest.width},
               {"height", manifest.height},
               {"layers", manifest.layers}};
  optionalTo(value, "resolution", manifest.resolution);
  optionalTo(value, "activeLayerID", manifest.activeLayerID);
}

std::vector<ValidationIssue> validate(const ProjectManifest& manifest) {
  std::vector<ValidationIssue> result;
  if (manifest.format != kProjectFormat)
    result.push_back(issue(ValidationCode::InvalidFormat, "Unexpected project format"));
  if (manifest.version < kOldestProjectVersion || manifest.version > kCurrentProjectVersion)
    result.push_back(issue(ValidationCode::UnsupportedVersion, "Unsupported project version"));
  if (manifest.colorSpace != "sRGB")
    result.push_back(issue(ValidationCode::UnsupportedColorSpace, "Only sRGB is valid in .comp v1-v7"));
  if (manifest.resolution && (!finite(*manifest.resolution) || *manifest.resolution < 1 || *manifest.resolution > 9600))
    result.push_back(issue(ValidationCode::InvalidResolution, "Resolution must be from 1 to 9600 ppi"));
  if (manifest.width < 1 || manifest.width > kMaximumDimension || manifest.height < 1 || manifest.height > kMaximumDimension)
    result.push_back(issue(ValidationCode::InvalidCanvasSize, "Canvas dimensions exceed the .comp contract"));
  if (manifest.layers.size() > kMaximumLayers)
    result.push_back(issue(ValidationCode::TooManyLayers, "Project contains more than 10,000 layers"));
  if (!validUuid(manifest.documentID))
    result.push_back(issue(ValidationCode::InvalidIdentifier, "Document ID is not a UUID"));

  std::unordered_set<std::string> ids;
  std::unordered_map<std::string, std::size_t> indices;
  for (std::size_t index = 0; index < manifest.layers.size(); ++index) {
    const auto& layer = manifest.layers[index];
    if (!validUuid(layer.id))
      result.push_back(issue(ValidationCode::InvalidIdentifier, "Layer ID is not a UUID", index));
    if (!ids.insert(layer.id).second)
      result.push_back(issue(ValidationCode::DuplicateIdentifier, "Layer ID is duplicated", index));
    indices[layer.id] = index;
    if (layer.name.empty() || layer.name.size() > 16'384)
      result.push_back(issue(ValidationCode::InvalidLayerName, "Layer name is empty or too long", index));
    const auto& t = layer.transform;
    if (!finite(t.origin.x) || !finite(t.origin.y) || !finite(t.size.width) || !finite(t.size.height) ||
        !finite(t.rotation) || t.size.width < 1 || t.size.width > 300'000 ||
        t.size.height < 1 || t.size.height > 300'000 || std::abs(t.origin.x) > 1'000'000 ||
        std::abs(t.origin.y) > 1'000'000 ||
        (t.sampling != "Nearest" && t.sampling != "Smooth" && t.sampling != "High quality"))
      result.push_back(issue(ValidationCode::InvalidTransform, "Layer transform is invalid", index));

    const bool group = layer.isGroup.value_or(false);
    const auto expectedImage = layer.id + ".png";
    const auto expectedMask = layer.id + ".mask.png";
    if (layer.imageFile && *layer.imageFile != expectedImage)
      result.push_back(issue(ValidationCode::InvalidLayerAsset, "Image filename must be derived from the layer UUID", index));
    if (group && layer.imageFile)
      result.push_back(issue(ValidationCode::InvalidLayerAsset, "A group cannot own image pixels", index));
    if (layer.maskFile && *layer.maskFile != expectedMask)
      result.push_back(issue(ValidationCode::InvalidMaskAsset, "Mask filename must be derived from the layer UUID", index));
    if (layer.maskEnabled && !layer.maskFile)
      result.push_back(issue(ValidationCode::InvalidMaskAsset, "maskEnabled requires maskFile", index));

    const double opacity = layer.opacity.value_or(1.0);
    const std::string blend = layer.blendMode.value_or("Normal");
    if (!finite(opacity) || opacity < 0 || opacity > 1 || (group && (opacity != 1 || blend != "Normal")))
      result.push_back(issue(ValidationCode::InvalidAppearance, "Opacity or blend mode is invalid", index));
    if (manifest.version < 3 && (opacity != 1 || blend != "Normal"))
      result.push_back(issue(ValidationCode::InvalidVersionedFeature, "Layer appearance requires v3", index));
    if (layer.maskFile && manifest.version < (group ? 6 : 4))
      result.push_back(issue(ValidationCode::InvalidVersionedFeature, "Layer mask requires a newer project version", index));
    if (layer.maskSourceID && manifest.version < 5)
      result.push_back(issue(ValidationCode::InvalidVersionedFeature, "Clipping mask requires v5", index));
    if (layer.adjustment && (manifest.version < 7 || group || layer.imageFile))
      result.push_back(issue(ValidationCode::InvalidVersionedFeature, "Adjustment layer requires an image-free v7 leaf", index));
    if (manifest.version == 1 && (layer.parentID || group))
      result.push_back(issue(ValidationCode::InvalidVersionedFeature, "Groups require v2", index));
  }

  if (manifest.activeLayerID && !ids.contains(*manifest.activeLayerID))
    result.push_back(issue(ValidationCode::InvalidActiveLayer, "Active layer is missing"));

  for (std::size_t index = 0; index < manifest.layers.size(); ++index) {
    const auto& layer = manifest.layers[index];
    if (layer.parentID) {
      const auto parent = indices.find(*layer.parentID);
      if (parent == indices.end() || !manifest.layers[parent->second].isGroup.value_or(false))
        result.push_back(issue(ValidationCode::InvalidHierarchy, "Parent must refer to an existing group", index));
    }
    if (layer.maskSourceID) {
      const auto source = indices.find(*layer.maskSourceID);
      if (source == indices.end() || *layer.maskSourceID == layer.id ||
          manifest.layers[source->second].isGroup.value_or(false) || layer.isGroup.value_or(false))
        result.push_back(issue(ValidationCode::InvalidLiveMask, "Clipping mask source is invalid", index));
    }
  }
  return result;
}

ProjectManifest readManifest(const std::filesystem::path& package) {
  const auto file = package / "manifest.json";
  if (!std::filesystem::is_regular_file(file) || std::filesystem::file_size(file) > kMaximumManifestBytes)
    throw std::runtime_error("Missing or oversized manifest.json");
  std::ifstream stream(file, std::ios::binary);
  if (!stream) throw std::runtime_error("Cannot open manifest.json");
  auto manifest = json::parse(stream).get<ProjectManifest>();
  if (const auto issues = validate(manifest); !issues.empty()) throw std::runtime_error(issues.front().message);
  return manifest;
}

void writeManifest(const ProjectManifest& manifest, const std::filesystem::path& package) {
  if (const auto issues = validate(manifest); !issues.empty()) throw std::runtime_error(issues.front().message);
  std::filesystem::create_directories(package / "images");
  const auto temporary = package / "manifest.json.tmp";
  const auto destination = package / "manifest.json";
  {
    std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
    if (!stream) throw std::runtime_error("Cannot create manifest.json.tmp");
    stream << json(manifest).dump(2) << '\n';
  }
  if (std::filesystem::file_size(temporary) > kMaximumManifestBytes) {
    std::filesystem::remove(temporary);
    throw std::runtime_error("Manifest exceeds 4 MiB");
  }
  std::error_code ignored;
  std::filesystem::remove(destination, ignored);
  std::filesystem::rename(temporary, destination);
}

}  // namespace compositor
