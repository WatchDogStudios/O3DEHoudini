#pragma once
#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/std/string/string_view.h>

namespace AZ::RPI { class ModelAsset; }

namespace HoudiniEngine
{
    struct HoudiniCookedMesh;

    class HoudiniModelBuilderUtil
    {
    public:
        //! Build an in-memory ModelAsset (1 LOD, 1 mesh) from a cooked mesh.
        //! Returns a null asset if `mesh.IsValid()` is false. `name` seeds the
        //! asset hint and is used for the buffer/material names.
        static AZ::Data::Asset<AZ::RPI::ModelAsset> BuildModelAsset(
            const HoudiniCookedMesh& mesh, AZStd::string_view name);
    };
} // namespace HoudiniEngine
