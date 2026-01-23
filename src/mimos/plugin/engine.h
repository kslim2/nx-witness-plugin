#pragma once

#include <nx/sdk/analytics/helpers/engine.h>
#include <nx/sdk/analytics/helpers/plugin.h>
#include <nx/sdk/analytics/i_uncompressed_video_frame.h>

namespace mimos
{
namespace plugin
{

class Engine: public nx::sdk::analytics::Engine
{

public:
    // constructor Inherits from the SDK's base Engine class
    Engine();
    // destructor inherits from the SDK;s base Engine class
    virtual ~Engine() override;

protected:
    // tells the VMS what the plugin is called, what version it is
    virtual std::string manifestString() const override;

protected:
    // a user enables this plugin on specific camera in the VMS client, the
    // server calls this function.
    virtual void doObtainDeviceAgent(
        nx::sdk::Result<nx::sdk::analytics::IDeviceAgent*>* outResult,
        const nx::sdk::IDeviceInfo* deviceInfo
    ) override;

}

}
}