#include "utils.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <numeric>
#include <cmath>
#include <filesystem>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)  // suppress warning about filesystem in older MSVC
#endif

#include <cnpy.h>

namespace mimos
{
namespace plugin
{
namespace face_recognition
{

using namespace nx::sdk;

bool toBool(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str == "true" || str == "1" || str == "yes" || str == "on";
}

bool startsWith(const std::string& str, const std::string& prefix)
{
    return str.rfind(prefix, 0) == 0;
}

std::vector<char> loadFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
        return {};
    return std::vector<char>(std::istreambuf_iterator<char>(file), {});
}

/**
 * Optional feature
 * 
 * Might be useful if you ever want to send images (crops,
 * debug images) to NX or external system.
 */
std::string imageFormatFromPath(const std::string& path)
{
    auto endsWith = [](const std::string& str, const std::string& suffix)
    {
        if (suffix.size() > str.size())
            return false;
        return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
    };

    if (endsWith(path, ".jpg") || endsWith(path, ".jpeg")) return "image/jpeg";
    if (endsWith(path, ".png"))                             return "image/png";
    if (endsWith(path, ".tiff") || endsWith(path, ".tif"))  return "image/tiff";
    if (endsWith(path, ".bmp"))                             return "image/bmp";
    return "";
}

bool isHttpOrHttpsUrl(const std::string& path)
{
    return startsWith(path, "http://") || startsWith(path, "https://");
}

std::string join(
    const std::vector<std::string>& strings,
    const std::string& delimiter,
    const std::string& itemPrefix,
    const std::string& itemPostfix)
{
    std::string result;
    for (size_t i = 0; i < strings.size(); ++i)
    {
        result += itemPrefix + strings[i] + itemPostfix;
        if (i != strings.size() - 1)
            result += delimiter;
    }
    return result;
}

std::map<std::string, std::string> toStdMap(const Ptr<const IStringMap>& sdkMap)
{
    std::map<std::string, std::string> result;
    if (!sdkMap)
        return result;
    for (int i = 0; i < sdkMap->count(); ++i)
        result[sdkMap->key(i)] = sdkMap->value(i);
    return result;
}

// ------------------------------------------
// Add function to face recognition
// ------------------------------------------

std::vector<float> loadNpyEmbedding(const std::string& path)
{
    try
    {
        cnpy::NpyArray arr = cnpy::npyLoad(path);
        
        // We expect a 1D array of floats (512 elements for ArcFace usually)
        if (arr.shape.size() != 1 || arr.type != cnpy::NpyArray::float_type)
            return {};

        std::vector<float> embedding(arr.data<float>(), arr.data<float>() + arr.shape[0]);
        return embedding;
    }
    catch (...)
    {
        return {};
    }
}

std::map<std::string, std::vector<float>> loadEmbeddingsFromDirectory(
    const std::string& directoryPath)
{
    std::map<std::string, std::vector<float>> embeddings;

    if (!std::filesystem::exists(directoryPath) ||
        !std::filesystem::is_directory(directoryPath))
        return embeddings;

    for (const auto& entry : std::filesystem::directory_iterator(directoryPath))
    {
        if (!entry.is_regular_file())
            continue;

        std::string path = entry.path().string();
        std::string ext = entry.path().extension().string();

        if (ext != ".npy" && ext != ".NPY")
            continue;

        std::string name = entry.path().stem().string();
        if (name.empty())
            continue;

        auto emb = loadNpyEmbedding(path);
        if (!emb.empty())
            embeddings[name] = std::move(emb);
    }

    return embeddings;
}

float cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b)
{
    if (a.size() != b.size() || a.empty())
        return -1.0f;

    float dot = 0.0f;
    float normA = 0.0f;
    float normB = 0.0f;

    for (size_t i = 0; i < a.size(); ++i)
    {
        dot += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }

    float denom = std::sqrt(normA) * std::sqrt(normB);
    if (denom < 1e-6f)
        return 0.0f;

    return dot / denom;
}

std::pair<std::string, float> findBestMatch(
    const std::vector<float>& queryEmbedding,
    const std::map<std::string, std::vector<float>>& knownEmbeddings,
    float minSimilarityThreshold)
{
    std::string bestName;
    float bestScore = -1.0f;

    for (const auto& [name, emb] : knownEmbeddings)
    {
        float score = cosineSimilarity(queryEmbedding, emb);
        if (score > bestScore)
        {
            bestScore = score;
            bestName = name;
        }
    }

    if (bestScore >= minSimilarityThreshold)
        return {bestName, bestScore};

    return {"", -1.0f};
}

} // namespace face_recognition
} // namespace plugin
} // namespace mimos