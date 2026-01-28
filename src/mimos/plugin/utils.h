#pragma once

#include <map>
#include <string>
#include <vector>

#include <nx/sdk/i_string_map.h>
#include <nx/sdk/ptr.h>

namespace mimos
{
namespace plugin
{

/**
 * Converts string like "true", "1", "yes" to boolean.
 * Used for reading plugin toggle settings from the VMS GUI.
 */
bool toBool(std::string str);

/**
 * Checks if a setting keys starts with a specific prefix.
 * Essential if you the objectTypeIdToGenerate prefixing logic.
 */
bool startsWith(const std::string& str, const std::string& prefix);

/**
 * Helper to keep values within a range (e.g., confidence 0.0 to 1.0)
 */
template<typename T>
T clamp(const T& value, const T& lowerBound, const T& upperBound)
{
    if (value < lowerBound) return lowerBound;
    if (value > upperBound) return upperBound;

    return value;
}

/**
 * Load binary files from disk.
 * You will use this to load RetinaFace, Arcface, and Face embedding files.
 */
std::vector<char> loadFile(const std::string& path);

/**
 * Converts the SDK's internal IStringMap into a standard C++ map.
 * This is the most important utility for reading camera-specific settings.
 */
std::map<std::string, std::string> toStdMap(
    const nx::sdk::Ptr<const nx::sdk::IStringMap>& sdkMap
);

/**
 * Scans a directory for .npy files and extracts the the person's name
 * from the filename.
 * @param directoryPath Path to the whitelist or blacklist directory.
 * @param isBlacklist Flag to mark the loaded embeddings as blacklisted.
 */
std::vector<FaceEmbedding> loadEmbeddingsFromDir(
    const std::string& directoryPath,
    bool isBlacklist
);

/**
 * Simplified .npy loader
 * Note: Real .npy files have a header. For ArcFace (512 floats)
 * 
 * This helper skips the header and reads the raw floats.
 */
std::vector<float> loadNpyVector(const std::string& path);

} // namespace plugin
} // namespace mimos