#include "device_agent.h"

#include <nx/sdk/analytics/helpers/object_metadata.h>
#include <nx/sdk/analytics/helpers/object_metadata_packet.h>
#include <nx/sdk/analytics/helpers/event_metadata.h>
#include <nx/sdk/analytics/helpers/event_metadata_packet.h>
#include <nx/sdk/helpers/uuid_helper.h>

#include "device_agent_manifest.h"
#include "utils.h"

#include <nx/kit/debug.h>   // NX_PRINT, NX_KIT_LOG_*
#include <nx/sdk/log.h>

namespace mimos
{
namespace plugin
{
namespace face_recognition
{

using namespace nx::sdk;
using namespace nx::sdk::analytics;

DeviceAgent::DeviceAgent(
    const IDeviceInfo* deviceInfo,
    std::shared_ptr<std::map<std::string, std::vector<float>>> whitelist,
    std::shared_ptr<std::map<std::string, std::vector<float>>> blacklist)
    :
    ConsumingDeviceAgent(deviceInfo, ini().enabledByDefault),
    m_whitelist(whitelist),
    m_blacklist(blacklist)
{
    NX_PRINT << "DeviceAgent created for device: " << deviceInfo->id().toStdString();
}

DeviceAgent::~DeviceAgent()
{
    NX_PRINT << "DeviceAgent destroyed";
}

std::string DeviceAgent::manifestString() const
{
    return kDeviceAgentManifest;
}

bool DeviceAgent::pushCompressedVideoFrame(const ICompressedVideoPacket* videoPacket)
{
    if (!m_enabled.load())
        return true;

    ++m_frameCounter;

    // Skip frames according to configured interval
    if (m_processEveryNFrames.load() > 0 &&
        (m_frameCounter.load() % (m_processEveryNFrames.load() + 1)) != 0)
    {
        return true;
    }

    processFrame(videoPacket);
    return true;
}

void DeviceAgent::processFrame(const ICompressedVideoPacket* packet)
{
    // TODO: Implement real face detection & recognition here
    //
    // Steps you need to implement:
    //
    // 1. Decode frame to image (use your preferred decoder / OpenCV / FFmpeg wrapper)
    // 2. Run RetinaFace.onnx → get bounding boxes + landmarks + scores
    // 3. For each detection with score >= m_minConfidence:
    //    - crop & align face using landmarks
    //    - run ArcFace.onnx → get 512-dim embedding
    //    - normalize embedding (L2 norm)
    //    - compare to whitelist & blacklist using cosine similarity
    //    - find best match (use findBestMatch from utils.h)
    // 4. Collect results → FaceResult struct (bbox, name, similarity, isWhitelist, confidence, etc.)
    // 5. Create metadata packet
    // 6. If blacklist match → create & push event

    // Placeholder: simulate some detections (remove in real implementation)
    std::vector<FaceResult> simulatedResults;

    // Example simulated detection
    FaceResult sim;
    sim.boundingBox = {0.3f, 0.2f, 0.4f, 0.5f};  // x, y, width, height [0..1]
    sim.confidence = 0.88f;

    // Try to find match (in real code this comes from ArcFace)
    auto queryEmbedding = std::vector<float>(512, 0.01f); // dummy

    auto wlMatch = findBestMatch(queryEmbedding, *m_whitelist, m_matchThreshold);
    auto blMatch = findBestMatch(queryEmbedding, *m_blacklist, m_matchThreshold);

    if (blMatch.second >= m_matchThreshold)
    {
        sim.name = blMatch.first;
        sim.similarity = blMatch.second;
        sim.isBlacklisted = true;
        simulatedResults.push_back(sim);

        if (m_blacklistAlerts)
            pushBlacklistEvent(blMatch.first, blMatch.second, packet->timestampUs());
    }
    else if (wlMatch.second >= m_matchThreshold)
    {
        sim.name = wlMatch.first;
        sim.similarity = wlMatch.second;
        sim.isBlacklisted = false;
        simulatedResults.push_back(sim);

        if (m_whitelistEvents)
        {
            // Optional: push whitelist event if enabled
        }
    }
    else
    {
        sim.name = "Unknown";
        sim.similarity = 0.0f;
        sim.isBlacklisted = false;
        simulatedResults.push_back(sim);
    }

    // Create and push metadata
    auto metadataPacket = createMetadataPacket(packet->timestampUs(), simulatedResults);
    if (metadataPacket)
        pushMetadataPacket(metadataPacket.releasePtr());
}

nx::sdk::Ptr<IMetadataPacket> DeviceAgent::createMetadataPacket(
    int64_t timestampUs,
    const std::vector<FaceResult>& faces)
{
    if (faces.empty())
        return nullptr;

    auto packet = makePtr<ObjectMetadataPacket>();
    packet->setTimestampUs(timestampUs);

    for (const auto& face : faces)
    {
        auto obj = createFaceObject(face, timestampUs);
        if (obj)
            packet->addItem(obj.releasePtr());
    }

    return packet;
}

nx::sdk::Ptr<ObjectMetadata> DeviceAgent::createFaceObject(
    const FaceResult& result,
    int64_t /*timestampUs*/)
{
    std::string typeId;

    if (result.isBlacklisted)
        typeId = "face_recognition.blacklisted";
    else if (result.name != "Unknown")
        typeId = "face_recognition.whitelisted";
    else
        typeId = "nx.base.Person";

    auto obj = makePtr<ObjectMetadata>();
    obj->setTypeId(typeId);
    obj->setBoundingBox(result.boundingBox);

    // Generate consistent track ID (very basic version)
    // In real implementation you should use better tracking (IoU, embedding distance, etc.)
    int faceIndex = m_nextTrackIdCounter++;  // simplistic
    auto it = m_faceTrackIds.find(faceIndex);
    if (it == m_faceTrackIds.end())
    {
        auto trackId = UuidHelper::randomUuid();
        m_faceTrackIds[faceIndex] = trackId;
        obj->setTrackId(trackId);
    }
    else
    {
        obj->setTrackId(it->second);
    }

    // Add attributes
    obj->addAttribute(makePtr<Attribute>("Name", result.name));
    obj->addAttribute(makePtr<Attribute>("Similarity", std::to_string(result.similarity)));
    obj->addAttribute(makePtr<Attribute>("Confidence", std::to_string(result.confidence)));

    if (result.isBlacklisted)
        obj->addAttribute(makePtr<Attribute>("ListType", "blacklist"));
    else if (result.name != "Unknown")
        obj->addAttribute(makePtr<Attribute>("ListType", "whitelist"));
    else
        obj->addAttribute(makePtr<Attribute>("ListType", "unknown"));

    return obj;
}

void DeviceAgent::pushBlacklistEvent(
    const std::string& personName,
    float similarity,
    int64_t timestampUs)
{
    auto eventPacket = makePtr<EventMetadataPacket>();
    eventPacket->setTimestampUs(timestampUs);

    auto event = makePtr<EventMetadata>();
    event->setTypeId("face_recognition.blacklist_alert");
    event->setCaption("Blacklisted person detected: " + personName);
    event->addAttribute(makePtr<Attribute>("personName", personName));
    event->addAttribute(makePtr<Attribute>("similarity", std::to_string(similarity)));

    eventPacket->addItem(event.releasePtr());
    pushMetadataPacket(eventPacket.releasePtr());
}

nx::sdk::Result<const ISettingsResponse*> DeviceAgent::settingsReceived()
{
    std::lock_guard<std::mutex> lock(m_settingsMutex);

    const auto& settings = currentSettings();

    auto getBool = [&](const std::string& key, bool defaultVal) {
        auto it = settings.find(key);
        return (it != settings.end()) ? toBool(it->second) : defaultVal;
    };

    auto getFloat = [&](const std::string& key, float defaultVal) {
        auto it = settings.find(key);
        if (it == settings.end()) return defaultVal;
        try { return std::stof(it->second); } catch (...) { return defaultVal; }
    };

    auto getInt = [&](const std::string& key, int defaultVal) {
        auto it = settings.find(key);
        if (it == settings.end()) return defaultVal;
        try { return std::stoi(it->second); } catch (...) { return defaultVal; }
    };

    m_enabled.store(                 getBool("enableRecognition", true));
    m_matchThreshold.store(          getFloat("matchThreshold", ini().matchThreshold));
    m_minConfidence.store(           getFloat("minDetectionConfidence", ini().minDetectionConfidence));
    m_processEveryNFrames.store(     getInt("processingIntervalFrames", ini().faceProcessingIntervalFrames));
    m_blacklistAlerts.store(         getBool("enableBlacklistAlerts", ini().enableBlacklistAlerts));
    m_whitelistEvents.store(         getBool("enableWhitelistEvents", ini().enableWhitelistEvents));

    // Optional: handle "reloadEmbeddings" button if you implement it
    // if (settings.count("reloadEmbeddings") && toBool(settings.at("reloadEmbeddings")))
    // {
    //     // You could reload from disk here, but since Engine holds shared_ptr,
    //     // you might need a mechanism to signal Engine to reload.
    // }

    return nullptr;
}

void DeviceAgent::doSetNeededMetadataTypes(
    Result<void>* /*outValue*/,
    const IMetadataTypes* /*neededMetadataTypes*/)
{
    // We don't request any specific metadata from the server
}

} // namespace face_recognition
} // namespace plugin
} // namespace mimos