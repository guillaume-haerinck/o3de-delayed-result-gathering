
#pragma once

#include <DelayedResultGathering/DelayedResultGatheringTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace DelayedResultGathering
{
    class DelayedResultGatheringRequests
    {
    public:
        AZ_RTTI(DelayedResultGatheringRequests, DelayedResultGatheringRequestsTypeId);
        virtual ~DelayedResultGatheringRequests() = default;
        // Put your public methods here
    };

    class DelayedResultGatheringBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using DelayedResultGatheringRequestBus = AZ::EBus<DelayedResultGatheringRequests, DelayedResultGatheringBusTraits>;
    using DelayedResultGatheringInterface = AZ::Interface<DelayedResultGatheringRequests>;

} // namespace DelayedResultGathering
