#include "face_recognition_ini.h"

namespace mimos
{
namespace plugin
{

/**
 * providers a single, globally accessible instance of the Ini settings.
 * the `static` keyword ensires it is initialized only once.
 */
Init& ini()
{
    static Ini ini;
    return ini;
}

} // namespace plugin
} // namespace mimos