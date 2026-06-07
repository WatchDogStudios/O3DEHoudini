#pragma once
#include <AssetBuilderSDK/AssetBuilderSDK.h>
#include <AssetBuilderSDK/AssetBuilderBusses.h>

namespace HoudiniEngine
{
    class HoudiniSession;

    //! AssetBuilderSDK builder: *.hda -> *.azmodel. Cooks the first operator and
    //! serializes the resulting model. Registered by the editor system component.
    class HoudiniBakeBuilder
        : public AssetBuilderSDK::AssetBuilderCommandBus::Handler
    {
    public:
        ~HoudiniBakeBuilder() override;

        //! `session` is owned by the system component and must outlive this builder.
        void Register(HoudiniSession& session);

        void CreateJobs(const AssetBuilderSDK::CreateJobsRequest& request,
            AssetBuilderSDK::CreateJobsResponse& response);
        void ProcessJob(const AssetBuilderSDK::ProcessJobRequest& request,
            AssetBuilderSDK::ProcessJobResponse& response);

        // AssetBuilderCommandBus
        void ShutDown() override { m_isShuttingDown = true; }

    private:
        HoudiniSession* m_session = nullptr;
        AZStd::atomic_bool m_isShuttingDown{ false };
    };
} // namespace HoudiniEngine
