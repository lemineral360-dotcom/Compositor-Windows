#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace compositor {

inline constexpr std::string_view kProjectFormat = "com.compositor.project";
inline constexpr int kOldestProjectVersion = 1;
inline constexpr int kCurrentProjectVersion = 7;
inline constexpr std::uint32_t kMaximumDimension = 30'000;
inline constexpr std::uint32_t kMaximumLayers = 10'000;
inline constexpr std::uint64_t kMaximumPixels = 100'000'000;
inline constexpr std::uint64_t kMaximumManifestBytes = 4ULL * 1024 * 1024;
inline constexpr std::uint64_t kMaximumAssetBytes = 512ULL * 1024 * 1024;

struct Point final {
  double x{};
  double y{};
};

struct Size final {
  double width{};
  double height{};
};

// The serialized field names deliberately match Swift's synthesized Codable
// representation. Keeping this contract stable is what permits Mac/Windows
// round trips without a migration layer.
struct LayerTransform final {
  Point origin;
  Size size;
  double rotation{};
  bool flipX{};
  bool flipY{};
  std::string sampling{"High quality"};
};

struct LayerRecord final {
  std::string id;
  std::string name;
  bool isVisible{true};
  LayerTransform transform;
  std::optional<std::string> imageFile;
  std::optional<std::string> parentID;
  std::optional<bool> isGroup;
  std::optional<double> opacity;
  std::optional<std::string> blendMode;
  std::optional<std::string> maskFile;
  std::optional<bool> maskEnabled;
  std::optional<std::string> maskSourceID;

  // Version 7 payloads are retained as JSON. The Windows editor can introduce
  // typed models incrementally without dropping data written by the Mac app.
  std::optional<nlohmann::json> adjustment;
  std::optional<LayerTransform> maskPlacement;
  std::optional<bool> maskLinked;
  std::optional<nlohmann::json> shape;
};

struct ProjectManifest final {
  std::string format{std::string(kProjectFormat)};
  int version{kCurrentProjectVersion};
  std::string colorSpace{"sRGB"};
  std::optional<double> resolution;
  std::string documentID;
  std::uint32_t width{};
  std::uint32_t height{};
  std::optional<std::string> activeLayerID;
  std::vector<LayerRecord> layers;
};

enum class ValidationCode {
  InvalidFormat,
  UnsupportedVersion,
  UnsupportedColorSpace,
  InvalidResolution,
  InvalidCanvasSize,
  TooManyLayers,
  InvalidIdentifier,
  DuplicateIdentifier,
  InvalidActiveLayer,
  InvalidLayerName,
  InvalidTransform,
  InvalidLayerAsset,
  InvalidMaskAsset,
  InvalidHierarchy,
  InvalidLiveMask,
  InvalidAppearance,
  InvalidVersionedFeature,
};

struct ValidationIssue final {
  ValidationCode code;
  std::string message;
  std::optional<std::size_t> layerIndex;
};

void from_json(const nlohmann::json& value, Point& point);
void to_json(nlohmann::json& value, const Point& point);
void from_json(const nlohmann::json& value, Size& size);
void to_json(nlohmann::json& value, const Size& size);
void from_json(const nlohmann::json& value, LayerTransform& transform);
void to_json(nlohmann::json& value, const LayerTransform& transform);
void from_json(const nlohmann::json& value, LayerRecord& layer);
void to_json(nlohmann::json& value, const LayerRecord& layer);
void from_json(const nlohmann::json& value, ProjectManifest& manifest);
void to_json(nlohmann::json& value, const ProjectManifest& manifest);

[[nodiscard]] std::vector<ValidationIssue> validate(const ProjectManifest& manifest);
[[nodiscard]] ProjectManifest readManifest(const std::filesystem::path& package);
void writeManifest(const ProjectManifest& manifest, const std::filesystem::path& package);

}  // namespace compositor
