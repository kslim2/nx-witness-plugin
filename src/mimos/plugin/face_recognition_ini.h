#pragma once

#include <nx/kit/ini_config.h>
#include <nx/sdk/analytics/helpers/pixel_format.h>

namespace mimos
{
namespace plugin
{

struct Ini:: public nx::kit::IniConfig
{
    Ini(): IniConfig("face_recognition.ini") { reload(); }

    NX_INIT_FLAG(0, enabledOutput, "")
    NX_INIT_FLAG(0, isLicenseRequired, "Whether the plugin declares in its manifest that requires license.")
};

Ini& ini();

} // namespace plugin
} // namespace mimos