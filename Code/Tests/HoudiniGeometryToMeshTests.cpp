#include <AzTest/AzTest.h>
#include <Translation/HoudiniGeometryToMesh.h>
#include <HoudiniEngine/HoudiniCookedMesh.h>

namespace HoudiniEngine
{
    // A single quad: 4 points, 1 face of 4 vertices. Expect 2 triangles (6 indices),
    // with Houdini's clockwise winding reversed to CCW.
    TEST(HoudiniGeometryToMesh, Quad_TriangulatesToTwoTrianglesReversedWinding)
    {
        RawCookedPart raw;
        raw.m_positions = { 0,0,0,  1,0,0,  1,1,0,  0,1,0 }; // 4 points (xyz flat)
        raw.m_faceCounts = { 4 };
        raw.m_vertexList = { 0, 1, 2, 3 };                    // point index per face-vertex

        HoudiniCookedMesh mesh;
        const bool ok = HoudiniGeometryToMesh::Build(raw, mesh);

        EXPECT_TRUE(ok);
        EXPECT_TRUE(mesh.IsValid());
        EXPECT_EQ(mesh.m_positions.size(), 4u);
        ASSERT_EQ(mesh.m_indices.size(), 6u);
        // Houdini fan (0,1,2),(0,2,3) reversed for CCW -> (0,2,1),(0,3,2).
        EXPECT_EQ(mesh.m_indices[0], 0u);
        EXPECT_EQ(mesh.m_indices[1], 2u);
        EXPECT_EQ(mesh.m_indices[2], 1u);
        EXPECT_EQ(mesh.m_indices[3], 0u);
        EXPECT_EQ(mesh.m_indices[4], 3u);
        EXPECT_EQ(mesh.m_indices[5], 2u);
    }

    TEST(HoudiniGeometryToMesh, Triangle_PassesThroughReversed)
    {
        RawCookedPart raw;
        raw.m_positions = { 0,0,0,  1,0,0,  0,1,0 };
        raw.m_faceCounts = { 3 };
        raw.m_vertexList = { 0, 1, 2 };

        HoudiniCookedMesh mesh;
        ASSERT_TRUE(HoudiniGeometryToMesh::Build(raw, mesh));
        ASSERT_EQ(mesh.m_indices.size(), 3u);
        EXPECT_EQ(mesh.m_indices[0], 0u);
        EXPECT_EQ(mesh.m_indices[1], 2u);
        EXPECT_EQ(mesh.m_indices[2], 1u);
    }

    TEST(HoudiniGeometryToMesh, EmptyInput_Fails)
    {
        RawCookedPart raw;
        HoudiniCookedMesh mesh;
        EXPECT_FALSE(HoudiniGeometryToMesh::Build(raw, mesh));
    }
}

// AzTest entry point for this gem's GoogleTest module (exports AzRunUnitTests).
AZ_UNIT_TEST_HOOK(DEFAULT_UNIT_TEST_ENV);
