#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/std/containers/vector.h>

namespace DelayedResultGathering
{

    //! If entity is inside an exposed area in the exposure map,
    //! then it will move to the nearest unexposed position
    class MoveToHideout_BehaviorComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(MoveToHideout_BehaviorComponent, "{711E0087-AFA5-43C2-B44A-588EA1570D31}");

        static void Reflect(AZ::ReflectContext* context);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        //! AZ::TickBus::Handler overrides
        int GetTickOrder() final override;
        void OnTick(float deltaTime, AZ::ScriptTimePoint timePoint) final override;

    private:
        //////////////////////////////////////////////////////////////////////////
        // AZ::Component
        void Activate() override;
        void Deactivate() override;

    private:
        AZStd::vector<AZ::Vector3> m_pathToTarget;
    };
} // namespace DelayedResultGathering
