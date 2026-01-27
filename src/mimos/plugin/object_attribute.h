#pragma once

#include <string>
#include <map>

namespace mimos
{
namespace plugin
{

/**
 * standard object type id for faces n the Nx Meta ecosystem
 */
static const std::string kFaceObjectType = "nx.base.Face";

/**
 * Attributes names used by the plugin.
 * Using constants prevent typos in .cpp files
 */
namespace Attributes {
    static const std::string kName = "Name";
    static const std::string kConfidence = "Confidence";
}

} // namespace mimos
} // namespace plugin