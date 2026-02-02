#pragma once

#include <map>
#include <string>
#include <vector>
#include <optional>
#include <filesystem>

#include <nx/sdk/i_string_map.h>
#include <nx/sdk/ptr.h>

namespace mimos
{
namespace plugin
{
namespace face_recognition
{

bool toBool(std::string str);
bool startsWith(const std::string& str, const std::string& prefix);

template<typename T>
T clamp(const T& value, const T& lowerBound, const T& upperBound)
{
    if (value < lowerBound)
        return lowerBound;
    if (value > upperBound)
        return upperBound;
    return value;
}

/**
 * Load entire file content into memory as vector<char>
 */
std::vector<char> loadFile(const std::string& path);

/**
 * Guess mime type from file extension (mainly for images)
 */
std::string imageFormatFromPath(const std::string& path);

bool isHttpOrHttpsUrl(const std::string& path);

/**
 * Join vector of strings with delimiter
 */
std::string join(
    const std::vector<std::string>& strings,
    const std::string& delimiter,
    const std::string& itemPrefix = std::string(),
    const std::string& itemPostfix = std::string()
);

/**
 * Convert NX SDK string map to std::map
 */
std::map<std::string, std::string> toStdMap(const nx::sdk::Ptr<const nx::sdk::IStringMap>& sdkMap);

/**
 * Simple optional-like class (original from stub)
 */
template<typename T>
class SimpleOptional
{
public:
    SimpleOptional() = default;
    SimpleOptional(const T& value) : m_value(value), m_isInitialized(true) {}
    
    template<typename U>
    SimpleOptional(const SimpleOptional<U>& other)
        : m_value(other.value()), m_isInitialized(other.isInitialized()) {}

    const T* operator->() const { return m_isInitialized ? &m_value : nullptr; }
    T* operator->() { return m_isInitialized ? &m_value : nullptr; }
    
    const T& operator*() const { return m_value; }
    T& operator*() { return m_value; }

    template<typename U>
    SimpleOptional& operator=(const SimpleOptional<U>& other)
    {
        m_value = other.value();
        m_isInitialized = other.isInitialized();
        return *this;
    }

    template<typename U>
    SimpleOptional& operator=(U&& value)
    {
        m_value = std::forward<U>(value);
        m_isInitialized = true;
        return *this;
    }

    explicit operator bool() const { return m_isInitialized; }
    const T& value() const { return m_value; }
    bool isInitialized() const { return m_isInitialized; }
    void reset() { m_isInitialized = false; }

private:
    T m_value{};
    bool m_isInitialized = false;
};

// -------------------------------------------------------------
// Added function for face recognition plugin
// -------------------------------------------------------------

/**
 * Load a single .npy file containing a face embedding (typically 512 floats)
 * Return empty vector on failure
 */
std::vector<float> loadNpyEmbedding(const std::string& path);

/**
 * Load all .npy files from a directory into a name -> embedding map
 * Filename (without .npy) becomes the person name
 */
std::map<std::string, std::vector<float>> loadEmbeddingsFromDirectory(
    const std::string& directoryPath
);

/**
 * Compute consine similarity between two normalized embeddings
 * (ArcFace embeddings are usually L2-normalized)
 * Returns value in [-1, 1], higher = more similar
 */
float cosineSimilarity(const std::vector<float>&a, const std::vector<float>& b);

/**
 * Find the best match in a map of known embeddings
 * Returns {name, similarity} of best match, or {"", -1} if none above threshold
 */
std::pair<std::string, float> findBestMatch(
    const std::vector<float>& queryEmbedding,
    const std::map<std::string, std::vector<float>>& knownEmbeddings,
    float minSimilarityThreshold = 0.45f);

} // namespace face_recognition
} // namespace plugin
} // namespace mimos