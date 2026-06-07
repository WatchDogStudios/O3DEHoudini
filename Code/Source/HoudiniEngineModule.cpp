#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>
#include "HoudiniEngineEditorSystemComponent.h"
#include "Components/HoudiniDigitalAssetComponent.h"

namespace HoudiniEngine
{
    class HoudiniEngineModule : public AZ::Module
    {
    public:
        AZ_RTTI(HoudiniEngineModule, "{C2F1C2B1-8A4E-4FD3-8B2F-3C4D5E6F7081}", AZ::Module);
        AZ_CLASS_ALLOCATOR(HoudiniEngineModule, AZ::SystemAllocator);

        HoudiniEngineModule()
        {
            m_descriptors.insert(m_descriptors.end(), {
                HoudiniEngineEditorSystemComponent::CreateDescriptor(),
                HoudiniDigitalAssetComponent::CreateDescriptor(),
            });
        }

        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<HoudiniEngineEditorSystemComponent>(),
            };
        }
    };
} // namespace HoudiniEngine

AZ_DECLARE_MODULE_CLASS(Gem_HoudiniEngine, HoudiniEngine::HoudiniEngineModule)
