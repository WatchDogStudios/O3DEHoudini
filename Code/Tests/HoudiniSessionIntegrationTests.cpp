#include <AzTest/AzTest.h>
#include <Session/HoudiniSession.h>
#include <HAPI/HapiWrapper.h>
#include <Translation/HoudiniGeometryToMesh.h>
#include <HoudiniEngine/HoudiniCookedMesh.h>
#include <AzCore/Utils/Utils.h>

namespace HoudiniEngine
{
    // These tests check out a real Houdini Engine license and cook a real HDA.
    // They self-skip when no license/install is present so the suite stays green
    // on unlicensed machines (CI without Houdini). Gate: env var
    // HOUDINI_ENGINE_RUN_INTEGRATION must be set, and the session must start.
    class HoudiniSessionIntegration : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            if (!AZ::Utils::IsEnvSet("HOUDINI_ENGINE_RUN_INTEGRATION"))
            {
                GTEST_SKIP() << "Set HOUDINI_ENGINE_RUN_INTEGRATION to run licensed Houdini tests.";
            }
            auto started = m_session.Start();
            if (!started.IsSuccess())
            {
                GTEST_SKIP() << "No Houdini Engine session: " << started.GetError().c_str();
            }
        }
        HoudiniSession m_session;
    };

    TEST_F(HoudiniSessionIntegration, LoadCookExtract_BoxHda_ProducesMesh)
    {
        // TestHDAs/box.hda sits next to the test dll; resolve relative to exe dir.
        AZStd::string hda{ AZ::Utils::GetExecutableDirectory().c_str() };
        hda += "/TestHDAs/box.hda";

        auto op = Hapi::LoadFirstOperator(m_session.Get(), hda.c_str());
        ASSERT_TRUE(op.IsSuccess()) << op.GetError().c_str();

        auto node = Hapi::CreateAndCook(m_session.Get(), op.GetValue().c_str());
        ASSERT_TRUE(node.IsSuccess()) << node.GetError().c_str();

        RawCookedPart raw;
        auto extracted = Hapi::ExtractDisplayPart(m_session.Get(), node.GetValue(), raw);
        ASSERT_TRUE(extracted.IsSuccess()) << extracted.GetError().c_str();

        HoudiniCookedMesh mesh;
        ASSERT_TRUE(HoudiniGeometryToMesh::Build(raw, mesh));
        EXPECT_TRUE(mesh.IsValid());
        EXPECT_GE(mesh.m_indices.size(), 12u * 3u); // a box cap-triangulated >= 12 tris
    }
}
