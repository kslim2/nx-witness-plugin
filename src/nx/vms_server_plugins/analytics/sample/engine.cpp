// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "engine.h"

#include "device_agent.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace sample {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

Engine::Engine():
    // Call the DeviceAgent helper class constructor telling it to verbosely report to stderr.
    nx::sdk::analytics::Engine(/*enableOutput*/ true)
{
}

Engine::~Engine()
{
}

/**
 * Called when the Server opens a video-connection to the camera if the plugin is enabled for this
 * camera.
 *
 * @param outResult The pointer to the structure which needs to be filled with the resulting value
 *     or the error information.
 * @param deviceInfo Contains various information about the related device such as its id, vendor,
 *     model, etc.
 */
void Engine::doObtainDeviceAgent(Result<IDeviceAgent*>* outResult, const IDeviceInfo* deviceInfo)
{
    *outResult = new DeviceAgent(deviceInfo);
}

/**
 * @return JSON with the particular structure. Note that it is possible to fill in the values
 *     that are not known at compile time, but should not depend on the Engine settings.
 */
std::string Engine::manifestString() const
{
    return /*suppress newline*/ 1 + (const char*) R"json(
    {
        "typeLibrary":
        {
            "objectTypes":
            [
                {
                    "id": "mimos.face",
                    "name": "Face",
                    "attributes":
                    [
                        { "type": "String", "name": "personName", "description": "Recognized person name" },
                        { "type": "String", "name": "listType", "description": "whitelist or blacklist" },
                        { "type": "Number", "name": "confidence", "subtype": "float", "unit": "%", "minValue": 0, "maxValue": 100 }
                    ]
                }
            ],
            "eventTypes":
            [
                {
                    "id": "mimos.face.whitelist_match",
                    "name": "Whitelisted Person Detected",
                    "description": "A person from the whitelist was recognized"
                },
                {
                    "id": "mimos.face.blacklist_match",
                    "name": "Blacklisted Person Detected",
                    "description": "A person from the blacklist was recognized"
                }
            ]
        },
        "supportedTypes":
        [
            { "objectTypeId": "nx.base.Camera" }
        ],
        "streamSelection":
        {
            "primaryStreamPreference": "high"
        },
        "deviceAgent":
        {
            "capabilities": "needUncompressedVideoFrames|disableLiveStreamIfNoActiveObjects"
        }
    }
    )json";
}

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
