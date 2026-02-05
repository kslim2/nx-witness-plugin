#include "object_attribute.h"

namespace mimos
{
namespace plugin
{
namespace face_recognition
{

const std::map<std::string, std::map<std::string, std::string>> kObjectAttributes = {
    {
        "nx.base.Person",
        {
            // Generic detected face / person (no strong match or low confidence)
            {"Name",         "Unknown Person"},
            {"Similarity",   "0.12"},
            {"ListType",     "none"},
            {"Confidence",   "0.68"}
        }
    },
    {
        "face_recognition.whitelisted",
        {
            // Example of a successful whitelist match
            {"Name",         "Alice Tan"},
            {"Similarity",   "0.78"},
            {"Confidence",   "0.92"},
            // You can add more if you want to expose other metadata
            // {"AgeGroup",   "Adult"},
            // {"Gender",     "Female"}
        }
    },
    {
        "face_recognition.blacklisted",
        {
            // Example of a blacklist match (for testing / demo)
            {"Name",         "John Lim (Wanted)"},
            {"Similarity",   "0.85"},
            {"Confidence",   "0.89"},
            // Optional: you can add alert-related info
            // {"Reason",     "Theft suspect"},
            // {"ListType",   "blacklist"}
        }
    }
};

} // namespace face_recognition
} // namespace plugin
} // namespace mimos