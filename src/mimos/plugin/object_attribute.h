#pragma once

#include <map>
#include <string>

namespace mimos
{
namespace plugin
{
namespace face_recognition
{

/**
 * Static map of example / demo attribute values for different object types.
 * Used mainly for:
 *  - plugin testing and debugging
 *  - providing fallback or sample metadata when real detection is not active
 *  - helping the VMS UI understand possible attribute types/values
 *
 * In production code you usually generate real values from inference results.
 */
extern const std::map<
    /* objectTypeId */ std::string,
    std::map</* attributeName */ std::string, /* attributeValue */ std::string>
> kObjectAttributes;

} // namespace face_recognition
} // namespace plugin
} // namespace mimos