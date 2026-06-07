# Test HDAs

Drop a tiny parametric **`box.hda`** here for the opt-in integration test
(`HoudiniSessionIntegrationTests.cpp`).

## How to create `box.hda`

1. In Houdini, create a `Geometry` SOP, dive in, add a `Box` SOP.
2. Select the `geo` node, **Assets ▸ New Digital Asset from Selection…**.
3. Name it (e.g. `box`), save to **this directory** as `box.hda`.
4. Save with the **same Houdini version pinned for this engine branch**
   (HAPI is version-locked — see `3rdParty/FindHoudiniEngine.cmake`).

This file is intentionally not committed as a binary placeholder; the
integration test self-skips (`GTEST_SKIP`) when it is absent or when
`HOUDINI_ENGINE_RUN_INTEGRATION` is unset, so unit-test runs stay green
without it.
