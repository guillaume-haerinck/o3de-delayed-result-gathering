
#pragma once

#include <DelayedResultGathering/DelayedResultGatheringTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace AZ
{
    class Vector3;
}

namespace DelayedResultGathering
{
    class ExposureMapInterface
    {
    public:
        AZ_RTTI(ExposureMapInterface, "{07215B60-D566-4CD6-B7AA-06C0D9F86DEA}");
        virtual ~ExposureMapInterface() = default;

        virtual bool IsPositionExposed(const AZ::Vector3& position) const = 0;
        virtual bool FindNearestNonExposedPosition(const AZ::Vector3& currentPosition, AZ::Vector3& positionOut) = 0;
    };

} // namespace DelayedResultGathering
