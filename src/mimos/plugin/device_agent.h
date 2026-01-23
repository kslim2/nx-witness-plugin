#pragma once

#include <set>
#include <thread>

#include <nx/sdk/analytics/helpers/consuming_device_agent.h>
#include <nx/sdk/helpers/uuid_helper.h>

# include "engine.h"

namespace mimos
{
namespace plugin
{

class DeviceAgent: public nx::sdk::analytics::ConsumingDeviceAgent
{

public:
    DeviceAgent(const nx::sdk::IDeviceInfo* deviceInfo);
    virtual ~DeviceAgent() override;

protected:
    virtual std::string manifestString() const override;
    virtual bool pushCompressedVideoFrame(
        const nx::sdk::analytics::ICompressedVideoPacket* videoFrame
    ) override;
    virtual void doSetNeededMetadataTypes(
        nx::sdk::Result<void>* outValue,
        const nx::sdk::analytics::IMetadataTypes* neededMetadataTypes
    ) override;

}

} // namespace plugin
} // namespace mimos