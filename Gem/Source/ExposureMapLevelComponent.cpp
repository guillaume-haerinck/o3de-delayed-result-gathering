#include "ExposureMapLevelComponent.h"

#include <AzCore/Math/Aabb.h>
#include <AzCore/Math/Color.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/limits.h>
#include <DebugDraw/DebugDrawBus.h>

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

            const AZ::Vector2& center = m_grid[cellIndex];

            AZ::Aabb aabb = AZ::Aabb::CreateCenterHalfExtents(
                AZ::Vector3(center.GetX(), center.GetY(), 0.f), AZ::Vector3(m_cellSize / 2.2f, m_cellSize / 2.2f, 0.01f));
            const AZ::Color color = AZ::Color::CreateFromRgba(255, 0, 0, 200);
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

    AZ::Vector2 ExposureMapLevelComponent::ComputeCellCenter(uint8_t row, uint8_t column) const
    {
        return AZ::Vector2(static_cast<float>((column + 0.5) * m_cellSize), static_cast<float>((row + 0.5) * m_cellSize));
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
        BuildGrid();
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

        for (uint8_t row = 0; row < m_gridDimension; ++row)
        {
            for (uint8_t column = 0; column < m_gridDimension; ++column)
            {
                const uint16_t cellIndex = ComputeCellIndex(row, column);
                const AZ::Vector2 center = ComputeCellCenter(row, column);

                m_grid[cellIndex] = center;
                m_distanceToSentinelMap[cellIndex] = AZStd::numeric_limits<float>::max();
                m_isPositionObstructedMap[cellIndex] = false; // #GH_TODO need a topdown raycast
                m_isPositionExposedMap[cellIndex] = false;
            }
        }
    }

    void ExposureMapLevelComponent::UpdateExposure_SingleThreaded([[maybe_unused]] const AZ::Vector3& eyePosition)
    {
    }

} // namespace DelayedResultGathering
