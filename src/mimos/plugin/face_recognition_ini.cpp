#include "face_recognition_ini.h"

namespace mimos
{
namespace plugin
{
namespace face_recognition
{

Ini& ini()
{
    static Ini iniInstance;
    return iniInstance;
}

} // namespace face_recognition
} // namespace plugin
} // namespace mimos