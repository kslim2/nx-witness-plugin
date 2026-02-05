#include "engine.h"

#include "device_agent.h"
#include "device_agent_manifest.h"
#include "face_recognition_ini.h"
#include "utils.h"  // For loadEmbeddingsFromDirectory and logging helpers if any

#include <nx/kit/json.h>
#include <nx/kit/debug.h>  // For logging (NX_PRINT, etc.)
#include <nx/sdk/log.h>    // Assuming SDK logging is available

#include <set>
#include <string>

namespace mimos
{
namespace plugin
{
namespace face_recognition
{

using namespace nx::sdk;
using namespace nx::sdk::analytics;
using namespace nx::kit;

Engine::Engine()
    : nx::sdk::analytics::Engine(ini().enabledByDefault)
{
    // Load embeddings during engine initialization
    if (!loadEmbeddings())
    {
        // Log error but continue (plugin can still run without embeddings)
        NX_PRINT << "Failed to load embeddings during Engine init";
    }
}

Engine::~Engine()
{
    // No need to explicitly delete shared_ptrs
}

bool Engine::loadEmbeddings()
{
    const std::string whitelistDir = ini().whitelistDirectory;
    const std::string blacklistDir = ini().blacklistDirectory;

    m_whitelistEmbeddings = std::make_shared<std::map<std::string, std::vector<float>>>(
        loadEmbeddingsFromDirectory(whitelistDir));

    m_blacklistEmbeddings = std::make_shared<std::map<std::string, std::vector<float>>>(
        loadEmbeddingsFromDirectory(blacklistDir));

    if (m_whitelistEmbeddings->empty())
    {
        NX_PRINT << "Warning: No whitelist embeddings loaded from " << whitelistDir;
    }

    if (m_blacklistEmbeddings->empty())
    {
        NX_PRINT << "Warning: No blacklist embeddings loaded from " << blacklistDir;
    }

    return !m_whitelistEmbeddings->empty() || !m_blacklistEmbeddings->empty();
}

void Engine::doObtainDeviceAgent(
    Result<IDeviceAgent*>* outResult,
    const IDeviceInfo* deviceInfo)
{
    try
    {
        // Create agent and pass shared embeddings
        *outResult = new DeviceAgent(
            deviceInfo,
            m_whitelistEmbeddings,
            m_blacklistEmbeddings
        );
    }
    catch (const std::exception& e)
    {
        NX_PRINT << "Error creating DeviceAgent: " << e.what();
        *outResult = nullptr;
        outResult->setErrorMessage(e.what());
        outResult->setErrorCode(nx::sdk::ErrorCode::internalError);
    }
}

std::string Engine::manifestString() const
{
    std::string errors;
    Json deviceAgentManifestJson = Json::parse(kDeviceAgentManifest, errors).object_items();

    if (!errors.empty())
    {
        NX_PRINT << "Manifest parse error: " << errors;
        // In production, you may want to return a fallback manifest
        return "{}";  // Empty manifest to prevent crash
    }

    // ------------------------------------------------------------------------
    // Build settings model for UI
    // ------------------------------------------------------------------------

    Json::array settingsItems;

    settingsItems.push_back(Json::object{ {"type", "Separator"} });

    settingsItems.push_back(Json::object{
        {"type",        "CheckBox"},
        {"name",        "enableRecognition"},
        {"caption",     "Enable face recognition"},
        {"description", "Toggle face detection and recognition"},
        {"defaultValue", true}
    });

    settingsItems.push_back(Json::object{
        {"type",        "DoubleSpinBox"},
        {"name",        "matchThreshold"},
        {"caption",     "Min match similarity"},
        {"description", "Cosine similarity threshold (0.0-1.0)"},
        {"defaultValue", ini().matchThreshold},
        {"minimum",      0.0},
        {"maximum",      1.0},
        {"step",         0.01}
    });

    settingsItems.push_back(Json::object{
        {"type",        "DoubleSpinBox"},
        {"name",        "minDetectionConfidence"},
        {"caption",     "Min detection confidence"},
        {"description", "RetinaFace confidence threshold (0.0-1.0)"},
        {"defaultValue", ini().minDetectionConfidence},
        {"minimum",      0.0},
        {"maximum",      1.0},
        {"step",         0.05}
    });

    settingsItems.push_back(Json::object{
        {"type",        "SpinBox"},
        {"name",        "processingIntervalFrames"},
        {"caption",     "Process every Nth frame"},
        {"description", "Higher = lower CPU (0 = every frame)"},
        {"defaultValue", ini().faceProcessingIntervalFrames},
        {"minimum",      0},
        {"maximum",      60},
        {"step",         1}
    });

    settingsItems.push_back(Json::object{ {"type", "Separator"} });

    settingsItems.push_back(Json::object{
        {"type",        "CheckBox"},
        {"name",        "enableBlacklistAlerts"},
        {"caption",     "Blacklist alerts"},
        {"defaultValue", ini().enableBlacklistAlerts}
    });

    settingsItems.push_back(Json::object{
        {"type",        "CheckBox"},
        {"name",        "enableWhitelistEvents"},
        {"caption",     "Whitelist events"},
        {"defaultValue", ini().enableWhitelistEvents}
    });

    // Additional production setting: Reload embeddings button
    settingsItems.push_back(Json::object{
        {"type",        "Button"},
        {"name",        "reloadEmbeddings"},
        {"caption",     "Reload embeddings"},
        {"description", "Reload whitelist/blacklist from disk"}
    });

    Json::object settingsModel = {
        {"type",  "Settings"},
        {"items", settingsItems}
    };

    // ------------------------------------------------------------------------
    // Engine manifest
    // ------------------------------------------------------------------------

    Json::object engineManifest = {
        {"streamTypeFilter",        "compressedVideo"},
        {"deviceAgentSettingsModel", settingsModel}
    };

    return Json(engineManifest).dump();
}

} // namespace face_recognition
} // namespace plugin
} // namespace mimos