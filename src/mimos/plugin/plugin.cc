#include "plugin.h"
#include "engine.h"

namespace mimos
{
namespace plugin
{

using namespace nx::sdk;
using namespace nx::sdk::analytics;

Result<IEngine*> Plugin::doObtainEngine()
{
    const auto utilityProvider = this->utilityProvider();
    const std::filesystem::path pluginHomeDir = utilityProvider->homeDir();
    return new Engine(pluginHomeDir);
}

/**
* JSON with the particular structure. Note that it is possible to fill the value that are known
* at the compile time
* - id: unique identifier
* - name: a human readable short name of plugin (display in the camera settings)
* - description: description of the plugin
* - version: version of the plugin
* - vendor: plugin vendor
*/
std::string Plugin::manifestString() const
{
    return /*suppress newline*/ 1 + R"json(
    {
        "id": "mimos.plugin",
        "name": "OpenCV Face Recognition",
        "description": ")json"
            "This plugin is for face classification. It's based on OpenCV."
            R"json(",
        "version": "1.0.0",
        "vendor": "mimos"
    }
    )json";
}

/**
 * Called by the server to instantiate the plugin object
 * 
 * The server requires the function to have C linkage, which
 * leads to no c++ name mangling in the export table of plugin
 * dynamic library, so that makes it possible to write plugins
 * in any language and compiler.
 * 
 * NX_PLUGIN_API is the macro defined by CMake scripts for
 * exporting the function
 */
extern "C" NX_PLUGIN_API nx::sdk::IPlugin* createNxPlugin()
{
    // the object will be freed when the server calls releaseRef()
    return new Plugin();
}

} // namespace plugin
} // namespace mimos