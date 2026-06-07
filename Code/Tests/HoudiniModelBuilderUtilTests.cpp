#include <AzTest/AzTest.h>
#include <Asset/HoudiniModelBuilderUtil.h>
#include <HoudiniEngine/HoudiniCookedMesh.h>
#include <Atom/RPI.Reflect/Model/ModelAsset.h>

namespace HoudiniEngine
{
    // NOTE on coverage: BuildModelAsset's *positive* path constructs real RPI
    // Buffer/Model assets, which requires the RPI system + asset manager to be
    // running. That infrastructure isn't packaged as a reusable test library
    // (RPITestFixture lives inside Atom_RPI.Tests), so the valid-geometry build
    // is verified end-to-end by the editor/bake integration path rather than a
    // bare unit test.
    //
    // The invalid-mesh guard below returns before any Atom creator is touched,
    // so it is safe to assert in the plain unit environment.
    TEST(HoudiniModelBuilderUtil, BuildModelAsset_InvalidMesh_ReturnsNull)
    {
        HoudiniCookedMesh empty;
        auto model = HoudiniModelBuilderUtil::BuildModelAsset(empty, "Bad");
        EXPECT_TRUE(model.Get() == nullptr);
    }
}
