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
    const auto utilityProvider = this->utilityProvider();
    const std::filesystem::path pluginHomeDir = utilityProvider->homeDir();
    return new Engine(pluginHomeDir);
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
    const static std::string manifest = /*suppress newline*/ 1 + (const char*) R"json(
    {
        "id": "custom.face_recognition",
        "name": "Face Recognition (RetinaFace + ArcFace)",
        "description": "Detects faces using RetinaFace, extracts embeddings with ArcFace, and matches them against whitelist and blacklist. Generates object metadata and blacklist match events.",
        "version": "1.0.0",
        "vendor": "Your Company / Custom Plugin",
        "isLicenseRequired": %s,
        "supportedTypes": [
            "nx.base.Person"
        ],
        "objectTypes": [
            {
                "id": "nx.base.Person",
                "name": "Person",
                "icon": "person"
            },
            {
                "id": "face_recognition.whitelisted_person",
                "name": "Whitelisted Person",
                "icon": "person_check",
                "attributes": [
                    {"name": "personName", "displayName": "Name"},
                    {"name": "similarity", "displayName": "Similarity", "type": "number"}
                ]
            },
            {
                "id": "face_recognition.blacklisted_person",
                "name": "Blacklisted Person",
                "icon": "person_danger",
                "attributes": [
                    {"name": "personName", "displayName": "Name"},
                    {"name": "similarity", "displayName": "Similarity", "type": "number"}
                ]
            }
        ],
        "eventTypes": [
            {
                "id": "face_recognition.blacklist_match",
                "name": "Blacklist Match Detected",
                "description": "A blacklisted person was detected in the camera view",
                "icon": "warning",
                "isStateDependent": false
            },
            {
                "id": "face_recognition.whitelist_match",
                "name": "Whitelisted Person Detected",
                "description": "A whitelisted (authorized) person was detected",
                "icon": "check_circle",
                "isStateDependent": false
            }
        ],
        "settingsModel": [
            {
                "name": "whitelistDirectory",
                "displayName": "Whitelist Directory",
                "type": "string",
                "defaultValue": "/opt/nx-face-plugin/whitelist"
            },
            {
                "name": "blacklistDirectory",
                "displayName": "Blacklist Directory",
                "type": "string",
                "defaultValue": "/opt/nx-face-plugin/blacklist"
            },
            {
                "name": "minSimilarityThreshold",
                "displayName": "Minimum Similarity Threshold",
                "type": "number",
                "defaultValue": 0.45,
                "minimum": 0.0,
                "maximum": 1.0
            },
            {
                "name": "minFaceConfidence",
                "displayName": "Minimum Face Detection Confidence",
                "type": "number",
                "defaultValue": 0.6,
                "minimum": 0.0,
                "maximum": 1.0
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