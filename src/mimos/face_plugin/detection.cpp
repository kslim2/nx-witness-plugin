// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "detection.h"

namespace mimos {
namespace face_plugin {

// Class labels for the MobileNet SSD model (VOC dataset).
// const std::vector<std::string> kClasses{
//     "face"
// };
// const std::vector<std::string> kClassesToDetect{"face"};
// const std::map<std::string, std::string> kClassesToDetectPluralCapitalized{{"face", "Face"}};

/**
 * The single class label relevant for face detection
 */
const std::string kFaceClassLabel = "face";

/**
 * capitalized version, useful for ui display within Network Optix VMS
 */
const std::string kFaceClassLabelCapitalized = "Face";

} // namespace face_plugin
} // namespace mimos
