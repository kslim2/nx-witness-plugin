#pragma once

#include <nx/kit/ini_config.h>

namespace mimos
{
namespace plugin
{
namespace face_recognition
{

/**
 * Central configuration for the Face Recognition plugin.
 * Values are read from face_recognition.ini (if present) or fall back to these defaults.
 */
struct Ini : public nx::kit::IniConfig
{
    /**
     * Constructor loads the configuration file automatically.
     * The file face_recognition.ini is searched in the standard plugin configuration locations.
     */
    Ini()
        : IniConfig("face_recognition.ini")
    {
        reload();
    }

    // ------------------------------------------------------------------------
    //  General plugin behavior
    // ------------------------------------------------------------------------

    /**
     * Indicates whether this plugin requires a license.
     * This value is referenced in the plugin manifest.
     * @default false
     */
    NX_INI_FLAG(false, isLicenseRequired,
        "Whether the plugin declares in its manifest that it requires a license.");

    /**
     * Whether the plugin should be enabled by default when added to a camera.
     * @default true
     */
    NX_INI_FLAG(true, enabledByDefault,
        "Whether the plugin is enabled by default when added to a camera");

    // ------------------------------------------------------------------------
    //  Paths to embedding databases
    // ------------------------------------------------------------------------

    /**
     * Directory containing .npy files with embeddings of **whitelisted** persons.
     * Each file should contain one 512-float vector (ArcFace embedding).
     * The filename (without .npy extension) is used as the person's name.
     *
     * Examples:
     *   - relative path: "db/whitelist"
     *   - absolute path: "/opt/nx/db/whitelist" or "/home/nx/face/db/whitelist"
     *
     * @default "db/whitelist"
     */
    NX_INI_STRING("db/whitelist", whitelistDirectory,
        "Directory with .npy files of whitelisted persons (filename = name without .npy)");

    /**
     * Directory containing .npy files with embeddings of **blacklisted** persons.
     * Same format as whitelistDirectory.
     *
     * @default "db/blacklist"
     */
    NX_INI_STRING("db/blacklist", blacklistDirectory,
        "Directory with .npy files of blacklisted persons (filename = name without .npy)");

    // ------------------------------------------------------------------------
    //  Matching and detection parameters
    // ------------------------------------------------------------------------

    /**
     * Minimum cosine similarity required to accept a match between a detected face
     * and a known embedding (whitelist or blacklist).
     * Typical range for ArcFace: 0.40 – 0.60
     * @default 0.45
     */
    NX_INI_FLOAT(0.45f, matchThreshold,
        "Minimum cosine similarity for a match (typical range: 0.40–0.60)");

    /**
     * Minimum confidence score from RetinaFace to accept a detected face.
     * Lower values = more detections, but more false positives.
     * @default 0.60
     */
    NX_INI_FLOAT(0.60f, minDetectionConfidence,
        "Minimum confidence from RetinaFace to consider a face detection valid");

    // ------------------------------------------------------------------------
    //  Model and preprocessing parameters
    // ------------------------------------------------------------------------

    /**
     * Expected input size for ArcFace (width = height).
     * Standard ArcFace models use 112×112.
     * @default 112
     */
    NX_INI_INT(112, arcfaceInputSize,
        "Input image size for ArcFace model (usually 112)");

    /**
     * Scale factor to enlarge the detected face bounding box before cropping.
     * Value > 1.0 includes more context around the face (helps with alignment).
     * @default 1.2
     */
    NX_INI_FLOAT(1.2f, faceCropScaleFactor,
        "Enlargement factor for face crop (>1.0 = more context around face)");

    // ------------------------------------------------------------------------
    //  Event generation control
    // ------------------------------------------------------------------------

    /**
     * Generate analytics events when a blacklisted person is detected.
     * Usually should be enabled in security use cases.
     * @default true
     */
    NX_INI_FLAG(true, enableBlacklistAlerts,
        "Generate events when a blacklisted person is detected");

    /**
     * Generate events also for whitelisted persons.
     * Can be useful for access logging, but often turned off to reduce event noise.
     * @default false
     */
    NX_INI_FLAG(false, enableWhitelistEvents,
        "Generate events also for whitelisted persons (can be noisy)");

    // ------------------------------------------------------------------------
    //  Performance tuning
    // ------------------------------------------------------------------------

    /**
     * Controls how often face recognition is performed.
     * 0 = every frame
     * 2 = every 2nd frame
     * 5 = every 5th frame, etc.
     * Higher values reduce CPU/GPU usage but may miss short events.
     * @default 0 (every frame)
     */
    NX_INI_INT(0, faceProcessingIntervalFrames,
        "Process every Nth frame for recognition (0 = every frame)");

    // ------------------------------------------------------------------------
    //  Debugging & logging
    // ------------------------------------------------------------------------

    /**
     * Log match details (name, similarity score) to the server log when a match occurs.
     * Very helpful during development and troubleshooting.
     * @default true
     */
    NX_INI_FLAG(true, logMatchesToServer,
        "Log match information (name, similarity) to server log");
};

/**
 * Global accessor to the singleton configuration instance.
 * Usage example:
 *    ini().matchThreshold
 *    ini().whitelistDirectory
 */
Ini& ini();

} // namespace face_recognition
} // namespace plugin
} // namespace mimos