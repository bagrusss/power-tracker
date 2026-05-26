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
        char ver[16], plat[16], type[16];
        strcpy_P(ver, PSTR(BUILD_VERSION));
        strcpy_P(plat, PSTR(BUILD_PLATFORM));
        strcpy_P(type, PSTR(BUILD_TYPE));
        snprintf(buf, size, "%s_%s_%s_%d", ver, plat, type, BUILD_NUMBER);
    }

}
