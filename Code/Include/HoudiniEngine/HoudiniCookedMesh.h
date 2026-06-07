#pragma once
#include <AzCore/std/containers/vector.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Vector2.h>

namespace HoudiniEngine
{
    //! Engine- and HAPI-agnostic representation of one cooked Houdini part,
    //! already triangulated. This is the seam that makes geometry translation
    //! unit-testable without a Houdini license: HapiWrapper produces it from
    //! raw HAPI arrays, and HoudiniModelBuilderUtil consumes it to build an
    //! Atom model (for both viewport preview and bake).
    struct HoudiniCookedMesh
    {
        //! One vertex per entry; parallel arrays. uvs/normals may be empty if
        //! the cooked part lacked those attributes.
        AZStd::vector<AZ::Vector3> m_positions;
        AZStd::vector<AZ::Vector3> m_normals;   // empty or size == m_positions
        AZStd::vector<AZ::Vector2> m_uvs;       // empty or size == m_positions

        //! Triangle index list (3 indices per triangle) into the arrays above.
        AZStd::vector<AZ::u32> m_indices;

        bool IsValid() const
        {
            const size_t v = m_positions.size();
            if (v == 0 || m_indices.empty() || (m_indices.size() % 3) != 0)
            {
                return false;
            }
            if (!m_normals.empty() && m_normals.size() != v) { return false; }
            if (!m_uvs.empty() && m_uvs.size() != v) { return false; }
            for (AZ::u32 i : m_indices) { if (i >= v) { return false; } }
            return true;
        }
    };
} // namespace HoudiniEngine
