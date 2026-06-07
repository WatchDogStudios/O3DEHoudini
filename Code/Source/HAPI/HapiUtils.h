#pragma once
#include <HAPI/HAPI.h>
#include <AzCore/std/string/string.h>
#include <AzCore/Outcome/Outcome.h>

# define AZ_HAPI_ENSURE_SUCCESS(expr) \
    do { \
        HAPI_Result result = (expr); \
        if (result != HAPI_RESULT_SUCCESS) { \
            return AZ::Failure(Hapi::LastError(session)); \
        } \
    } while (0)

namespace HoudiniEngine
{
    namespace Hapi
    {
        static AZStd::vector<AZStd::string> GetLicenseTypes();
    }
} // namespace HoudiniEngine
