#include <Translation/HoudiniGeometryToMesh.h>
#include <HoudiniEngine/HoudiniCookedMesh.h>

namespace HoudiniEngine
{
    bool HoudiniGeometryToMesh::Build(const RawCookedPart& raw, HoudiniCookedMesh& out)
    {
        out = {};

        const size_t pointCount = raw.m_positions.size() / 3;
        if (pointCount == 0 || raw.m_faceCounts.empty() || raw.m_vertexList.empty())
        {
            return false;
        }

        const bool haveNormals = !raw.m_normals.empty();
        const bool haveUvs = !raw.m_uvs.empty() && (raw.m_uvTupleSize == 2 || raw.m_uvTupleSize == 3);

        // HAPI normals/uvs here are vertex-domain (one per face-vertex). To keep
        // those per-corner attributes we emit a unique mesh vertex per face-vertex.
        // (Point-domain sharing is a later optimization; correctness first.)
        out.m_positions.reserve(raw.m_vertexList.size());
        if (haveNormals) { out.m_normals.reserve(raw.m_vertexList.size()); }
        if (haveUvs) { out.m_uvs.reserve(raw.m_vertexList.size()); }

        size_t vtx = 0; // running index into m_vertexList (face-vertex counter)
        for (int faceSize : raw.m_faceCounts)
        {
            if (faceSize < 3)
            {
                vtx += (faceSize > 0 ? static_cast<size_t>(faceSize) : 0);
                continue; // skip degenerate faces
            }

            // Emit this face's vertices as unique mesh vertices, recording their
            // new indices so we can fan-triangulate them below.
            AZStd::vector<AZ::u32> faceVerts;
            faceVerts.reserve(faceSize);
            for (int i = 0; i < faceSize; ++i)
            {
                const size_t faceVertexIndex = vtx + i;
                const int pointIndex = raw.m_vertexList[faceVertexIndex];
                if (pointIndex < 0 || static_cast<size_t>(pointIndex) >= pointCount)
                {
                    return false;
                }

                const auto newIndex = static_cast<AZ::u32>(out.m_positions.size());
                out.m_positions.emplace_back(
                    raw.m_positions[pointIndex * 3 + 0],
                    raw.m_positions[pointIndex * 3 + 1],
                    raw.m_positions[pointIndex * 3 + 2]);

                if (haveNormals)
                {
                    out.m_normals.emplace_back(
                        raw.m_normals[faceVertexIndex * 3 + 0],
                        raw.m_normals[faceVertexIndex * 3 + 1],
                        raw.m_normals[faceVertexIndex * 3 + 2]);
                }
                if (haveUvs)
                {
                    const size_t base = faceVertexIndex * raw.m_uvTupleSize;
                    out.m_uvs.emplace_back(raw.m_uvs[base + 0], raw.m_uvs[base + 1]);
                }
                faceVerts.push_back(newIndex);
            }

            // Fan triangulation (0, k, k+1), winding reversed (CW->CCW): (0, k+1, k).
            for (int k = 1; k + 1 < faceSize; ++k)
            {
                out.m_indices.push_back(faceVerts[0]);
                out.m_indices.push_back(faceVerts[k + 1]);
                out.m_indices.push_back(faceVerts[k]);
            }

            vtx += static_cast<size_t>(faceSize);
        }

        return out.IsValid();
    }
} // namespace HoudiniEngine
