#include "ExposureMapSentinelComponent.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>

#include <assert.h>

namespace DelayedResultGathering
{
    void ExposureMapSentinelComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ExposureMapSentinelComponent, AZ::Component>()->Version(0);

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                using namespace AZ::Edit;
                editContext
                    ->Class<ExposureMapSentinelComponent>("Exposure Map Sentinel", "Makes this entity a sentinel in the exposure map.")
                    ->ClassElement(ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "AI")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC_CE("Game"));
            }
        }
    }

    void ExposureMapSentinelComponent::Activate()
    {
        assert(false);
    }

    void ExposureMapSentinelComponent::Deactivate()
    {
    }

} // namespace DelayedResultGathering
