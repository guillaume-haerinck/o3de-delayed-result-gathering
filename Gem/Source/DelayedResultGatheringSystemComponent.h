
#pragma once

#include <AzCore/Component/Component.h>

#include <DelayedResultGathering/DelayedResultGatheringBus.h>

namespace DelayedResultGathering
{
    //! This Gem global system component
    class DelayedResultGatheringSystemComponent
        : public AZ::Component
        , protected DelayedResultGatheringRequestBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(DelayedResultGatheringSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        DelayedResultGatheringSystemComponent();
        ~DelayedResultGatheringSystemComponent();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // DelayedResultGatheringRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
