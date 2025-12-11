// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <cstdint>
#include <string>

namespace mimos {
namespace face_plugin {

enum class EventType
{
    detection_started,
    detection_finished,
    object_detected
};

struct Event
{
    const EventType eventType;
    const int64_t timestampUs;
    const std::string classLabel;
};

} // namespace face_plugin
} // namespace mimos
