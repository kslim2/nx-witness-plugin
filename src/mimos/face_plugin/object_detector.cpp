// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "object_detector.h"

#include <algorithm>
#include <fstream>
#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp> // required for dummy database loading
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp> // required for cv::randu
#include "exceptions.h"
#include "frame.h"

// assuming these are defined in exceptions.h and are correct
extern std::string cvExceptionToStdString(const cv::Exception& e);

namespace mimos {
namespace face_plugin {

using namespace std::string_literals;

using cv::Mat;
using cv::dnn::Net;
using cv::dnn::blobFromImage;
using cv::dnn::readNetFromONNX;

ObjectDetector::ObjectDetector(std::filesystem::path modelPath, std::filesystem::path faceDbPath)
    : m_modelPath(std::move(modelPath))
    , m_faceDBPath(std::move(faceDbPath))
{
}

/**
 * helper: simple blob creation wrapper.
 */
Mat ObjectDetector::preprocess(const Mat& image, const cv::Size& size, double scale,
    const cv::Scalar& mean) const
{
    Mat blob;
    blobFromImage(
        image,
        blob,
        scale,
        size,
        mean,
        /*swapRB*/ true,
        /*crop*/ false
    );
    return blob;
}

/**
 * helper: calculates the consine similarity between two normalized embedding vectors.
 */
float ObjectDetector::cosineSimilarity(const Mat& vec1, const Mat& vec2) const
{
    // ensure both mats are valid and of the expected size (1x512 CV_32F)
    if (vec1.empty() || vec2.empty() || vec1.cols != kArcFaceEmbeddingSize|| vec2.cols != kArcFaceEmbeddingSize)
    {
        return -1.0f; // return minimalscore on error
    }
    // assuming vec1 and vec2 are 1x512, CV_32F, and L2 normalized
    // cosine similarity = dot product (since L2 norm is 1)
    double dotProduct = vec1.dot(vec2);

    return static_cast<float>(dotProduct);
}



/**
 * helper function to simulate a database of known faces (embeddings)
 */
void ObjectDetector::loadFaceDatabase()
{
    if (!m_knownFaces.empty())
        return; // already loaded

    const auto dbDir = m_faceDBPath;

    if (!std::filesystem::exists(dbDir) || !std::filesystem::is_directory(dbDir))
    {
        throw ObjectDetectorInitializationError("Face database directory not found: " + dbDir.string());
    }

    constexpr size_t embeddingByteSize = kArcFaceEmbeddingSize * sizeof(float);
    size_t loadedCount = 0;

    // supported subfolders
    const std::vector<std::string> watchlists = {"whitelist", "blacklist"};

    for (const auto& watchlistName: watchlists)
    {
        const auto watchlistDir = dbDir / watchlistName;

        if (!std::filesystem::exists(watchlistDir) || !std::filesystem::is_directory(watchlistDir))
        {
            continue; // skip non-existing watchlist directories
        }

        for (const auto& entry: std::filesystem::directory_iterator(watchlistDir))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".npy")
            {
                const auto dbFilePath = entry.path();
                const std::string name = dbFilePath.stem().string();

                std::ifstream file(dbFilePath, std::ios::binary | std::ios::ate);
                if (!file.is_open()) continue;

                const auto fileSize = file.tellg();
                file.seekg(0, std::ios::beg);

                if (fileSize != embeddingByteSize) continue;

                std::vector<char> buffer(fileSize);
                if (!file.read(buffer.data(), fileSize)) continue;

                const float* rawData = reinterpret_cast<const float*>(buffer.data());

                KnownFace face;
                face.name = name;
                face.watchlist = watchlistName;
                face.embedding.assign(rawData, rawData + kArcFaceEmbeddingSize);

                m_knownFaces.push_back(std::move(face));
                loadedCount++;
            }
        }
    }

    if (loadedCount == 0)
    {
        throw ObjectDetectorInitializationError("No valid face embeddings found in database directory: " + dbDir.string());
    }
}

/**
 * Load the model if it is not loaded, do nothing otherwise. In case of errors terminate the
 * plugin and throw a specialized exception.
 */
