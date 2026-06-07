#include "HoudiniEngineEditorSystemComponent.h"
#include <HAPI/HapiWrapper.h>
#include <Translation/HoudiniGeometryToMesh.h>
#include <HoudiniEngine/HoudiniCookedMesh.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace HoudiniEngine
{
    void HoudiniEngineEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<HoudiniEngineEditorSystemComponent, AZ::Component>()->Version(1);
        }
    }

    void HoudiniEngineEditorSystemComponent::GetProvidedServices(
        AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("HoudiniEngineEditorService"));
    }

    void HoudiniEngineEditorSystemComponent::GetIncompatibleServices(
        AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("HoudiniEngineEditorService"));
    }

    void HoudiniEngineEditorSystemComponent::Activate()
    {
        HoudiniEngineInterface::Register(this);

        auto started = m_session.Start();
        if (!started.IsSuccess())
        {
            // Non-fatal: the editor runs fine; cooking is unavailable until a
            // license/install is present and the session is restarted.
            AZ_Warning("HoudiniEngine", false,
                "Houdini Engine session not started: %s", started.GetError().c_str());
        }
        else
        {
            m_bakeBuilder.Register(m_session);
        }
    }

    void HoudiniEngineEditorSystemComponent::Deactivate()
    {
        m_session.Stop();
        HoudiniEngineInterface::Unregister(this);
    }

    AZ::Outcome<HoudiniCookedMesh, AZStd::string>
    HoudiniEngineEditorSystemComponent::CookHdaToMesh(const AZStd::string& hdaPath)
    {
        if (!m_session.IsValid())
        {
            return AZ::Failure(AZStd::string("no Houdini Engine session (check license/install)"));
        }
        auto op = Hapi::LoadFirstOperator(m_session.Get(), hdaPath.c_str());
        if (!op.IsSuccess()) { return AZ::Failure(op.TakeError()); }

        auto node = Hapi::CreateAndCook(m_session.Get(), op.GetValue().c_str());
        if (!node.IsSuccess()) { return AZ::Failure(node.TakeError()); }

        RawCookedPart raw;
        auto extracted = Hapi::ExtractDisplayPart(m_session.Get(), node.GetValue(), raw);
        if (!extracted.IsSuccess()) { return AZ::Failure(extracted.TakeError()); }

        HoudiniCookedMesh mesh;
        if (!HoudiniGeometryToMesh::Build(raw, mesh))
        {
            return AZ::Failure(AZStd::string("triangulation produced no geometry"));
        }
        return AZ::Success(AZStd::move(mesh));
    }
} // namespace HoudiniEngine
