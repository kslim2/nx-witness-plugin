// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/
#pragma once

#include <opencv2/imgproc.hpp>
#include <nx/sdk/analytics/i_uncompressed_video_frame.h>
#include <nx/kit/debug.h>

namespace mimos {
namespace face_plugin {

/**
 * Zero-copy wrapper for Nx uncompressed video frame into OpenCV Mat.
 */
struct Frame
{
    const int width;
    const int height;
    const int64_t timestampUs;
    const int64_t index;
    cv::Mat cvMat;

public:
    Frame(const nx::sdk::analytics::IUncompressedVideoFrame* frame, int64_t index):
        width(frame->width()),
        height(frame->height()),
        timestampUs(frame->timestampUs()),
        index(index)
    {
        int cvType = CV_8UC3;  // Default assumption

        switch (frame->pixelFormat())
        {
            case nx::sdk::analytics::PixelFormat::bgr24:
                cvType = CV_8UC3;
                break;
            case nx::sdk::analytics::PixelFormat::rgb24:
                cvType = CV_8UC3;
                NX_KIT_DEBUG(this, "Received RGB frame — consider converting to BGR for models");
                break;
            default:
                NX_KIT_DEBUG(this, "Unsupported pixel format: " << static_cast<int>(frame->pixelFormat()));
                break;
        }

        cvMat = cv::Mat(
            frame->height(),
            frame->width(),
            cvType,
            (void*) frame->data(0),
            static_cast<size_t>(frame->lineSize(0)));
    }
};

} // namespace face_plugin
} // namespace mimos