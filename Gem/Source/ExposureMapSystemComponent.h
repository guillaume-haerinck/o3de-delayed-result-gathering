#pragma once

#include "DelayedResultGathering/ExposureMapInterface.h"

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Jobs/JobCompletion.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/std/containers/array.h>
#include <AzCore/std/parallel/atomic.h>
#include <AzCore/std/tuple.h>

namespace AZ
{
    class Vector2;
    class Job;
} // namespace AZ

namespace AzPhysics
{
    // Dirty but cannot include <AzFramework/Physics/Common/PhysicsTypes.h> from .h it states that file does not exist during build
    class SceneInterface;
    using SceneIndex = AZ::s8;
    using SceneHandle = AZStd::tuple<AZ::Crc32, SceneIndex>;
} // namespace AzPhysics

namespace DelayedResultGathering
{
    //! Split the current level into a grid of accessible/inaccessible areas
    //! We update the accessibility of each areas based on the field of view of the sentinel (grabbed from player tag)
    class ExposureMapSystemComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public AZ::Interface<ExposureMapInterface>::Registrar
    {
    public:
        AZ_COMPONENT(ExposureMapSystemComponent, "{EDEBBD55-69B0-4B9F-B86B-2E208457FB32}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        //! AZ::TickBus::Handler overrides
        int GetTickOrder() final override;
        void OnTick(float deltaTime, AZ::ScriptTimePoint timePoint) final override;

        //! ExposureMapInterface overrides
        bool IsPositionExposed(const AZ::Vector3& position) const override;
        bool FindNearestNonExposedPosition(const AZ::Vector3& currentPosition, AZ::Vector3& positionOut) override;

    private:
        // AZ::Component overrides
        void Activate() override;
        void Deactivate() override;

        uint16_t ComputeCellIndex(uint8_t row, uint8_t column) const;
        AZ::Vector3 ComputeCellCenter(uint8_t row, uint8_t column) const;
        uint16_t PositionToCellIndex(const AZ::Vector2& position) const;

        // Number of cell to perform operations per jobs
        uint16_t ComputeTaskBatchSize() const;

        //! Launch a raycast from cell position to provided position and check if there is an obstacle in between
        bool IsCellToPositionObstructed(
            const AZ::Vector3& position,
            uint16_t cellIndex,
            AzPhysics::SceneInterface& sceneInterface,
            const AzPhysics::SceneHandle& sceneHandle) const;

        void BuildGrid();
        void DebugDrawExposureMap();

        void UpdateDistanceToEnemyMap(const AZ::Vector3& enemyPosition);

        void UpdateExposure_SingleThreaded(const AZ::Vector3& eyePosition);
        void UpdateExposure_MultiThreadedGatherSameFrame(const AZ::Vector3& eyePosition);
        void UpdateExposure_MultiThreadedGatherNextFrame(const AZ::Vector3& eyePosition);
        void UpdateExposure_MultiThreadedTimeSlicing(const AZ::Vector3& eyePosition);

    private:
        uint8_t m_cellSize = 1; // in meters
        bool m_gridNeedRebuild = true;

        static constexpr const uint16_t gridDimension = 40;
        static constexpr const uint16_t cellCount = gridDimension * gridDimension;

        // All vectors below are of the same size with grid being source of truth.
        // Index in array represent a 2D position (see ComputeCellIndex() to see how it is computed)
        AZStd::array<AZ::Vector3, cellCount>
            m_grid; // A 2D grid which covers the current level. The 2D position stored is the center of the cell
        AZStd::array<float, cellCount> m_distanceToEnemyMap; // For each position, distance from cell to enemy
        AZStd::array<bool, cellCount> m_isPositionObstructedMap; // For each position, true if obstructed by an obstacle
        AZStd::array<AZStd::atomic<bool>, cellCount> m_isPositionExposedMap; // For each position, true if visible by a sentinel POV

        AZ::JobCompletion* m_raycastingJobCompletion = nullptr;
    };
} // namespace DelayedResultGathering
