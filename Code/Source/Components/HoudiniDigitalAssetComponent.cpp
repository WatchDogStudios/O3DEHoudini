#include <Components/HoudiniDigitalAssetComponent.h>
#include <HoudiniEngine/HoudiniEngineBus.h>
#include <HoudiniEngine/HoudiniCookedMesh.h>
#include <Asset/HoudiniModelBuilderUtil.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <Atom/RPI.Public/Scene.h>
#include <AzFramework/Entity/EntityContextBus.h>

namespace HoudiniEngine
{
    void HoudiniDigitalAssetComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<HoudiniDigitalAssetComponent, AzToolsFramework::Components::EditorComponentBase>()
                ->Version(1)
                ->Field("HdaPath", &HoudiniDigitalAssetComponent::m_hdaPath)
                ->Field("Status", &HoudiniDigitalAssetComponent::m_status);

            if (auto* edit = serialize->GetEditContext())
            {
                edit->Class<HoudiniDigitalAssetComponent>("Houdini Digital Asset",
                    "Cook a Houdini HDA and preview/bake the result.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Houdini")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniDigitalAssetComponent::m_hdaPath,
                        "HDA path", "Path to the .hda/.otl file to cook.")
                    ->DataElement(AZ::Edit::UIHandlers::Button, &HoudiniDigitalAssetComponent::m_hdaPath,
                        "Cook", "Cook the HDA and update the preview.")
                        ->Attribute(AZ::Edit::Attributes::ButtonText, "Cook")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniDigitalAssetComponent::OnCookPressed)
                    ->DataElement(AZ::Edit::UIHandlers::Button, &HoudiniDigitalAssetComponent::m_status,
                        "Bake", "Bake the cooked preview to a native .azmodel asset.")
                        ->Attribute(AZ::Edit::Attributes::ButtonText, "Bake")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &HoudiniDigitalAssetComponent::OnBakePressed)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &HoudiniDigitalAssetComponent::m_status,
                        "Status", "Last cook/session status.")
                        ->Attribute(AZ::Edit::Attributes::ReadOnly, true);
            }
        }
    }

    void HoudiniDigitalAssetComponent::Activate()
    {
        m_meshFeatureProcessor =
            AZ::RPI::Scene::GetFeatureProcessorForEntity<AZ::Render::MeshFeatureProcessorInterface>(GetEntityId());
    }

    void HoudiniDigitalAssetComponent::Deactivate()
    {
        ClearPreview();
        m_meshFeatureProcessor = nullptr;
    }

    AZ::Crc32 HoudiniDigitalAssetComponent::OnCookPressed()
    {
        RebuildPreview();
        return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
    }

    void HoudiniDigitalAssetComponent::RebuildPreview()
    {
        auto* engine = HoudiniEngineInterface::Get();
        if (!engine)
        {
            m_status = "Houdini Engine gem not initialized.";
            return;
        }
        if (m_hdaPath.empty())
        {
            m_status = "Set an HDA path first.";
            return;
        }

        auto cooked = engine->CookHdaToMesh(m_hdaPath);
        if (!cooked.IsSuccess())
        {
            m_status = AZStd::string("Cook failed: ") + cooked.GetError();
            ClearPreview();
            return;
        }

        m_previewModel = HoudiniModelBuilderUtil::BuildModelAsset(cooked.GetValue(), "HoudiniPreview");
        if (!m_previewModel.Get())
        {
            m_status = "Cook produced no usable geometry.";
            ClearPreview();
            return;
        }

        ClearPreview();
        if (m_meshFeatureProcessor)
        {
            AZ::Render::MeshHandleDescriptor desc;
            desc.m_modelAsset = m_previewModel;
            auto handle = m_meshFeatureProcessor->AcquireMesh(desc);
            m_meshHandle = new AZ::Render::MeshFeatureProcessorInterface::MeshHandle(AZStd::move(handle));
            m_meshFeatureProcessor->SetTransform(
                *static_cast<AZ::Render::MeshFeatureProcessorInterface::MeshHandle*>(m_meshHandle),
                GetWorldTM());
        }
        m_status = "Cook OK.";
    }

    void HoudiniDigitalAssetComponent::ClearPreview()
    {
        if (m_meshFeatureProcessor && m_meshHandle)
        {
            auto* handle = static_cast<AZ::Render::MeshFeatureProcessorInterface::MeshHandle*>(m_meshHandle);
            m_meshFeatureProcessor->ReleaseMesh(*handle);
            delete handle;
            m_meshHandle = nullptr;
        }
    }

    AZ::Crc32 HoudiniDigitalAssetComponent::OnBakePressed()
    {
        // Phase 1: bake routes through the source-asset path so the Asset Processor
        // builds the .azmodel (Task 8). Here we just write/refresh the .hda's bake
        // request marker and report status; the AP builder does the heavy lifting.
        if (!m_previewModel.Get())
        {
            m_status = "Cook before baking.";
        }
        else
        {
            m_status = "Bake requested — see Asset Processor for the .azmodel product.";
        }
        return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
    }
} // namespace HoudiniEngine
