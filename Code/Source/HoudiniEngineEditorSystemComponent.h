#pragma once
#include <AzCore/Component/Component.h>
#include <HoudiniEngine/HoudiniEngineBus.h>
#include <Session/HoudiniSession.h>
#include <Asset/HoudiniBakeBuilder.h>

namespace HoudiniEngine
{
    //! Gem bootstrap: owns the shared out-of-process session, implements the
    //! HoudiniEngineRequests interface, and registers the bake builder.
    class HoudiniEngineEditorSystemComponent
        : public AZ::Component
        , public HoudiniEngineRequests
    {
    public:
        AZ_COMPONENT(HoudiniEngineEditorSystemComponent, "{B1E0B1A0-7F3D-49C2-9A1E-2B3C4D5E6F70}",
            HoudiniEngineRequests);
        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        // HoudiniEngineRequests
        AZ::Outcome<HoudiniCookedMesh, AZStd::string> CookHdaToMesh(const AZStd::string& hdaPath) override;
        bool IsSessionValid() const override { return m_session.IsValid(); }
        AZ::Outcome<void, AZStd::string> RestartSession() override { return m_session.Restart(); }

    protected:
        void Activate() override;
        void Deactivate() override;

    private:
        HoudiniSession m_session;
        HoudiniBakeBuilder m_bakeBuilder;
    };
} // namespace HoudiniEngine
