#include "Components/MoveToHideout_BehaviorComponent.h"
#include "DelayedResultGathering/DelayedResultGatheringTypeIds.h"
#include "ExposureMapLevelComponent.h"

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

namespace DelayedResultGathering
{
    class DelayedResultGatheringModule : public AZ::Module
    {
    public:
        AZ_RTTI(DelayedResultGatheringModule, DelayedResultGatheringModuleTypeId, AZ::Module);
        AZ_CLASS_ALLOCATOR(DelayedResultGatheringModule, AZ::SystemAllocator);

        DelayedResultGatheringModule()
            : AZ::Module()
        {
            m_descriptors.insert(
                m_descriptors.end(),
                { ExposureMapLevelComponent::CreateDescriptor(), MoveToHideout_BehaviorComponent::CreateDescriptor() });
        }
    };
} // namespace DelayedResultGathering

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), DelayedResultGathering::DelayedResultGatheringModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_DelayedResultGathering, DelayedResultGathering::DelayedResultGatheringModule)
#endif
