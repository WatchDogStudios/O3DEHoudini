#
# Copyright (c) Contributors to the Open 3D Engine Project.
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
# FindHoudiniEngine.cmake
# Locates a Houdini install and exposes the HAPI C library via the
# 3rdParty::HoudiniEngine interface target (headers + libHAPIL import lib).
#
# Search order for the Houdini root (HFS):
#   1) HOUDINI_ENGINE_HFS  (cache var)
#   2) ENV{HOUDINI_ENGINE_HFS}
#   3) ENV{HFS}            (set by Houdini's own environment)
# Pin one Houdini version per engine branch — HAPI is version-locked to the
# DLLs in $HFS/bin. Record the pinned version in the gem README.
#
# Pinned for this branch: Houdini 20.0.547 (Houdini Engine API 6.0).
# Verified layout on this version:
#   headers : ${HFS}/toolkit/include/HAPI/HAPI.h
#   import  : ${HFS}/custom/houdini/dsolib/libHAPIL.lib
#   runtime : ${HFS}/bin/libHAPIL.dll

if(TARGET 3rdParty::HoudiniEngine)
    return()
endif()

set(HOUDINI_ENGINE_HFS "" CACHE PATH
    "Absolute path to the Houdini install root (the dir containing toolkit/include/HAPI).")

set(_hfs "")
if(HOUDINI_ENGINE_HFS)
    set(_hfs "${HOUDINI_ENGINE_HFS}")
elseif(DEFINED ENV{HOUDINI_ENGINE_HFS})
    set(_hfs "$ENV{HOUDINI_ENGINE_HFS}")
elseif(DEFINED ENV{HFS})
    set(_hfs "$ENV{HFS}")
endif()

set(_hapi_header "${_hfs}/toolkit/include/HAPI/HAPI.h")
if(NOT _hfs OR NOT EXISTS "${_hapi_header}")
    message(WARNING
        "HoudiniEngine: could not locate a Houdini install. Set HOUDINI_ENGINE_HFS "
        "(cache var) or the HFS environment variable to the Houdini root "
        "(the directory containing toolkit/include/HAPI/HAPI.h). The HoudiniEngine "
        "gem will be disabled.")
    set(PAL_TRAIT_HOUDINI_SUPPORTED FALSE PARENT_SCOPE)
    return()
endif()

# libHAPIL import library path varies by Houdini version:
#   - Newer builds: ${_hfs}/custom/houdini/dsolib/libHAPIL.lib
#   - Older builds: ${_hfs}/toolkit/lib/libHAPIL.lib
# Task 0's spike records which one this branch's pinned Houdini uses.
find_library(HOUDINI_HAPI_LIB
    NAMES libHAPIL HAPIL
    PATHS "${_hfs}/custom/houdini/dsolib" "${_hfs}/toolkit/lib"
    NO_DEFAULT_PATH)

if(NOT HOUDINI_HAPI_LIB)
    message(WARNING "HoudiniEngine: found Houdini headers at ${_hfs} but not libHAPIL. Disabling gem.")
    set(PAL_TRAIT_HOUDINI_SUPPORTED FALSE PARENT_SCOPE)
    return()
endif()

add_library(HoudiniEngine_3rdParty INTERFACE)
add_library(3rdParty::HoudiniEngine ALIAS HoudiniEngine_3rdParty)
target_include_directories(HoudiniEngine_3rdParty INTERFACE "${_hfs}/toolkit/include")
target_link_libraries(HoudiniEngine_3rdParty INTERFACE "${HOUDINI_HAPI_LIB}")
# Propagate the bin dir so the editor can prepend it to PATH at runtime.
target_compile_definitions(HoudiniEngine_3rdParty INTERFACE
    HOUDINI_ENGINE_BIN_DIR="${_hfs}/bin")

message(STATUS "HoudiniEngine: using Houdini at ${_hfs} (lib: ${HOUDINI_HAPI_LIB})")
