#pragma once

#include <memory>  // Added for shared_ptr
#include <map>
#include <vector>
#include <string>

#include <nx/sdk/analytics/helpers/engine.h>
#include <nx/sdk/analytics/i_device_info.h>
#include <nx/sdk/analytics/i_uncompressed_video_frame.h>
#include <nx/sdk/result.h>

#include "utils.h"  // Assuming utils.h has loadEmbeddingsFromDirectory

namespace mimos
{
namespace plugin
{
namespace face_recognition
{

class Engine : public nx::sdk::analytics::Engine
{
public:
    Engine();
    virtual ~Engine() override;

protected:
    /**
     * Returns the manifest string describing engine capabilities,
     * including supported stream types and device agent settings model.
     * This manifest is used by the VMS to configure the plugin.
     */
    virtual std::string manifestString() const override;

    /**
     * Creates a new DeviceAgent instance for the given device/camera.
     * This is called by the server for each camera the plugin is enabled on.
     */
    virtual void doObtainDeviceAgent(
        nx::sdk::Result<nx::sdk::analytics::IDeviceAgent*>* outResult,
        const nx::sdk::IDeviceInfo* deviceInfo) override;

private:
    // Shared embeddings databases (loaded once per engine, shared across agents)
    std::shared_ptr<std::map<std::string, std::vector<float>>> m_whitelistEmbeddings;
    std::shared_ptr<std::map<std::string, std::vector<float>>> m_blacklistEmbeddings;

    // Helper to load embeddings with error handling
    bool loadEmbeddings();
};

} // namespace face_recognition
} // namespace plugin
} // namespace mimos