/**
 * The DeviceAgent sits in the middle of the video pipeline. It
 * receives video frames, processes them (or simulates processing,
 * since this is a "stub"), and sends metadata (like bounding box
 * ) back to the VMS.
 */

#pragma once

#include <set>
#include <thread>

#include <nx/sdk/analytics/helpers/consuming_device_agent.h>
#include <nx/sdk/helpers/uuid_helper.h>

# include "engine.h"

namespace mimos
{
namespace plugin
{

class DeviceAgent: public nx::sdk::analytics::ConsumingDeviceAgent
{

public:
    DeviceAgent(const nx::sdk::IDeviceInfo* deviceInfo);
    virtual ~DeviceAgent() override;

protected:

    /**
     * Similar to Engine, but specific to the device. It defines
     * what this specific agent can do (e.g. "I can detect license
     * plates on this specific high-res camera.")
     */
    virtual std::string manifestString() const override;

    /**
     * This is the heart of the agent. The VMS calls this function
     * every time a new video frame arrives from the camera. The
     * agent can then decode the frame, run an AI model, and detect
     * objects.
     */
    virtual bool pushCompressedVideoFrame(
        const nx::sdk::analytics::ICompressedVideoPacket* videoFrame
    ) override;

    /**
     * This is an optimization. The VMS tells the agent, "I only 
     * care about "Person" detections right now." The agent can
     * save CPU power by not looking for cars or dogs.
     */
    virtual void doSetNeededMetadataTypes(
        nx::sdk::Result<void>* outValue,
        const nx::sdk::analytics::IMetadataTypes* neededMetadataTypes
    ) override;

private:
    /**
     * This is a helper function used for Object Tracking. When
     * an AI detects a person, it assigns them a "Track ID". This
     * ID must stay the same as long as that person is on screen.
     */
    nx::sdk::Uuid trackIdByTrackIndex(int trackIndex);

    /**
     * This likely creates "fake" bounding boxes or detection data
     * to show how the metadata should be formatted and sent back
     * to the server.
     */
    nx::sdk::Ptr<nx::sdk::analytics::IMetadataPacket> generateObjectMetadataPacket(
        int64_t frameTimestampUs
    );

private:

    /**
     * Crucial for Thread Safety. Since video frames come in on one
     * thread and setting changes might come in on another, the mutex
     * prevents the plugin from crashing dur to simultaneous data
     * access.
     */
    mutable std::mutex m_mutex;

    /**
     * This is a counter that keep tracks of how many video frames
     * have passed. Every time `pushCompressedVideoFrame` is called,
     * the code likely runs `m_frameIndex++;`.
     */
    int m_frameIndex = 0;

    /**
     * Likely used to simulate processing lag or sync the AI results
     * with the video timeline.
     */
    int m_timestampShiftMs = 0;

    /**
     * 
     */
    bool m_sendAttributes = true;

    /**
     * Used to maintain "Object Tracking". It ensures that a box
     * around a person on Frame 1 is recognized as the same person
     * on Frame 2.
     */
    std::vector<nx::sdk::Uuid> m_trackIds;

    /**
     * A list of what the plugin is currently looking for.
     * 
     * example: {"nx.base.Person", "nx.base.Vehicle"}
     */
    std::set<std::string> m_objectTypeIdsToGenerate;

// add to perform face recognition using Arcface and Retinaface
private:
    // inference engine wrappers (Retinaface / Arcface)
    std::unique_ptr<RetinaFaceDetector> m_detector;
    std::unique_ptr<ArcFaceRecognizer> m_recognizer;

    // a simple daabase of known embeddings
    // key: person's name; value: vector of floats (arcface embedding)
    std::map<std::string, std::vector<float>> m_faceGallery;

    // helper to decode video frames into OpenCV Mats
    cv::Mat decodeFrame(
        const nx::sdk::analytics::ICompressedVideoPacket* videoFrame
    );

}

} // namespace plugin
} // namespace mimos