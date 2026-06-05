#include "BuildInfo.h"
#include <stdio.h>
#include <string.h>

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

    void getFullVersion(char *buf, const size_t &size)
    {
        snprintf(buf, size, "%s_%s_%s_%d",
                 BUILD_VERSION, BUILD_PLATFORM, BUILD_TYPE, BUILD_NUMBER);
    }
}
