#
# Copyright (c) Contributors to the Open 3D Engine Project.
# SPDX-License-Identifier: Apache-2.0 OR MIT
#

# Windows platform traits for the HoudiniEngine gem.
# Supported only when a Houdini install is discoverable (HFS env var or
# HOUDINI_ENGINE_HFS cache var). The FindHoudiniEngine module flips this off
# with a clear message if it cannot locate libHAPIL.
set(PAL_TRAIT_HOUDINI_SUPPORTED TRUE)
set(PAL_TRAIT_HOUDINI_TEST_SUPPORTED TRUE)
