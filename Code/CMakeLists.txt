#
# Copyright (c) Contributors to the Open 3D Engine Project.
# SPDX-License-Identifier: Apache-2.0 OR MIT
#

o3de_gem_setup("HoudiniEngine")

o3de_pal_dir(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME}
    "${gem_restricted_path}" "${gem_path}" "${gem_parent_relative_path}")

include(${pal_dir}/PAL_${PAL_PLATFORM_NAME_LOWERCASE}.cmake)

# Helper: declare the empty client/server aliases (this gem is editor-time only,
# so the runtime/client/server modules ship nothing) plus empty tools aliases.
function(houdini_empty_aliases)
    ly_create_alias(NAME ${gem_name}.Clients NAMESPACE Gem TARGETS)
    ly_create_alias(NAME ${gem_name}.Servers NAMESPACE Gem TARGETS)
    ly_create_alias(NAME ${gem_name}.Unified NAMESPACE Gem TARGETS)
    if(PAL_TRAIT_BUILD_HOST_TOOLS)
        ly_create_alias(NAME ${gem_name}.Tools    NAMESPACE Gem TARGETS)
        ly_create_alias(NAME ${gem_name}.Builders NAMESPACE Gem TARGETS)
    endif()
endfunction()

if(NOT PAL_TRAIT_HOUDINI_SUPPORTED)
    houdini_empty_aliases()
    return()
endif()

find_package(HoudiniEngine)
if(NOT TARGET 3rdParty::HoudiniEngine)
    # FindHoudiniEngine flipped the trait off (no install). Resolve to empty.
    houdini_empty_aliases()
    return()
endif()

# Empty runtime aliases (no game-side Houdini code).
ly_create_alias(NAME ${gem_name}.Clients NAMESPACE Gem TARGETS)
ly_create_alias(NAME ${gem_name}.Servers NAMESPACE Gem TARGETS)
ly_create_alias(NAME ${gem_name}.Unified NAMESPACE Gem TARGETS)

if(PAL_TRAIT_BUILD_HOST_TOOLS)
    # ---------------------------------------------------------------------------
    # Shared implementation built once as a static object, linked into both the
    # editor GEM_MODULE and the test target (FSR3 pattern). Deps are PUBLIC so
    # both consumers inherit the include paths and link libraries.
    # ---------------------------------------------------------------------------
    ly_add_target(
        NAME ${gem_name}.Private.Object STATIC
        NAMESPACE Gem
        FILES_CMAKE
            houdiniengine_private_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
                ${pal_dir}
            PUBLIC
                Include
        BUILD_DEPENDENCIES
            PUBLIC
                AZ::AzCore
                AZ::AzFramework
                AZ::AzToolsFramework
                AZ::AssetBuilderSDK
                Gem::Atom_RPI.Public
                Gem::Atom_RPI.Edit
                Gem::Atom_Feature_Common.Public
                3rdParty::HoudiniEngine
    )

    # Editor / Tools module.
    ly_add_target(
        NAME ${gem_name}.Editor GEM_MODULE
        NAMESPACE Gem
        FILES_CMAKE
            houdiniengine_editor_shared_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
                ${pal_dir}
            PUBLIC
                Include
        BUILD_DEPENDENCIES
            PRIVATE
                Gem::${gem_name}.Private.Object
    )

    ly_create_alias(NAME ${gem_name}.Tools    NAMESPACE Gem TARGETS Gem::${gem_name}.Editor)
    ly_create_alias(NAME ${gem_name}.Builders NAMESPACE Gem TARGETS Gem::${gem_name}.Editor)
endif()

# Tests
if(PAL_TRAIT_BUILD_TESTS_SUPPORTED AND PAL_TRAIT_HOUDINI_TEST_SUPPORTED AND PAL_TRAIT_BUILD_HOST_TOOLS)
    ly_add_target(
        NAME ${gem_name}.Tests ${PAL_TRAIT_TEST_TARGET_TYPE}
        NAMESPACE Gem
        FILES_CMAKE
            houdiniengine_tests_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
                Tests
                ${pal_dir}
            PUBLIC
                Include
        BUILD_DEPENDENCIES
            PRIVATE
                AZ::AzTest
                Gem::${gem_name}.Private.Object
    )
    ly_add_googletest(NAME Gem::${gem_name}.Tests)

    add_custom_command(TARGET ${gem_name}.Tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_LIST_DIR}/Tests/TestHDAs
            $<TARGET_FILE_DIR:${gem_name}.Tests>/TestHDAs)
endif()
