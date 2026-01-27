#include "object_attribute.h"

namespace mimos
{
namespace plugin
{

/**
 * This map now only contains the Face object type. 
 * We use the constants defined in the header to ensure consistency.
 */
const std::map<std::string, std::map<std::string, std::string>> kObjectAttributes = {
    {
        kFaceObjectType, // "nx.base.Face"
        {
            {Attributes::kName, "Unknown"},
            {Attributes::kConfidence, "0.0"}
        }
    }
};

} // namespace plugin
} // namespace mimos