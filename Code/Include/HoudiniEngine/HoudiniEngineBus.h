#pragma once
#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/std/string/string.h>
#include <AzCore/Outcome/Outcome.h>

namespace HoudiniEngine
{
    struct HoudiniCookedMesh;

    //! Editor-only interface to the gem's shared Houdini session. Implemented by
    //! HoudiniEngineEditorSystemComponent; consumed by the asset component.
    class HoudiniEngineRequests
    {
    public:
        AZ_RTTI(HoudiniEngineRequests, "{D3A2D3C2-9B5F-4AE4-9C3F-4D5E6F708192}");
        virtual ~HoudiniEngineRequests() = default;

        //! Load `hdaPath`, instantiate its first operator, cook, and return the
        //! triangulated display mesh. Fails (with a message) if the session is
        //! unavailable or the cook errors. Never throws / never crashes the editor.
        virtual AZ::Outcome<HoudiniCookedMesh, AZStd::string> CookHdaToMesh(
            const AZStd::string& hdaPath) = 0;

        virtual bool IsSessionValid() const = 0;
        virtual AZ::Outcome<void, AZStd::string> RestartSession() = 0;
    };

    using HoudiniEngineInterface = AZ::Interface<HoudiniEngineRequests>;
} // namespace HoudiniEngine
