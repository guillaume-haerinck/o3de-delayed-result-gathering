#include "MoveToHideout_BehaviorComponent.h"

#include <AzCore/Component/ComponentApplicationBus.h> // Missing include in Physics/CharacterBus.h
#include <AzCore/Math/Vector3.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/containers/vector.h>
#include <AzFramework/Physics/CharacterBus.h>
#include <RecastNavigation/DetourNavigationBus.h>

#include <assert.h>

namespace DelayedResultGathering
{
    void MoveToHideout_BehaviorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<MoveToHideout_BehaviorComponent, AZ::Component>()->Version(0);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                using namespace AZ::Edit;
                editContext
                    ->Class<MoveToHideout_BehaviorComponent>(
                        "Move to hideout", "Entity will move to the nearest unexposed cell in the exposure map.")
                    ->ClassElement(ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "AI")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"));
            }
        }
    }

    void MoveToHideout_BehaviorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("PhysicsCharacterControllerService"));
        //required.push_back(AZ_CRC_CE("DetourNavigationComponent"));
    }

    int MoveToHideout_BehaviorComponent::GetTickOrder()
    {
        return AZ::ComponentTickBus::TICK_GAME;
    }

    void MoveToHideout_BehaviorComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint timePoint)
    {
        AZ::Vector3 velocity = AZ::Vector3(1.f, 0.f, 0.f);
        Physics::CharacterRequestBus::Event(GetEntityId(), &Physics::CharacterRequests::AddVelocityForTick, velocity);
        // #GH_TODO query the exposure map, and move to position if invalid

        AZ::Vector3 from(5.f, 4.f, 0.f);
        AZ::Vector3 to(5.f, 12.f, 0.f);

        // #GH_TODO always empty
        AZStd::vector<AZ::Vector3> waypoints;
        RecastNavigation::DetourNavigationRequestBus::EventResult(
            waypoints, GetEntityId(), &RecastNavigation::DetourNavigationRequestBus::Events::FindPathBetweenPositions, from, to);

        if (waypoints.empty())
        {
            AZ_Printf("Test", "Hey");
        }
    }

    void MoveToHideout_BehaviorComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
    }

    void MoveToHideout_BehaviorComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }

} // namespace DelayedResultGathering
