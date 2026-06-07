#pragma once
#include <HAPI/HAPI.h>
#include <AzCore/std/string/string.h>
#include <AzCore/Outcome/Outcome.h>
#include <Translation/HoudiniGeometryToMesh.h>

namespace HoudiniEngine
{
    //! Free functions over a HAPI_Session. Each returns an Outcome carrying a
    //! human-readable HAPI status string on failure. No global state — the
    //! session handle is always passed in (owned by HoudiniSession).
    namespace Hapi
    {
        using VoidOutcome = AZ::Outcome<void, AZStd::string>;

        //! Read HAPI's last detailed error string for `session`.
        AZStd::string LastError(const HAPI_Session& session);

        //! Load an HDA file and return the first operator type name inside it.
        AZ::Outcome<AZStd::string, AZStd::string> LoadFirstOperator(
            const HAPI_Session& session, const char* hdaPath);

        //! Instantiate an operator and cook it, blocking until the cook finishes.
        AZ::Outcome<HAPI_NodeId, AZStd::string> CreateAndCook(
            const HAPI_Session& session, const char* operatorName);

        //! Re-cook an existing node, blocking until done.
        VoidOutcome Cook(const HAPI_Session& session, HAPI_NodeId nodeId);

        //! Extract part 0 of the node's display geometry into `out`.
        VoidOutcome ExtractDisplayPart(
            const HAPI_Session& session, HAPI_NodeId nodeId, RawCookedPart& out);
    }
} // namespace HoudiniEngine
