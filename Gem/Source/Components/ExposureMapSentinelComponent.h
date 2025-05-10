#pragma once

#include <AzCore/Component/Component.h>

namespace DelayedResultGathering
{
    //! Any entity with this component will be treated as a sentinel in the Exposure Map
    //! Any area that is visible from the sentinel field of view will be marked in the Exposure Map as inaccessible
    class ExposureMapSentinelComponent : public AZ::Component
    {
    public:
        AZ_COMPONENT(ExposureMapSentinelComponent, "{711E0087-AFA5-43C2-B44A-588EA1570D31}");

        static void Reflect(AZ::ReflectContext* context);

    private:
        //////////////////////////////////////////////////////////////////////////
        // AZ::Component
        void Activate() override;
        void Deactivate() override;
    };
} // namespace DelayedResultGathering
