// detection.h
// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <vector>
#include <memory>
#include <string>
#include <nx/sdk/analytics/rect.h>
#include <nx/sdk/uuid.h>

namespace mimos {
namespace face_plugin {

extern const std::string kFaceClassLabel;
extern const std::string kFaceClassLabelCapitalized;

/**
 * Single facial landmark point
 */
struct Landmark
{
    float x;
    float y;
};

/**
 * Type aliases - MUST be defined BEFORE using them in Detection
 */
using Landmarks = std::vector<Landmark>;           // 5 landmarks
using FaceEmbedding = std::vector<float>;          // 512-dimensional vector

/**
 * Final Detection struct - define ONLY ONCE
 */
struct Detection
{
    nx::sdk::analytics::Rect boundingBox;
    float confidence;
    Landmarks landmarks;
    FaceEmbedding embedding;
    nx::sdk::Uuid trackId;

    std::string classLabel = kFaceClassLabel;        // default "face"
    std::string recognizedName = "unknown";
    float similarityScore = 0.0f;
    std::string watchlist = "unknown";               // "whitelist", "blacklist", or "unknown"
};

using DetectionList = std::vector<std::shared_ptr<Detection>>;

} // namespace face_plugin
} // namespace mimos