void ObjectDetector::ensureInitialized()
{
    if (m_terminated)
    {
        throw ObjectDetectionError(
            "Object detector initialization error: object detector is terminated.");
    }

    // check if both models and the database are loaded
    if (m_embedderLoaded && m_detectorLoaded && !m_knownFaces.empty())
        return;

    try
    {
        loadModel();
        loadFaceDatabase();
    }
    catch (const cv::Exception& e)
    {
        terminate();
        throw ObjectDetectorInitializationError("Loading model: " + cvExceptionToStdString(e));
    }
    catch (const std::exception& e)
    {
        terminate();
        throw ObjectDetectorInitializationError("Loading model: Error: "s + e.what());
    }
}

/**
 * helper: extract a normalized 512D embedding from a CROPPED face image
 */
Mat ObjectDetector::getEmbedding(const Mat& faceImage) const
{
    if (!m_net_embedder || !m_embedderLoaded || faceImage.empty())
        return Mat();

    // 1. preprocess the cropped face image
    const Mat embedderInputBlob = preprocess(
        faceImage, m_embedderInputSize, m_embedderScaleFactor, m_embedderMean
    );

    // 2. run the embedder network
    m_net_embedder->setInput(embedderInputBlob);
    Mat embeddingMat = m_net_embedder->forward().clone(); // this is the 512-D vector

    // 3. reshape and ensure it is a 1x512 vector
    if (embeddingMat.rows * embeddingMat.cols != kArcFaceEmbeddingSize)
    {
        // handle common cases where output is 512x1 instead of 1x512
        if(embeddingMat.rows == kArcFaceEmbeddingSize && embeddingMat.cols == 1)
        {
            embeddingMat = embeddingMat.t(); // transpose to 1x512
            embeddingMat = embeddingMat.clone();
        }
        else
        {
            return Mat(); // return empty on unexpected size
        }
    }

    // 4. L2 Normalization (crucial for accurate recognition)
    if (embeddingMat.total() > 0)
    {
        cv::normalize(embeddingMat, embeddingMat);
    }

    return embeddingMat;
}

/**
 * process the face detection
 */
DetectionList ObjectDetector::processData(const Mat& image)
{
    if (!m_detectorLoaded || !m_embedderLoaded || m_knownFaces.empty())
        return{};

    DetectionList finalDetections;

    // step 1: RetinaFace forward pass (find  faces)
    // prepare input blob for the detector
    const Mat detectorInputblob = preprocess(
        image, m_detectorInputSize, m_detectorScaleFactor, m_detectorMean
    );
    m_net_detector->setInput(detectorInputblob);

    // NOTE: RetinaFace typically requires multiple outputs (Classification, boxes, landmarks)
    // for simplicity, assume the forward() call returns a simplified, aggregated detection
    // matrix
    Mat rawDetections = m_net_detector->forward();

    // safety check and reshape
    if (rawDetections.dims < 4 || rawDetections.size[2] == 0) return {};

    // assume output is 1x1xNx7 (N=detections, 7=propperties)
    const Mat detections(
        /*_rows*/ rawDetections.size[2],
        /*_cols*/ rawDetections.size[3],
        /*_type*/ CV_32F,
        /*_s*/ rawDetections.ptr<float>()
    );

    // step 2: loop and perform arcface embedding (recognition)
    for (int i = 0; i < detections.rows; ++i)
    {
        const float confidence = detections.at<float>(i, kConfidenceIndex);
        if (confidence > m_confidenceThreshold)
        {
            // extract normalized bouding box coordinates
            const float normXMin = detections.at<float>(i, kXMinIndex);
            const float normYMin = detections.at<float>(i, kYMinIndex);
            const float normXMax = detections.at<float>(i, kXMaxIndex);
            const float normYMax = detections.at<float>(i, kYMaxIndex);

            // convert normalized coordinates to pixel values
            const int xMin = static_cast<int>(normXMin * image.cols);
            const int yMin = static_cast<int>(normYMin * image.rows);
            const int xMax = static_cast<int>(normXMax * image.cols);
            const int yMax = static_cast<int>(normYMax * image.rows);

            cv::Rect roi(xMin, yMin, xMax - xMin, yMax - yMin);

            // ensure ROI is valid and clip it to image boundaries
            roi = roi & cv::Rect(0, 0, image.cols, image.rows);

            if (roi.empty() || roi.width < 10 || roi.height < 10) continue;

            // crop and get the normalized 512D embedding
            Mat roiImage = image(roi).clone();
            Mat embeddingMat = getEmbedding(roiImage);

            if (embeddingMat.empty()) continue; // skip if embedding failed
            
            // copy the normalized embedding data from Mat to std::vector<float> for the detection object
            FaceEmbedding embedding(kArcFaceEmbeddingSize);
            std::memcpy(embedding.data(), embeddingMat.ptr<float>(), 
                kArcFaceEmbeddingSize * sizeof(float));
            
            // dummy landmark (are not extracted by the simplified model output)
            Landmarks landmarks(5);

            // step 3: recognition (compare embedding against known faces)
            std::string recognizedName = "unknown";
            std::string recognizedWatchlist = "unknown";
            float bestMatchScore = 0.0f;

            for (const auto& knownFace : m_knownFaces)
            {
                Mat knownEmbeddingMat(1, kArcFaceEmbeddingSize, CV_32F, (void*)knownFace.embedding.data());
                float similarity = cosineSimilarity(embeddingMat, knownEmbeddingMat);

                if (similarity > bestMatchScore)
                {
                    bestMatchScore = similarity;
                    if (bestMatchScore > kRecognitionThreshold)
                    {
                        recognizedName = knownFace.name;
                        recognizedWatchlist = knownFace.watchlist;  // "whitelist" or "blacklist"
                    }
                }
            }

            // Then in Detection creation:
            finalDetections.push_back(
                std::make_shared<Detection>(
                    Detection{
                        /*boundingBox*/ nx::sdk::analytics::Rect(normXMin, normYMin, normXMax - normYMin, normYMax - normYMin),
                        /*confidence*/ confidence,
                        /*landmarks*/ landmarks,
                        /*embedding*/ embedding,
                        /*trackId*/ nx::sdk::Uuid(),  // Tracker will update this
                        /*classLabel*/ kFaceClassLabel,
                        /*recognizedName*/ recognizedName,
                        /*similarityScore*/ bestMatchScore,
                        /*watchlist*/ recognizedWatchlist
                    }
                )
            );
        }
    }
    return finalDetections;
}

