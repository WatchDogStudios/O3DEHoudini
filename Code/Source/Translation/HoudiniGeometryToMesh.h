#pragma once
#include <AzCore/std/containers/vector.h>
#include <AzCore/base.h>

namespace HoudiniEngine
{
    struct HoudiniCookedMesh;

    //! Raw, HAPI-shaped arrays for one cooked part. Flat layouts exactly as the
    //! HAPI getters return them, so HapiWrapper can fill this with no reshaping.
    struct RawCookedPart
    {
        AZStd::vector<float> m_positions;       // xyz per point, flat (size = pointCount*3)
        AZStd::vector<int>   m_faceCounts;      // vertices per face (n-gon sizes)
        AZStd::vector<int>   m_vertexList;      // point index per face-vertex (size = sum of faceCounts)
        AZStd::vector<float> m_normals;         // optional, xyz per face-vertex, flat
        AZStd::vector<float> m_uvs;             // optional, uv per face-vertex, flat (tupleSize 2 or 3)
        int m_uvTupleSize = 0;                  // 2 or 3 when m_uvs present
    };

    class HoudiniGeometryToMesh
    {
    public:
        //! Triangulate a raw cooked part into an indexed, CCW-wound mesh.
        //! Returns false on empty/malformed input. Normals/uvs are carried per
        //! resulting vertex when present (face-vertex attributes are expanded to
        //! unique vertices; points without attributes share by point index).
        static bool Build(const RawCookedPart& raw, HoudiniCookedMesh& out);
    };
} // namespace HoudiniEngine
