// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "device_agent.h"
#include "engine.h"

#include <nx/kit/utils.h>
#include <opencv2/opencv.hpp>  // Ready for face recognition

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace sample {

DeviceAgent::DeviceAgent(const nx::sdk::IDeviceInfo* deviceInfo)
    : nx::sdk::analytics::ConsumingDeviceAgent(deviceInfo, /*enableOutput*/ true)
{
    // TODO: Load models and embeddings here later
    // Example:
    // m_retinaface = cv::dnn::readNet("retinaface.onnx");
    // m_arcface = cv::dnn::readNet("arcface.onnx");
    // loadKnownEmbeddings();
}

DeviceAgent::~DeviceAgent()
{
}

bool DeviceAgent::pushUncompressedVideoFrame(
    const nx::sdk::analytics::IUncompressedVideoFrame* videoFrame)
{
    // This is where the magic will happen:
    // 1. Convert videoFrame → cv::Mat
    // 2. Detect faces with RetinaFace
    // 3. Extract embeddings with ArcFace
    // 4. Compare to whitelist/blacklist
    // 5. Generate ObjectMetadata + EventMetadata

    // For now: just keep the stream alive
    return true;
}

std::string DeviceAgent::manifestString() const
{
    return R"json(
    {
        "capabilities": "needUncompressedVideoFrames|disableLiveStreamIfNoActiveObjects"
    }
    )json";
}

void DeviceAgent::doSetNeededMetadataTypes(
    nx::sdk::Result<void>* outResult,
    const nx::sdk::analytics::IMetadataTypes* /*neededMetadataTypes*/)
{
    // We don't need any external metadata types for now
    *outResult = {};
}

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx