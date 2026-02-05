#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <nx/sdk/analytics/helpers/consuming_device_agent.h>
#include <nx/sdk/analytics/i_compressed_video_packet.h>
#include <nx/sdk/analytics/i_uncompressed_video_packet.h>
#include <nx/sdk/helpers/uuid_helper.h>

#include "engine.h"
#include "face_recognition_ini.h"
#include "utils.h"           // loadEmbeddingsFromDirectory, cosineSimilarity, findBestMatch

namespace mimos
{
namespace plugin
{
namespace face_recognition
{

class DeviceAgent : public nx::sdk::analytics::ConsumingDeviceAgent
{
public:
    DeviceAgent(
        const nx::sdk::IDeviceInfo* deviceInfo,
        std::shared_ptr<std::map<std::string, std::vector<float>>> whitelist,
        std::shared_ptr<std::map<std::string, std::vector<float>>> blacklist);

    virtual ~DeviceAgent() override;

protected:
    virtual std::string manifestString() const override;

    /**
     * Main entry point for compressed video frames.
     * We will decode / process frames here (or skip based on interval).
     */
    virtual bool pushCompressedVideoFrame(
        const nx::sdk::analytics::ICompressedVideoPacket* videoPacket) override;

    /**
     * Called when settings are changed from UI or applied.
     */
    virtual nx::sdk::Result<const nx::sdk::ISettingsResponse*> settingsReceived() override;

    /**
     * Optional: can be used to tell server which metadata types we need
     */
    virtual void doSetNeededMetadataTypes(
        nx::sdk::Result<void>* outValue,
        const nx::sdk::analytics::IMetadataTypes* neededMetadataTypes) override;

private:
    // Core processing logic
    void processFrame(const nx::sdk::analytics::ICompressedVideoPacket* packet);

    // Generate metadata packet for detected & recognized faces
    nx::sdk::Ptr<nx::sdk::analytics::IMetadataPacket> createMetadataPacket(
        int64_t timestampUs,
        const std::vector<FaceResult>& detectedFaces);

    // Helper to create object metadata for a single recognized face
    nx::sdk::Ptr<nx::sdk::analytics::ObjectMetadata> createFaceObject(
        const FaceResult& result,
        int64_t timestampUs);

    // Push event when blacklist match is found
    void pushBlacklistEvent(
        const std::string& personName,
        float similarity,
        int64_t timestampUs);

private:
    // Shared (read-only) embeddings from Engine
    std::shared_ptr<std::map<std::string, std::vector<float>>> m_whitelist;
    std::shared_ptr<std::map<std::string, std::vector<float>>> m_blacklist;

    // Runtime state
    std::mutex m_settingsMutex;
    std::atomic<bool> m_enabled{true};
    std::atomic<float> m_matchThreshold{0.45f};
    std::atomic<float> m_minConfidence{0.60f};
    std::atomic<int> m_processEveryNFrames{0};
    std::atomic<bool> m_blacklistAlerts{true};
    std::atomic<bool> m_whitelistEvents{false};

    // Frame counter for skipping frames
    std::atomic<int64_t> m_frameCounter{0};

    // Track IDs for consistent tracking (optional improvement)
    std::map<int, nx::sdk::Uuid> m_faceTrackIds;  // simple face index → track ID
    int m_nextTrackIdCounter = 0;
};

} // namespace face_recognition
} // namespace plugin
} // namespace mimos