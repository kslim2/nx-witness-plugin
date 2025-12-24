// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <nx/sdk/analytics/helpers/consuming_device_agent.h>
#include <nx/sdk/analytics/i_uncompressed_video_frame.h>

#include "engine.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace sample {

class DeviceAgent: public nx::sdk::analytics::ConsumingDeviceAgent
{
public:
    DeviceAgent(const nx::sdk::IDeviceInfo* deviceInfo);
    virtual ~DeviceAgent() override;

protected:
    virtual bool pushUncompressedVideoFrame(
        const nx::sdk::analytics::IUncompressedVideoFrame* videoFrame) override;

    virtual std::string manifestString() const override;

    virtual void doSetNeededMetadataTypes(
        nx::sdk::Result<void>* outResult,
        const nx::sdk::analytics::IMetadataTypes* neededMetadataTypes) override;
};

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx