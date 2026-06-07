#pragma once
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzCore/Asset/AssetCommon.h>

namespace AZ::RPI { class ModelAsset; }
namespace AZ::Render { class MeshFeatureProcessorInterface; }

namespace HoudiniEngine
{
    //! Editor component: cooks a referenced HDA and previews the result on this
    //! entity via the MeshFeatureProcessor. Provides a "Cook" and "Bake" button.
    class HoudiniDigitalAssetComponent
        : public AzToolsFramework::Components::EditorComponentBase
    {
    public:
        AZ_EDITOR_COMPONENT(HoudiniDigitalAssetComponent, "{E4B3E4D3-0C6F-4BF5-8D4F-5E6F70819203}");
        static void Reflect(AZ::ReflectContext* context);

        void Activate() override;
        void Deactivate() override;

    private:
        AZ::Crc32 OnCookPressed();
        AZ::Crc32 OnBakePressed();
        void RebuildPreview();
        void ClearPreview();

        AZStd::string m_hdaPath;            // path to the .hda
        AZStd::string m_status;             // last cook/session status, shown read-only
        AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor = nullptr;
        void* m_meshHandle = nullptr;       // MeshFeatureProcessor handle (opaque here)
        AZ::Data::Asset<AZ::RPI::ModelAsset> m_previewModel;
    };
} // namespace HoudiniEngine
