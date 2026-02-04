#include "plugin.h"

#include <nx/kit/utils.h>

#include "engine.h"
#include "face_recognition_ini.h"

namespace mimos
{
namespace plugin
{
namespace face_recognition
{

using namespace nx::sdk;
using namespace nx::sdk::analytics;

Result<IEngine*> Plugin::doObtainEngine()
{
    return new Engine();
}

/**
* JSON with the particular structure. Note that it is possible to fill the value that are known
* at the compile time
* - id: unique identifier
* - name: a human readable short name of plugin (display in the camera settings)
* - description: description of the plugin
* - version: version of the plugin
* - vendor: plugin vendor
*/
std::string Plugin::manifestString() const
{
    const static std::string manifest = R"json(
    {
        "id": "custom.face_recognition",
        "name": "Face Recognition (RetinaFace + ArcFace)",
        "description": "Performs face detection with RetinaFace and recognition with ArcFace. Matches detected faces against whitelist and blacklist embeddings stored as .npy files. Generates object metadata and blacklist alert events.",
        "version": "1.0.0",
        "vendor": "Custom Development",
        "isLicenseRequired": %s,

        "supportedObjectTypes": [
            "nx.base.Person"
        ],

        "objectTypes": [
            {
                "id": "nx.base.Person",
                "name": "Person (Detected Face)",
                "icon": "person"
            },
            {
                "id": "face_recognition.whitelisted",
                "name": "Whitelisted Person",
                "icon": "person_check",
                "attributes": [
                    {"name": "name",        "displayName": "Name"},
                    {"name": "similarity",  "displayName": "Similarity", "type": "number"}
                ]
            },
            {
                "id": "face_recognition.blacklisted",
                "name": "Blacklisted Person",
                "icon": "person_warning",
                "attributes": [
                    {"name": "name",        "displayName": "Name"},
                    {"name": "similarity",  "displayName": "Similarity", "type": "number"}
                ]
            }
        ],

        "eventTypes": [
            {
                "id": "face_recognition.blacklist_alert",
                "name": "Blacklisted Person Detected",
                "description": "A person from the blacklist was detected",
                "icon": "warning",
                "isStateDependent": false
            },
            {
                "id": "face_recognition.whitelist_detected",
                "name": "Whitelisted Person Detected",
                "description": "A person from the whitelist was detected",
                "icon": "check_circle",
                "isStateDependent": false
            }
        ],

        "settingsModel": [
            {
                "name": "whitelistPath",
                "displayName": "Whitelist Directory",
                "type": "string",
                "defaultValue": "/opt/face-plugin/whitelist",
                "description": "Path to folder containing .npy whitelist embeddings"
            },
            {
                "name": "blacklistPath",
                "displayName": "Blacklist Directory",
                "type": "string",
                "defaultValue": "/opt/face-plugin/blacklist",
                "description": "Path to folder containing .npy blacklist embeddings"
            },
            {
                "name": "matchThreshold",
                "displayName": "Minimum Match Similarity",
                "type": "number",
                "defaultValue": 0.45,
                "minimum": 0.0,
                "maximum": 1.0,
                "step": 0.01
            },
            {
                "name": "minDetectionScore",
                "displayName": "Min Face Detection Confidence",
                "type": "number",
                "defaultValue": 0.55,
                "minimum": 0.0,
                "maximum": 1.0,
                "step": 0.01
            }
        ]
    }
    )json";

    return nx::kit::utils::format(
        manifest,
        ini().isLicenseRequired ? "true" : "false"
    );
}

} // namespace face_recognition
} // namespace plugin
} // namespace mimos

// ---------------------------------------------
// Nx Plugin Entry Point (required by the server)
// ---------------------------------------------
extern "C" nx::sdk::IPlugin* createNxPlugin()
{
    return new nx::vms_server_plugins::analytics::face_recognition::Plugin();
}