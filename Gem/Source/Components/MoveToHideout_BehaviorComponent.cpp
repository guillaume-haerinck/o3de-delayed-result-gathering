#include "MoveToHideout_BehaviorComponent.h"

#include "DelayedResultGathering/ExposureMapInterface.h"

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
            serializeContext->Class<MoveToHideout_BehaviorComponent, AZ::Component>()->Version(0)->Field(
                "moveSpeed", &MoveToHideout_BehaviorComponent::m_moveSpeed);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                using namespace AZ::Edit;
                editContext
                    ->Class<MoveToHideout_BehaviorComponent>(
                        "Move to hideout", "Entity will move to the nearest unexposed cell in the exposure map.")
                    ->ClassElement(ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "AI")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"))
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default,
                        &MoveToHideout_BehaviorComponent::m_moveSpeed,
                        "Move Speed",
                        "How fast actor should move when going into hideout");
            }
        }
    }

    void MoveToHideout_BehaviorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("PhysicsCharacterControllerService"));
        // required.push_back(AZ_CRC_CE("DetourNavigationComponent"));
    }

    int MoveToHideout_BehaviorComponent::GetTickOrder()
    {
        return AZ::ComponentTickBus::TICK_GAME;
    }

    void MoveToHideout_BehaviorComponent::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint timePoint)
    {
        AZ::Vector3 currentPosition = AZ::Vector3(1.f, 0.f, 0.f);
        Physics::CharacterRequestBus::EventResult(currentPosition, GetEntityId(), &Physics::CharacterRequests::GetBasePosition);

        const bool isMoving = !m_pathToTarget.empty();
        if (isMoving)
        {
            // Proceed to the current target position
            const AZ::Vector3& distance = m_pathToTarget.back() - currentPosition;
            constexpr float tolerance = 0.4f;
            const bool hasReachedDestination = distance.GetLength() < tolerance;
            if (hasReachedDestination)
            {
                m_pathToTarget.pop_back();
            }
            else
            {
                const AZ::Vector3 direction = distance.GetNormalized();
                const AZ::Vector3 velocity = direction * deltaTime * m_moveSpeed;
                Physics::CharacterRequestBus::Event(GetEntityId(), &Physics::CharacterRequests::AddVelocityForTick, velocity);
            }

            return;
        }

        const ExposureMapInterface* exposureMap = AZ::Interface<ExposureMapInterface>::Get();
        if (!exposureMap)
        {
            assert(false);
            return;
        }

        if (!exposureMap->IsPositionExposed(currentPosition))
        {
            // No need to move as we are already hidden
            return;
        }

        AZ::Vector3 newPosition;
        const bool success = exposureMap->FindNearestNonExposedPosition(currentPosition, newPosition);
        if (!success)
        {
            assert(false);
            return;
        }

        RecastNavigation::DetourNavigationRequestBus::EventResult(
            m_pathToTarget,
            GetEntityId(),
            &RecastNavigation::DetourNavigationRequestBus::Events::FindPathBetweenPositions,
            newPosition,
            currentPosition);
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
