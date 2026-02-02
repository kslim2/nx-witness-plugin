#pragma once

#include <nx/sdk/analytics/helpers/plugin.h>
#include <nx/sdk/analytics/i_engine.h>

namespace mimos {
namespace plugin {
namespace face_recognition

class Plugin: public nx::sdk::analytics::Plugin
{

protected:
    virtual nx::sdk::Result<nx::sdk::analytics::IEngine*> doObtainEngine() override;
    virtual std::string manifestString() const override;

};

} // namespace face_recognition
} // namespace plugin
} // namespace mimos