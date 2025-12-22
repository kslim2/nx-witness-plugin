// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <map>
#include <memory>
#include <set>

#include <opencv2/tracking/tracking_by_matching.hpp>

#include <nx/sdk/helpers/uuid_helper.h>
#include <nx/sdk/uuid.h>

#include "detection.h"
#include "frame.h"

namespace mimos {
namespace face_plugin {

/**
 * Provides conversion from int ids coming from the tracker to Uuid ids that are needed by the
 * Server.
 */
class IdMapper
{
public:
    nx::sdk::Uuid get(int64_t id);
    void removeAllExcept(const std::set<nx::sdk::Uuid>& idsToKeep);

private:
    std::map<int64_t, nx::sdk::Uuid> m_map;
};

struct DetectionInternal
{
    std::shared_ptr<Detection> detection;
    int64_t cvTrackId;
};

using DetectionInternalList = std::vector<std::shared_ptr<DetectionInternal>>;

struct CompositeDetectionId
{
    const int64_t frameIndex;
    const cv::Rect rect;
};

using ClassLabelMap = std::map<const CompositeDetectionId, std::string>;

cv::detail::tracking::tbm::TrackedObjects convertDetectionsToTrackedObjects(
    const Frame& frame,
    const DetectionList& detections);

std::shared_ptr<DetectionInternal> convertTrackedObjectToDetection(
    const Frame& frame,
    const cv::detail::tracking::tbm::TrackedObject& trackedDetection,
    IdMapper* idMapper);

DetectionInternalList convertTrackedObjectsToDetections(
    const Frame& frame,
    const cv::detail::tracking::tbm::TrackedObjects& trackedDetections,
    IdMapper* idMapper);

DetectionList extractDetectionList(const DetectionInternalList& detectionsInternal);

} // namespace face_plugin
} // namespace mimos

namespace std
{
template<> struct less<
    const mimos::face_plugin::CompositeDetectionId>
{
    bool operator()(
        const mimos::face_plugin::CompositeDetectionId&
        lhs,
        const mimos::face_plugin::CompositeDetectionId&
        rhs) const
    {
        if (lhs.frameIndex != rhs.frameIndex)
            return lhs.frameIndex < rhs.frameIndex;
        if (lhs.rect.x != rhs.rect.x)
            return lhs.rect.x < rhs.rect.x;
        if (lhs.rect.y != rhs.rect.y)
            return lhs.rect.y < rhs.rect.y;
        if (lhs.rect.width != rhs.rect.width)
            return lhs.rect.width < rhs.rect.width;
        return lhs.rect.height < rhs.rect.height;
    }
};
}
