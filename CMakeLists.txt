#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#

o3de_gem_setup("HoudiniEngine")

# Register the 3rdParty finder path so find_package(HoudiniEngine) resolves
# Gems/HoudiniEngine/3rdParty/FindHoudiniEngine.cmake.
ly_add_external_target_path(${CMAKE_CURRENT_SOURCE_DIR}/3rdParty)

add_subdirectory(Code)
