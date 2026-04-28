#include "BuildInfo.h"

namespace BuildInfo
{
    const char *getVersion()
    {
        return BUILD_VERSION;
    }

    const char *getPlatform()
    {
        return BUILD_PLATFORM;
    }

    const char *getBuildType()
    {
        return BUILD_TYPE;
    }

    int getBuildNumber()
    {
        return BUILD_NUMBER;
    }

    String getFullVersion()
    {
        return String(BUILD_VERSION) + '_' + BUILD_PLATFORM + '_' + BUILD_TYPE + '_' + String(BUILD_NUMBER);
    }
}