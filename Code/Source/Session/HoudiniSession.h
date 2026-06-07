#pragma once
#include <HAPI/HAPI.h>
#include <AzCore/std/string/string.h>
#include <AzCore/Outcome/Outcome.h>

namespace HoudiniEngine
{
    //! Owns one out-of-process Houdini Engine session: it starts a named-pipe
    //! thrift server, connects, and initializes HAPI. A bad-HDA cook crash takes
    //! down the server process, not the editor; Restart() recovers.
    class HoudiniSession
    {
    public:
        HoudiniSession() = default;
        ~HoudiniSession();

        HoudiniSession(const HoudiniSession&) = delete;
        HoudiniSession& operator=(const HoudiniSession&) = delete;

        //! Start the server, connect, and HAPI_Initialize. Checks out a license.
        AZ::Outcome<void, AZStd::string> Start();
        void Stop();
        AZ::Outcome<void, AZStd::string> Restart();

        bool IsValid() const { return m_initialized; }
        const HAPI_Session& Get() const { return m_session; }
    private:
        //! Checks for the presence of a Houdini Engine license, but fails if the local computer doesn't contain a interactive (or exportable) license. This is used to avoid starting the server if we know it won't be able to cook.
        bool CheckForPreferredHoudiniLicense() const;
        
    private:
        HAPI_Session m_session{};
        HAPI_ProcessId m_serverPid = static_cast<HAPI_ProcessId>(-1);
        bool m_initialized = false;
        AZStd::string m_pipeName;
    };
} // namespace HoudiniEngine
