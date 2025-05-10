
#include <AzCore/Serialization/SerializeContext.h>

#include "DelayedResultGatheringSystemComponent.h"

#include <DelayedResultGathering/DelayedResultGatheringTypeIds.h>

namespace DelayedResultGathering
{
    AZ_COMPONENT_IMPL(DelayedResultGatheringSystemComponent, "DelayedResultGatheringSystemComponent",
        DelayedResultGatheringSystemComponentTypeId);

    void DelayedResultGatheringSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<DelayedResultGatheringSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void DelayedResultGatheringSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("DelayedResultGatheringService"));
    }

    void DelayedResultGatheringSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("DelayedResultGatheringService"));
    }

    void DelayedResultGatheringSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void DelayedResultGatheringSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    DelayedResultGatheringSystemComponent::DelayedResultGatheringSystemComponent()
    {
        if (DelayedResultGatheringInterface::Get() == nullptr)
        {
            DelayedResultGatheringInterface::Register(this);
        }
    }

    DelayedResultGatheringSystemComponent::~DelayedResultGatheringSystemComponent()
    {
        if (DelayedResultGatheringInterface::Get() == this)
        {
            DelayedResultGatheringInterface::Unregister(this);
        }
    }

    void DelayedResultGatheringSystemComponent::Init()
    {
    }

    void DelayedResultGatheringSystemComponent::Activate()
    {
        DelayedResultGatheringRequestBus::Handler::BusConnect();
    }

    void DelayedResultGatheringSystemComponent::Deactivate()
    {
        DelayedResultGatheringRequestBus::Handler::BusDisconnect();
    }
}
