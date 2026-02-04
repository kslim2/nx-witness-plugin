/**
 * This is optional but very useful part of Nx analytics plugin, it tells
 * Nx Witness:
 * 
 * - which object types your plugin can produce.
 * - which attributes (metadata fields) can be attached to each object type.
 * 
 * This information is useful in:
 * - The Rule Engine (when creating rules based on the object/ attributes)
 * - The search/filter interface
 * - The timeline/object list display
 * - Event notification
 */

#pragma once

#include <string>

namespace mimos
{
namespace plugin
{
namespace face_recognition
{

static const std::string kDeviceAgentManifest = R"json(
{
    "supportedTypes": [
        {
            "objectTypeId": "nx.base.Person",
            "attributes": [
                "Name",
                "Similarity",
                "ListType",
                "Confidence"
            ]
        },
        {
            "objectTypeId": "face_recognition.whitelisted",
            "attributes": [
                "Name",
                "Similarity",
                "Confidence"
            ]
        },
        {
            "objectTypeId": "face_recognition.blacklisted",
            "attributes": [
                "Name",
                "Similarity",
                "Confidence"
            ]
        }
    ]
}
)json";

} // namespace face_recognition
} // namespace plugin
} // namespace mimos