#include <Session/HoudiniSession.h>
#include <HAPI/HapiWrapper.h>

namespace HoudiniEngine
{
    HoudiniSession::~HoudiniSession()
    {
        Stop();
    }

    AZ::Outcome<void, AZStd::string> HoudiniSession::Start()
    {
        if (m_initialized) { return AZ::Success(); }

        // Unique per-editor-process pipe name avoids collisions with other tools.
        m_pipeName = AZStd::string::format("o3de_houdini_%p", static_cast<void*>(this));

        HAPI_ThriftServerOptions serverOptions = HAPI_ThriftServerOptions_Create();
        serverOptions.autoClose = true;
        serverOptions.timeoutMs = 10000.0f;

        if (HAPI_StartThriftNamedPipeServer(&serverOptions, m_pipeName.c_str(), &m_serverPid, nullptr)
            != HAPI_RESULT_SUCCESS)
        {
            return AZ::Failure(AZStd::string(
                "failed to start Houdini Engine server (is a Houdini Engine license available?)"));
        }

        // H20.0 (Engine API 6.0): HAPI_CreateThriftNamedPipeSession takes only
        // (session, pipe_name) — no HAPI_SessionInfo (that arrived in H20.5).
        if (HAPI_CreateThriftNamedPipeSession(&m_session, m_pipeName.c_str())
            != HAPI_RESULT_SUCCESS)
        {
            return AZ::Failure(AZStd::string("failed to connect to Houdini Engine server"));
        }

        HAPI_CookOptions cookOptions = HAPI_CookOptions_Create();
        if (HAPI_Initialize(&m_session, &cookOptions, /*use_cooking_thread*/ true,
                /*stack_size*/ -1, nullptr, nullptr, nullptr, nullptr, nullptr)
            != HAPI_RESULT_SUCCESS)
        {
            return AZ::Failure(Hapi::LastError(m_session));
        }

        m_initialized = true;
        return AZ::Success();
    }

    void HoudiniSession::Stop()
    {
        if (m_initialized)
        {
            HAPI_Cleanup(&m_session);
            HAPI_CloseSession(&m_session);
            m_initialized = false;
        }
        m_serverPid = static_cast<HAPI_ProcessId>(-1);
    }

    AZ::Outcome<void, AZStd::string> HoudiniSession::Restart()
    {
        Stop();
        return Start();
    }
} // namespace HoudiniEngine
