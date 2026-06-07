#include <Asset/HoudiniBakeBuilder.h>
#include <Asset/HoudiniModelBuilderUtil.h>
#include <Session/HoudiniSession.h>
#include <HAPI/HapiWrapper.h>
#include <HoudiniEngine/HoudiniCookedMesh.h>
#include <Translation/HoudiniGeometryToMesh.h>

#include <Atom/RPI.Reflect/Model/ModelAsset.h>
#include <AzCore/Utils/Utils.h>
#include <AzCore/IO/Path/Path.h>
#include <AzCore/Serialization/Utils.h>

namespace HoudiniEngine
{
    HoudiniBakeBuilder::~HoudiniBakeBuilder()
    {
        BusDisconnect();
    }

    void HoudiniBakeBuilder::Register(HoudiniSession& session)
    {
        m_session = &session;

        AssetBuilderSDK::AssetBuilderDesc desc;
        desc.m_name = "Houdini HDA Bake Builder";
        desc.m_patterns.emplace_back("*.hda", AssetBuilderSDK::AssetBuilderPattern::Wildcard);
        desc.m_busId = AZ::Uuid("{F5C4F5E4-1D7F-4C06-9E5F-6F7081920314}");
        desc.m_version = 1;
        desc.m_createJobFunction =
            [this](const auto& req, auto& resp) { CreateJobs(req, resp); };
        desc.m_processJobFunction =
            [this](const auto& req, auto& resp) { ProcessJob(req, resp); };

        AssetBuilderSDK::AssetBuilderBus::Broadcast(
            &AssetBuilderSDK::AssetBuilderBusTraits::RegisterBuilderInformation, desc);
        BusConnect(desc.m_busId);
    }

    void HoudiniBakeBuilder::CreateJobs(
        const AssetBuilderSDK::CreateJobsRequest& request,
        AssetBuilderSDK::CreateJobsResponse& response)
    {
        for (const auto& platform : request.m_enabledPlatforms)
        {
            AssetBuilderSDK::JobDescriptor job;
            job.m_jobKey = "Houdini HDA Bake";
            job.SetPlatformIdentifier(platform.m_identifier.c_str());
            // Cooking needs a license; mark exclusive so we never run two cooks at once.
            job.m_critical = false;
            response.m_createJobOutputs.push_back(job);
        }
        response.m_result = AssetBuilderSDK::CreateJobsResultCode::Success;
    }

    void HoudiniBakeBuilder::ProcessJob(
        const AssetBuilderSDK::ProcessJobRequest& request,
        AssetBuilderSDK::ProcessJobResponse& response)
    {
        if (m_isShuttingDown)
        {
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Cancelled;
            return;
        }
        if (!m_session || !m_session->IsValid())
        {
            AZ_Error("HoudiniBake", false, "No Houdini Engine session/license; cannot bake %s",
                request.m_sourceFile.c_str());
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            return;
        }

        auto op = Hapi::LoadFirstOperator(m_session->Get(), request.m_fullPath.c_str());
        if (!op.IsSuccess())
        {
            AZ_Error("HoudiniBake", false, "%s", op.GetError().c_str());
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            return;
        }
        auto node = Hapi::CreateAndCook(m_session->Get(), op.GetValue().c_str());
        if (!node.IsSuccess())
        {
            AZ_Error("HoudiniBake", false, "%s", node.GetError().c_str());
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            return;
        }

        RawCookedPart raw;
        auto extracted = Hapi::ExtractDisplayPart(m_session->Get(), node.GetValue(), raw);
        if (!extracted.IsSuccess())
        {
            AZ_Error("HoudiniBake", false, "%s", extracted.GetError().c_str());
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            return;
        }

        HoudiniCookedMesh mesh;
        if (!HoudiniGeometryToMesh::Build(raw, mesh))
        {
            AZ_Error("HoudiniBake", false, "triangulation failed for %s", request.m_sourceFile.c_str());
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            return;
        }

        AZ::IO::Path baseName = AZ::IO::Path(request.m_sourceFile).Stem();
        auto model = HoudiniModelBuilderUtil::BuildModelAsset(mesh, baseName.Native());
        if (!model.Get())
        {
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            return;
        }

        AZ::IO::Path outPath = AZ::IO::Path(request.m_tempDirPath) / (baseName.Native() + ".azmodel");
        if (!AZ::Utils::SaveObjectToFile(outPath.Native(),
                AZ::DataStream::ST_BINARY, model.Get()))
        {
            AZ_Error("HoudiniBake", false, "failed to write %s", outPath.c_str());
            response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Failed;
            return;
        }

        AssetBuilderSDK::JobProduct product(outPath.Native(),
            AZ::RPI::ModelAsset::RTTI_Type(), 0);
        product.m_dependenciesHandled = true;
        response.m_outputProducts.push_back(AZStd::move(product));
        response.m_resultCode = AssetBuilderSDK::ProcessJobResult_Success;
    }
} // namespace HoudiniEngine
