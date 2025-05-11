#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/tuple.h>

namespace AZ
{
    class Vector2;
}

namespace AzPhysics
{
    // Dirty but cannot include #include <AzFramework/Physics/Common/PhysicsTypes.h> from .h will have to look at it
    class SceneInterface;
    using SceneIndex = AZ::s8;
    using SceneHandle = AZStd::tuple<AZ::Crc32, SceneIndex>;
} // namespace AzPhysics

namespace DelayedResultGathering
{
    //! Split the current level into a grid of accessible/inaccessible areas
    //! We update the accessibility of each areas based on the field of view of the sentinel it contains
    class ExposureMapLevelComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(ExposureMapLevelComponent, "{EDEBBD55-69B0-4B9F-B86B-2E208457FB32}");

        static void Reflect(AZ::ReflectContext* context);

        //! AZ::TickBus::Handler overrides
        int GetTickOrder() final override;
        void OnTick(float deltaTime, AZ::ScriptTimePoint timePoint) final override;

    private:
        //////////////////////////////////////////////////////////////////////////
        // AZ::Component
        void Activate() override;
        void Deactivate() override;

        uint16_t ComputeCellCount() const;
        uint16_t ComputeCellIndex(uint8_t row, uint8_t column) const;
        AZ::Vector3 ComputeCellCenter(uint8_t row, uint8_t column) const;
        uint16_t PositionToCellIndex(const AZ::Vector2& position) const;

        //! Launch a raycast from cell position to provided position and check if there is an obstacle in between
        bool IsCellToPositionObstructed(
            const AZ::Vector3& position,
            uint16_t cellIndex,
            AzPhysics::SceneInterface& sceneInterface,
            const AzPhysics::SceneHandle& sceneHandle) const;

        void BuildGrid();
        void DebugDrawExposureMap();

        void UpdateExposure_SingleThreaded(const AZ::Vector3& eyePosition);

    private:
        uint8_t m_cellSize = 2; // in meters
        uint8_t m_gridDimension = 20; // in number of cells per dimension
        bool m_gridNeedRebuild = true;

        // All vectors below are of the same size with grid being source of truth.
        // Index in array represent a 2D position (see GetCellIndex() to see how it is computed)
        AZStd::vector<AZ::Vector3> m_grid; // A 2D grid which covers the current level. The 2D position stored is the center of the cell
        AZStd::vector<float> m_distanceToSentinelMap; // For each position, distance from self to sentinel
        AZStd::vector<bool> m_isPositionObstructedMap; // For each position, true if obstructed by an obstacle
        AZStd::vector<bool> m_isPositionExposedMap; // For each position, true if visible by a sentinel POV
    };
} // namespace DelayedResultGathering