bool ObjectDetector::isTerminated() const
{
    return m_terminated;
}

void ObjectDetector::terminate()
{
    m_terminated = true;
}

DetectionList ObjectDetector::run(const Frame& frame)
{
    ensureInitialized();
    if (isTerminated())
        throw ObjectDetectorIsTerminatedError("Detection error: object detector is terminated.");

    try
    {
        return runImpl(frame);
    }
    catch (const cv::Exception& e)
    {
        terminate();
        throw ObjectDetectionError(cvExceptionToStdString(e));
    }
    catch (const std::exception& e)
    {
        terminate();
        throw ObjectDetectionError("Error: "s + e.what());
    }
}

//-------------------------------------------------------------------------------------------------
// private

void ObjectDetector::loadModel()
{
    // load model 1: detector (RetinaFace - finds face bounding boxes)
    static const auto detectorPath = m_modelPath / "retina.onnx";

    // throw exception if the RetinaFace model not found
    if(!std::filesystem::exists(detectorPath))
        throw ObjectDetectorInitializationError("RetinaFace model file not found: " + detectorPath.string());

    // load the RetinaFace model
    m_net_detector = std::make_unique<Net>(
        readNetFromONNX(detectorPath.string())
    );
    m_detectorLoaded = !m_net_detector->empty();

    // if RetinaFace not loaded successfully throw exception
    if(!m_detectorLoaded)
        throw ObjectDetectionError("RetinaFace model loading failed.");

    // set preferred backend (cpu is usually safer for initial setup)
    m_net_detector->setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);

    // load model 2 : embedder (Arcface - generate 512D feature vector)
    static const auto embedderPath = m_modelPath / "arc.onnx";

    // throw exception if embedder is not found
    if (!std::filesystem::exists(embedderPath))
        throw ObjectDetectorInitializationError("ArcFace model file not found: " + embedderPath.string());

    // load the arcface model
    m_net_embedder = std::make_unique<Net>(
        readNetFromONNX(embedderPath.string())
    );
    m_embedderLoaded = !m_net_embedder->empty();

    // throw exception is not the embedder model not found
    if (!m_embedderLoaded)
        throw ObjectDetectorInitializationError("ArcFace model loading failed: Network is empty.");

    // set preferred backend (cpu is usually safer for initial setup)
    m_net_detector->setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    m_net_embedder->setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
}


DetectionList ObjectDetector::runImpl(const Frame& frame)
{
    if (isTerminated())
    {
        throw ObjectDetectorIsTerminatedError(
            "Object detection error: object detector is terminated.");
    }

    return processData(frame.cvMat);
}

} // namespace face_plugin
} // namespace mimos