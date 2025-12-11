// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <opencv2/dnn.hpp>
#include <opencv2/core/mat.hpp>
#include <mutex>

#include <nx/sdk/helpers/uuid_helper.h>
#include <nx/sdk/uuid.h>

#include "detection.h"
#include "frame.h"

namespace mimos {
namespace face_plugin {

// using OpenCV namespace for convenience within this file
using cv::Mat;
using cv::dnn::Net;



// Structure to simulate a database entry for face recognition
struct KnownFace
{
    std::string name;
    // Mat embedding; // 512-D vector
    std::vector<float> embedding; // storing as vector is safer for persistence
};

using DetectionList =std::vector<std::shared_ptr<Detection>>;

//---- constants for detector model output -------
// NOTE: actual retinaFace post-processing is highly complex. It is assume
// the output Mat is assumed to contain normalized bounding box.
constexpr int kBatchIndex = 0;
constexpr int kClassIndex = 1;
constexpr int kConfidenceIndex = 2;
constexpr int kXMinIndex = 3;
constexpr int kYMinIndex = 4;
constexpr int kXMaxIndex = 5;
constexpr int kYMaxIndex = 6;
constexpr int kDetectionSize = 7; // number of elements per detection

//---- constant for ArcFace (Embedder) -----------
constexpr int kArcFaceEmbeddingSize = 512;
constexpr float kRecognitionThreshold = 0.7f; // threshold for consine similarity

class ObjectDetector
{
public:
    // path to the directory where models are located
    explicit ObjectDetector(std::filesystem::path modelPath, std::filesystem::path faceDbPath);
    // check the detector is initialized
    void ensureInitialized();
    bool isTerminated() const;
    void terminate();
    // initiate the detection
    DetectionList run(const Frame& frame);
    // main processing function, the return type now the DetectionList from detection.h
    DetectionList processData(const Mat& image);

private:
    // load the AI model of both retinaface (detector) and arcface (classifier)
    void loadModel();
    // main processing function
    DetectionList runImpl(const Frame& frame);

private:
    // configuration
    const std::filesystem::path m_modelPath;
    const std::filesystem::path m_faceDBPath;
    std::vector<KnownFace> m_knownFaces; // simulated face database
    

    // detector configuration (RetinaFace input size)
    const cv::Size m_detectorInputSize = {640, 640}; // common RetinaFace size
    const double m_detectorScaleFactor = 1.0;
    const cv::Scalar m_detectorMean = {104.0, 177.0, 123.0}; // common mean for face models
    const float m_confidenceThreshold = 0.7f; // higher threshold for face detection

    // embedder config (ArcFace input size)
    const cv::Size m_embedderInputSize = {112, 112}; // standard ArcFace input size
    const double m_embedderScaleFactor = 1.0 / 255.0; // standard normalization
    const cv::Scalar m_embedderMean = {}; // no mean substraction
    
    // helper function to handle pre-processing (blob creation)
    Mat preprocess(const Mat& image, const cv::Size& size, double scale,
        const cv::Scalar& mean) const;

    // helper function to calculate consine similarity between two embeddings
    float cosineSimilarity(const Mat& vec1, const Mat& vec2) const;

    // helper function to face embedding from database
    void loadFaceDatabase();

    // helper: extract a normalized embedding from a CROPPED image
    // this allow us to reuse logic for both video frames and database images
    Mat getEmbedding(const Mat& faceImage) const;
    
    // --- model definitions ---
    // model 1: detector (RetinaFace - finds face bounding boxes)
    std::unique_ptr<Net> m_net_detector;
    bool m_detectorLoaded = false; // checker to determine whether the detector was successfully loaded

    // model 2: embedder (ArcFace - generates 512D feature vectors)
    std::unique_ptr<Net> m_net_embedder; // renamed to embedder
    bool m_embedderLoaded = false; // checker to determine whether the embedder was successfully loaded
    bool m_terminated = false;
};

} // namespace face_plugin
} // namespace mimos
