// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/
#pragma once

#include <opencv2/imgproc.hpp>
#include <nx/sdk/analytics/i_uncompressed_video_frame.h>
#include <nx/kit/debug.h>

namespace mimos {
namespace face_plugin {

/**
 * Zero-copy wrapper for Nx uncompressed video frame into OpenCV Mat.
 * Assumes BGR (requested in Engine manifest).
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
        index(index),
        cvMat(frame->height(),
              frame->width(),
              CV_8UC3,  // BGR24 (safe because Engine requests it)
              (void*) frame->data(0),
              static_cast<size_t>(frame->lineSize(0)))
    {
        // Optional: Verify format at runtime
        if (frame->pixelFormat() != nx::sdk::analytics::IUncompressedVideoFrame0::PixelFormat::bgr)
        {
            // Log or handle (e.g., convert)
            // For now, assume BGR as requested
            NX_PRINT << "Error: Frame format is not BGR as expected.";
        }
    }
};

} // namespace face_plugin
} // namespace mimos