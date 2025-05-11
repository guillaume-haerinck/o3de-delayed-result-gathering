#include "ExposureMapLevelComponent.h"

#include <AzCore/Interface/Interface.h>
#include <AzCore/Math/Aabb.h>
#include <AzCore/Math/Color.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/limits.h>
#include <AzFramework/Physics/Common/PhysicsSceneQueries.h>
#include <AzFramework/Physics/Common/PhysicsTypes.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <DebugDraw/DebugDrawBus.h>
#include <assert.h>

namespace DelayedResultGathering
{

    void ExposureMapLevelComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ExposureMapLevelComponent, AZ::Component>()->Version(0);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                using namespace AZ::Edit;
                editContext->Class<ExposureMapLevelComponent>("Exposure Map", "Build an exposure map on this level.")
                    ->ClassElement(ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "AI")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Level"));
            }
        }
    }

    int ExposureMapLevelComponent::GetTickOrder()
    {
        return AZ::ComponentTickBus::TICK_GAME;
    }

    void ExposureMapLevelComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint timePoint)
    {
        if (m_gridNeedRebuild)
        {
            m_gridNeedRebuild = false;
            BuildGrid();
        }
        DebugDrawExposureMap();
    }

    void ExposureMapLevelComponent::DebugDrawExposureMap()
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

    uint16_t ExposureMapLevelComponent::ComputeCellCount() const
    {
        return m_gridDimension * m_gridDimension;
    }

    uint16_t ExposureMapLevelComponent::ComputeCellIndex(uint8_t row, uint8_t column) const
    {
        return row * m_gridDimension + column;
    }

    AZ::Vector3 ExposureMapLevelComponent::ComputeCellCenter(uint8_t row, uint8_t column) const
    {
        return AZ::Vector3(static_cast<float>((column + 0.5) * m_cellSize), static_cast<float>((row + 0.5) * m_cellSize), 0.f);
    }

    uint16_t ExposureMapLevelComponent::PositionToCellIndex(const AZ::Vector2& position) const
    {
        const uint8_t row = static_cast<uint8_t>(floor(position.GetY() / m_cellSize));
        const uint8_t column = static_cast<uint8_t>(floor(position.GetX() / m_cellSize));
        return ComputeCellIndex(row, column);
    }

    void ExposureMapLevelComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
    }

    void ExposureMapLevelComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }

    void ExposureMapLevelComponent::BuildGrid()
    {
        const uint16_t cellCount = ComputeCellCount();
        m_grid.resize(cellCount);
        m_distanceToSentinelMap.resize(cellCount);
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
                m_distanceToSentinelMap[cellIndex] = AZStd::numeric_limits<float>::max();
                m_isPositionObstructedMap[cellIndex] = hasHit;
                m_isPositionExposedMap[cellIndex] = false;
            }
        }
    }

    void ExposureMapLevelComponent::UpdateExposure_SingleThreaded([[maybe_unused]] const AZ::Vector3& eyePosition)
    {
    }

} // namespace DelayedResultGathering
