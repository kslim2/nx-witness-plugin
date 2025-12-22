// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "device_agent.h"

#include <chrono>
#include <exception>
#include <cctype>

#include <opencv2/core.hpp>
#include <opencv2/dnn/dnn.hpp>

#include <nx/sdk/analytics/helpers/event_metadata.h>
#include <nx/sdk/analytics/helpers/event_metadata_packet.h>
#include <nx/sdk/analytics/helpers/object_metadata.h>
#include <nx/sdk/analytics/helpers/object_metadata_packet.h>
#include <nx/sdk/helpers/string.h>

#include "detection.h"
#include "exceptions.h"
#include "frame.h"

namespace mimos {
namespace face_plugin {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

using namespace std::string_literals;

/**
 * @param deviceInfo Various information about the related device, such as its id, vendor, model,
 *     etc.
 */
DeviceAgent::DeviceAgent(
    const nx::sdk::IDeviceInfo* deviceInfo,
    std::filesystem::path pluginHomeDir):
    // Call the DeviceAgent helper class constructor telling it to verbosely report to stderr.
    ConsumingDeviceAgent(deviceInfo, /*enableOutput*/ true),
    m_objectDetector(std::make_unique<ObjectDetector>(
        pluginHomeDir / "models",
        pluginHomeDir / "face_db"
    )),
    m_objectTracker(std::make_unique<ObjectTracker>())
{
    /**
     * updated code
     * =============
     * 
     * create the socket.
     */
}

DeviceAgent::~DeviceAgent()
{
}

/**
 * Called when the Server sends a new uncompressed frame from a camera.
 */
bool DeviceAgent::pushUncompressedVideoFrame(const IUncompressedVideoFrame* videoFrame)
{
    m_terminated = m_terminated || m_objectDetector->isTerminated();
    if (m_terminated)
    {
        if (!m_terminatedPrevious)
        {
            pushPluginDiagnosticEvent(
                IPluginDiagnosticEvent::Level::error,
                "Plugin is in broken state.",
                "Disable the plugin.");
            m_terminatedPrevious = true;
        }
        return true;
    }

    // Detecting objects only on every `kDetectionFramePeriod` frame.
    if (m_frameIndex % kDetectionFramePeriod == 0)
    {
        const MetadataPacketList metadataPackets = processFrame(videoFrame);
        for (const Ptr<IMetadataPacket>& metadataPacket: metadataPackets)
        {
            metadataPacket->addRef();
            pushMetadataPacket(metadataPacket.get());
        }
    }

    ++m_frameIndex;

    return true;
}

void DeviceAgent::doSetNeededMetadataTypes(
    nx::sdk::Result<void>* outValue,
    const nx::sdk::analytics::IMetadataTypes* /*neededMetadataTypes*/)
{
    if (m_terminated)
        return;

    try
    {
        m_objectDetector->ensureInitialized();
    }
    catch (const ObjectDetectorInitializationError& e)
    {
        *outValue = {ErrorCode::otherError, new String(e.what())};
        m_terminated = true;
    }
    catch (const ObjectDetectorIsTerminatedError& /*e*/)
    {
        m_terminated = true;
    }
};

//-------------------------------------------------------------------------------------------------
// private

DeviceAgent::MetadataPacketList DeviceAgent::eventsToEventMetadataPacketList(
    const EventList& events,
    int64_t timestampUs)
{
    if (events.empty())
        return {};

    MetadataPacketList result;

    const auto objectDetectedEventMetadataPacket = makePtr<EventMetadataPacket>();

    for (const std::shared_ptr<Event>& event: events)
    {
        const auto eventMetadata = makePtr<EventMetadata>();

        if (event->eventType == EventType::detection_started ||
            event->eventType == EventType::detection_finished)
        {
            static const std::string kStartedSuffix = " STARTED";
            static const std::string kFinishedSuffix = " FINISHED";

            const std::string suffix = (event->eventType == EventType::detection_started) ?
                kStartedSuffix : kFinishedSuffix;
            std::string caption = kFaceClassLabelCapitalized;
            caption += " detection";
            caption += suffix;
            const std::string description = caption;

            eventMetadata->setCaption(caption);
            eventMetadata->setDescription(description);
            eventMetadata->setIsActive(event->eventType == EventType::detection_started);
            eventMetadata->setTypeId(kProlongedDetectionEventType);

            const auto eventMetadataPacket = makePtr<EventMetadataPacket>();
            eventMetadataPacket->addItem(eventMetadata.get());
            eventMetadataPacket->setTimestampUs(event->timestampUs);
            result.push_back(eventMetadataPacket);
        }
        else if (event->eventType == EventType::object_detected)
        {
            std::string caption = event->classLabel + kDetectionEventCaptionSuffix;
            caption[0] = (char) toupper(caption[0]);
            std::string description = event->classLabel + kDetectionEventDescriptionSuffix;
            description[0] = (char) toupper(description[0]);

            eventMetadata->setCaption(caption);
            eventMetadata->setDescription(description);
            eventMetadata->setIsActive(true);
            eventMetadata->setTypeId(kDetectionEventType);

            objectDetectedEventMetadataPacket->addItem(eventMetadata.get());
        }
    }

    objectDetectedEventMetadataPacket->setTimestampUs(timestampUs);
    result.push_back(objectDetectedEventMetadataPacket);

    return result;
}

Ptr<ObjectMetadataPacket> DeviceAgent::detectionsToObjectMetadataPacket(
    const DetectionList& detections,
    int64_t timestampUs)
{
    if (detections.empty())
        return nullptr;

    const auto objectMetadataPacket = makePtr<ObjectMetadataPacket>();
    objectMetadataPacket->setTimestampUs(timestampUs);

    for (const std::shared_ptr<Detection>& detection : detections)
    {
        const auto objectMetadata = makePtr<ObjectMetadata>();

        // Basic object info
        objectMetadata->setBoundingBox(detection->boundingBox);
        objectMetadata->setConfidence(detection->confidence);
        objectMetadata->setTrackId(detection->trackId);
        objectMetadata->setTypeId(kFaceObjectType);  // "mimos.face"

        // === RECOGNITION ATTRIBUTES (THIS IS THE KEY PART) ===
        // Name of recognized person (or "unknown")
        objectMetadata->addAttribute(makePtr<Attribute>(
            "Name", detection->recognizedName));

        // Watchlist: "whitelist", "blacklist", or "unknown"
        objectMetadata->addAttribute(makePtr<Attribute>(
            "Watchlist", detection->watchlist));

        // Similarity score (as percentage)
        objectMetadata->addAttribute(makePtr<Attribute>(
            "Similarity",
            nx::kit::utils::format("%.1f%%", detection->similarityScore * 100.0f)));

        // Optional: Detection confidence
        objectMetadata->addAttribute(makePtr<Attribute>(
            "Detection Confidence",
            nx::kit::utils::format("%.1f%%", detection->confidence * 100.0f)));

        objectMetadataPacket->addItem(objectMetadata.get());
    }

    return objectMetadataPacket;
}

void DeviceAgent::reinitializeObjectTrackerOnFrameSizeChanges(const Frame& frame)
{
    const bool frameSizeUnset = m_previousFrameWidth == 0 && m_previousFrameHeight == 0;
    if (frameSizeUnset)
    {
        m_previousFrameWidth = frame.width;
        m_previousFrameHeight = frame.height;
        return;
    }

    const bool frameSizeChanged = frame.width != m_previousFrameWidth ||
        frame.height != m_previousFrameHeight;
    if (frameSizeChanged)
    {
        m_objectTracker = std::make_unique<ObjectTracker>();
        m_previousFrameWidth = frame.width;
        m_previousFrameHeight = frame.height;
    }
}

DeviceAgent::MetadataPacketList DeviceAgent::processFrame(
    const IUncompressedVideoFrame* videoFrame)
{
    const Frame frame(videoFrame, m_frameIndex);

    reinitializeObjectTrackerOnFrameSizeChanges(frame);

    try
    {
        DetectionList detections = m_objectDetector->run(frame);
        ObjectTracker::Result objectTrackerResult = m_objectTracker->run(frame, detections);
        const auto& objectMetadataPacket =
            detectionsToObjectMetadataPacket(objectTrackerResult.detections, frame.timestampUs);
        const auto& eventMetadataPacketList = eventsToEventMetadataPacketList(
            objectTrackerResult.events, frame.timestampUs);
        MetadataPacketList result;
        for (const auto& detection : objectTrackerResult.detections)
        {
            if (detection->watchlist == "blacklist" && detection->recognizedName != "unknown")
            {
                auto eventPacket = makePtr<EventMetadataPacket>();
                auto event = makePtr<EventMetadata>();
                event->setTypeId("mimos.face.blacklist");
                event->setCaption("Blacklisted person: " + detection->recognizedName);
                event->setDescription("Similarity: " + std::to_string(detection->similarityScore * 100) + "%");
                event->setIsActive(true);
                eventPacket->addItem(event.get());
                eventPacket->setTimestampUs(frame.timestampUs);
                result.push_back(eventPacket);
            }
            else if (detection->watchlist == "whitelist" && detection->recognizedName != "unknown")
            {
                auto eventPacket = makePtr<EventMetadataPacket>();
                auto event = makePtr<EventMetadata>();
                event->setTypeId("mimos.face.recognized.whitelist");
                event->setCaption("Whitelisted person: " + detection->recognizedName);
                event->setDescription("Similarity: " + std::to_string(detection->similarityScore * 100) + "%");
                event->setIsActive(true);
                eventPacket->addItem(event.get());
                eventPacket->setTimestampUs(frame.timestampUs);
                result.push_back(eventPacket);
            }
        }
        result.insert(
            result.end(),
            std::make_move_iterator(eventMetadataPacketList.begin()),
            std::make_move_iterator(eventMetadataPacketList.end()));
        return result;
    }
    catch (const ObjectDetectionError& e)
    {
        pushPluginDiagnosticEvent(
            IPluginDiagnosticEvent::Level::error,
            "Object detection error.",
            e.what());
        m_terminated = true;
    }
    catch (const ObjectTrackingError& e)
    {
        pushPluginDiagnosticEvent(
            IPluginDiagnosticEvent::Level::error,
            "Object tracking error.",
            e.what());
        m_terminated = true;
    }

    return {};
}

} // namespace face_plugin
} // namespace mimos
