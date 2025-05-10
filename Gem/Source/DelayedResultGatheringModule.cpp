
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

#include "DelayedResultGatheringSystemComponent.h"

#include <DelayedResultGathering/DelayedResultGatheringTypeIds.h>

namespace DelayedResultGathering
{
    class DelayedResultGatheringModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(DelayedResultGatheringModule, DelayedResultGatheringModuleTypeId, AZ::Module);
        AZ_CLASS_ALLOCATOR(DelayedResultGatheringModule, AZ::SystemAllocator);

        DelayedResultGatheringModule()
            : AZ::Module()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                DelayedResultGatheringSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<DelayedResultGatheringSystemComponent>(),
            };
        }
    };
}// namespace DelayedResultGathering

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), DelayedResultGathering::DelayedResultGatheringModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_DelayedResultGathering, DelayedResultGathering::DelayedResultGatheringModule)
#endif
