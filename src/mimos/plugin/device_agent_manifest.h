#pragma once

#include <string>

namespace mimos
{
namespace plugin
{

static const std::string kDeviceAgentManifest = /*suppress newline*/ 1 + (const char*) R"json(
{
    "supportedTypes": [
        {
            "objectTypeId": "nx.base.Face",
            "attributes": [
                { "name": "Name", "type": "String" },
                { "name": "Confidence", "type": "Float", "minValue": 0, "maxValue": 1 }
            ]
        }
    ]
}
)json";

} // namespace plugin
} // namespace mimos