#include "ExposureMapSystemComponent.h"

#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Console/IConsole.h>
#include <AzCore/EBus/Results.h>
#include <AzCore/Math/Aabb.h>
#include <AzCore/Math/Color.h>
#include <AzCore/Math/Crc.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Math/Vector2.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/limits.h>
#include <AzFramework/Physics/CharacterBus.h>
#include <AzFramework/Physics/Common/PhysicsSceneQueries.h>
#include <AzFramework/Physics/Common/PhysicsTypes.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <DebugDraw/DebugDrawBus.h>
#include <LmbrCentral/Scripting/TagComponentBus.h>
#include <RecastNavigation/DetourNavigationBus.h>
#include <assert.h>

AZ_CVAR(
    int,
    exposuremap_show,
    1,
    nullptr,
    AZ::ConsoleFunctorFlags::Null,
    "0: Hide exposure map debug draw, 1: Show exposure map debug draw");

namespace DelayedResultGathering
{

    void ExposureMapSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ExposureMapSystemComponent, AZ::Component>()->Version(0);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                using namespace AZ::Edit;
                editContext
                    ->Class<ExposureMapSystemComponent>("Exposure Map System Component", "System in charge of building the exposure map.")
                    ->ClassElement(ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "AI");
            }
        }
    }

    void ExposureMapSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("ExposureMapComponentService"));
    }

    void ExposureMapSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("ExposureMapComponentService"));
    }

    int ExposureMapSystemComponent::GetTickOrder()
    {
        return AZ::ComponentTickBus::TICK_GAME;
    }

    void ExposureMapSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint timePoint)
    {
        // #GH_TODO find an event to know when we are in game, we only want to rebuild the grid once we are in game
        // if (m_gridNeedRebuild)
        {
            m_gridNeedRebuild = false;
            BuildGrid();
        }

        AZ::EBusAggregateResults<AZ::EntityId> aggregator;
        LmbrCentral::TagGlobalRequestBus::EventResult(
            aggregator, AZ::Crc32("Player"), &LmbrCentral::TagGlobalRequests::RequestTaggedEntities);

        if (aggregator.values.empty())
        {
            assert(false && "No entity with a tag component with Player tag on the scene");
            return;
        }

        AZ::Vector3 playerPosition;
        const AZ::EntityId& playerId = aggregator.values[0];
        Physics::CharacterRequestBus::EventResult(playerPosition, playerId, &Physics::CharacterRequests::GetBasePosition);

        // Position is at the feet and the player is around 2 meter tall
        AZ::Vector3 eyePosition = playerPosition;
        eyePosition.SetZ(eyePosition.GetZ() + 1.9f);

        // #GH_TODO switch case for all the types
        UpdateExposure_SingleThreaded(eyePosition);

        if (exposuremap_show > 0)
            DebugDrawExposureMap();
    }

    bool ExposureMapSystemComponent::IsPositionExposed(const AZ::Vector3& position) const
    {
        const uint16_t cellIndex = PositionToCellIndex(AZ::Vector2(position.GetX(), position.GetY()));
        if (cellIndex > m_grid.size())
        {
            assert(false);
            return false;
        }

        return m_isPositionExposedMap[cellIndex];
    }

    bool ExposureMapSystemComponent::FindNearestNonExposedPosition(
        [[maybe_unused]] const AZ::Vector3& currentPosition, AZ::Vector3& positionOut)
    {
        UpdateDistanceToEnemyMap(currentPosition);

        // This search is sub-optimal and will badly scale as grid size increase, using an octree would be way more efficient
        float bestPathLength = AZStd::numeric_limits<float>::max();
        bool found = false;
        uint16_t bestCellIndex = 0;

        const uint16_t cellCount = ComputeCellCount();
        for (uint16_t cellIndex = 0; cellIndex < cellCount; ++cellIndex)
        {
            if (m_isPositionExposedMap[cellIndex])
                continue;

            if (m_isPositionObstructedMap[cellIndex])
                continue;

            const float pathLength = m_distanceToEnemyMap[cellIndex];
            constexpr float minDistance = 10.f; // We want the enemy to move away, not turn around a cube
            if (pathLength < minDistance || pathLength >= bestPathLength)
                continue;

            bestPathLength = pathLength;
            bestCellIndex = cellIndex;
            found = true;
        }

        if (!found)
        {
            positionOut = currentPosition;
            return false;
        }

        positionOut = m_grid[bestCellIndex];
        return true;
    }

    void ExposureMapSystemComponent::DebugDrawExposureMap()
    {
        const uint16_t cellCount = ComputeCellCount();
        for (uint16_t cellIndex = 0; cellIndex < cellCount; ++cellIndex)
        {
            if (m_isPositionObstructedMap[cellIndex])
            {
                // Cell is obstructed by an obstacle, no need to draw
                continue;
            }

            const AZ::Vector3& center = m_grid[cellIndex];

            AZ::Aabb aabb = AZ::Aabb::CreateCenterHalfExtents(center, AZ::Vector3(m_cellSize / 2.2f, m_cellSize / 2.2f, 0.01f));
            const bool isExposed = m_isPositionExposedMap[cellIndex];
            const AZ::Color color = isExposed ? AZ::Color::CreateFromRgba(255, 0, 0, 100) : AZ::Color::CreateFromRgba(0, 255, 0, 100);
            constexpr float duration = 0.f; // One frame
            DebugDraw::DebugDrawRequestBus::Broadcast(&DebugDraw::DebugDrawRequests::DrawAabb, aabb, color, duration);
        }
    }

    void ExposureMapSystemComponent::UpdateDistanceToEnemyMap(const AZ::Vector3& enemyPosition)
    {
        const uint16_t cellCount = ComputeCellCount();
        for (uint16_t cellIndex = 0; cellIndex < cellCount; ++cellIndex)
        {
            if (m_isPositionObstructedMap[cellIndex])
                continue;

            const AZ::Vector3& cellPosition = m_grid[cellIndex];

            // #GH_TODO it always fails, maybe that the caller needs to have a detour navigation component
            AZStd::vector<AZ::Vector3> path;
            RecastNavigation::DetourNavigationRequestBus::EventResult(
                path,
                GetEntityId(),
                &RecastNavigation::DetourNavigationRequestBus::Events::FindPathBetweenPositions,
                enemyPosition,
                cellPosition);

            float totalDistance = 0.f;
            if (path.empty())
                totalDistance = (cellPosition - enemyPosition).GetLength(); // Fallback if pathfinding failed
            else
            {
                for (size_t i = 0; i < path.size(); ++i)
                {
                    const bool isLast = i == path.size() - 1;
                    if (isLast)
                    {
                        continue;
                    }
                    totalDistance += (path[i] - path[i + 1]).GetLength();
                }
            }

            m_distanceToEnemyMap[cellIndex] = totalDistance;
        }
    }

    uint16_t ExposureMapSystemComponent::ComputeCellCount() const
    {
        return m_gridDimension * m_gridDimension;
    }

    uint16_t ExposureMapSystemComponent::ComputeCellIndex(uint8_t row, uint8_t column) const
    {
        return row * m_gridDimension + column;
    }

    AZ::Vector3 ExposureMapSystemComponent::ComputeCellCenter(uint8_t row, uint8_t column) const
    {
        return AZ::Vector3(static_cast<float>((column + 0.5) * m_cellSize), static_cast<float>((row + 0.5) * m_cellSize), 0.f);
    }

    uint16_t ExposureMapSystemComponent::PositionToCellIndex(const AZ::Vector2& position) const
    {
        const uint8_t row = static_cast<uint8_t>(floor(position.GetY() / m_cellSize));
        const uint8_t column = static_cast<uint8_t>(floor(position.GetX() / m_cellSize));
        return ComputeCellIndex(row, column);
    }

    bool ExposureMapSystemComponent::IsCellToPositionObstructed(
        const AZ::Vector3& position,
        uint16_t cellIndex,
        AzPhysics::SceneInterface& sceneInterface,
        const AzPhysics::SceneHandle& sceneHandle) const
    {
        if (m_isPositionObstructedMap[cellIndex])
            return true;

        const AZ::Vector3& cellCenter = m_grid[cellIndex];
        const AZ::Vector3 cellToPosition = cellCenter - position;
        const AZ::Vector3 unitDir = cellToPosition.GetNormalizedSafe();
        const float length = cellToPosition.GetLength() - 0.2f; // So that we stay above ground

        AzPhysics::RayCastRequest request;
        request.m_start = position;
        request.m_direction = unitDir;
        request.m_distance = length;
        request.m_queryType = AzPhysics::SceneQuery::QueryType::Static;

        const AzPhysics::SceneQueryHits result = sceneInterface.QueryScene(sceneHandle, &request);
        const bool hasHit = result && result.m_hits[0].IsValid();
        return hasHit;
    }

    void ExposureMapSystemComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
    }

    void ExposureMapSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }

    void ExposureMapSystemComponent::BuildGrid()
    {
        const uint16_t cellCount = ComputeCellCount();
        m_grid.resize(cellCount);
        m_distanceToEnemyMap.resize(cellCount);
        m_isPositionObstructedMap.resize(cellCount);
        m_isPositionExposedMap.resize(cellCount);

        auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();
        AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
        if (sceneHandle == AzPhysics::InvalidSceneHandle)
        {
            assert(false);
            return;
        }

        for (uint8_t row = 0; row < m_gridDimension; ++row)
        {
            for (uint8_t column = 0; column < m_gridDimension; ++column)
            {
                const uint16_t cellIndex = ComputeCellIndex(row, column);
                const AZ::Vector3 center = ComputeCellCenter(row, column);

                AzPhysics::RayCastRequest request;
                request.m_start = center + 5.f * AZ::Vector3::CreateAxisZ(); // Above all obstacles
                request.m_direction = AZ::Vector3::CreateAxisZ(-1.f);
                request.m_distance = 4.5f; // Above ground
                request.m_queryType = AzPhysics::SceneQuery::QueryType::Static;

                const AzPhysics::SceneQueryHits result = sceneInterface->QueryScene(sceneHandle, &request);
                const bool hasHit = result && result.m_hits[0].IsValid();

                m_grid[cellIndex] = center;
                m_distanceToEnemyMap[cellIndex] = AZStd::numeric_limits<float>::max();
                m_isPositionObstructedMap[cellIndex] = hasHit;
                m_isPositionExposedMap[cellIndex] = false;
            }
        }
    }

    void ExposureMapSystemComponent::UpdateExposure_SingleThreaded(const AZ::Vector3& eyePosition)
    {
        auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();
        AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
        if (sceneHandle == AzPhysics::InvalidSceneHandle)
        {
            assert(false);
            return;
        }

        const uint16_t cellCount = ComputeCellCount();
        for (uint16_t cellIndex = 0; cellIndex < cellCount; ++cellIndex)
        {
            m_isPositionExposedMap[cellIndex] = !IsCellToPositionObstructed(eyePosition, cellIndex, *sceneInterface, sceneHandle);
        }
    }

} // namespace DelayedResultGathering
