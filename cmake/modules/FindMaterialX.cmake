#
# TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
# All rights reserved.  See LICENSE.txt for license.
#
# Helper CMake module to Find MaterialX include dirs, libraries and document libraries
# Based on https://github.com/PixarAnimationStudios/USD/blob/master/cmake/modules/FindMaterialX.cmake
#
# Copyright 2018 Pixar
#
# Licensed under the Apache License, Version 2.0 (the "Apache License")
# with the following modification; you may not use this file except in
# compliance with the Apache License and the following modification to it:
# Section 6. Trademarks. is deleted and replaced with:
#
# 6. Trademarks. This License does not grant permission to use the trade
#    names, trademarks, service marks, or product names of the Licensor
#    and its affiliates, except as required to comply with Section 4(c) of
#    the License and to reproduce the content of the NOTICE file.
#
# You may obtain a copy of the Apache License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the Apache License with the above modification is
# distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the Apache License for the specific
# language governing permissions and limitations under the Apache License.
#
# Example Usage
#  - Build and Install MaterialX 
#  - register the path to this cmake module using CMAKE_MODULE_PATH
#  - use find_package(MaterialX REQUIRED) to locate core components
#  - additonal components can be found using shaderx, stdsurf and pytton
#     e.g:  find_package(MaterialX REQUIRED COMPONENTS shaderx)
#           will find inc, lib, corelib and generators
#
# Variables defined:
# MATERIALX_FOUND            True if headers and requested libraries were found
# MATERIALX_BASE_DIR         MaterialX root installation directory
# MATERIALX_INCLUDE_DIRS     MaterialX include directory
# MATERIALX_LIB_DIRS         MaterialX lib directory
# MATERIALX_CORE_LIBS        MaterialX Core libraries and ShaderX i.e. Core, Format & GenShader
# MATERIALX_GENERATOR_LIBS   MaterialX Generator libraries i.e GenGlsl, GenOsl
# MATERIALX_RENDER_LIBS      MaterialX Render libraries i.e RenderGlsl, RenderOsl
# MATERIALX_STDLIB_DIR       Path to the MaterialX standard library directory
# MATERIALX_PBRLIB_DIR       Path to the MaterialX pbr library directory
# MATERIALX_STDSURFLIB_DIR   Path to the Standard Surface library directory
# MATERIALX_PYTHON_DIR       Path to MaterialX Python library
# MATERIALX_RESOURCES_DIR    Path to MaterialX Resources (sample data, mtlx etc)
#

# make core variables required
set (MATERIALX_REQUIRED_VARS
        MATERIALX_INCLUDE_DIRS
        MATERIALX_LIB_DIRS
        MATERIALX_STDLIB_DIR
        MATERIALX_CORE_LIBS
    )

# make shaderx a required if requested
if ("shaderx" IN_LIST MaterialX_FIND_COMPONENTS)
 list (APPEND MATERIALX_REQUIRED_VARS
        MATERIALX_GENERATOR_LIBS
        MATERIALX_RENDER_LIBS)
endif()

# make standard surfacce required if requested
if ("stdsurf" IN_LIST MaterialX_FIND_COMPONENTS)
 list (APPEND MATERIALX_REQUIRED_VARS 
        MATERIALX_PBRLIB_DIR
        MATERIALX_STDSURFLIB_DIR)
endif()


if ("python" IN_LIST MaterialX_FIND_COMPONENTS)
 list (APPEND MATERIALX_REQUIRED_VARS
        MATERIALX_PYTHON_DIR
        MATERIALX_RESOURCES_DIR)
endif()



# Locate MaterialX base directory based on ENV var
find_path(MATERIALX_BASE_DIR
    NAMES
        include/MaterialXCore/Library.h
    HINTS
        "${MATERIALX_ROOT}"
        "$ENV{MATERIALX_ROOT}"
    )

find_path(MATERIALX_INCLUDE_DIRS 
    MaterialXCore/Library.h
    HINTS
        "${MATERIALX_ROOT}"
        "$ENV{MATERIALX_ROOT}"
        "${MATERIALX_BASE_DIR}"
    PATH_SUFFIXES
        include
    DOC
        "MaterialX Header Path"
)

set(MATERIALX_CORE_LIB_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}MaterialXCore${CMAKE_STATIC_LIBRARY_SUFFIX})
find_path(MATERIALX_LIB_DIRS 
    "${MATERIALX_CORE_LIB_NAME}"
    HINTS
        "${MATERIALX_ROOT}"
        "$ENV{MATERIALX_ROOT}"
        "${MATERIALX_BASE_DIR}"
    PATH_SUFFIXES
        lib
    DOC
        "MaterialX Library Path"
)

# Path to stdlib library
find_path(MATERIALX_STDLIB_DIR 
    stdlib_defs.mtlx
    HINTS
        "${MATERIALX_ROOT}"
        "$ENV{MATERIALX_ROOT}"
        "${MATERIALX_BASE_DIR}"
    PATH_SUFFIXES
        "libraries/stdlib"
    DOC
        "MaterialX Standard Libraries Path"
)

# Path to pbr library
find_path(MATERIALX_PBRLIB_DIR 
    pbrlib_defs.mtlx
    HINTS
        "${MATERIALX_ROOT}"
        "$ENV{MATERIALX_ROOT}"
        "${MATERIALX_BASE_DIR}"
    PATH_SUFFIXES
        "libraries/pbrlib"
    DOC
        "MaterialX PBR Libraries Path"
)

# Path to standard surface library
find_path(MATERIALX_STDSURFLIB_DIR
    standard_surface.mtlx
    HINTS
        "${MATERIALX_ROOT}"
        "$ENV{MATERIALX_ROOT}"
        "${MATERIALX_BASE_DIR}"
    PATH_SUFFIXES
        "libraries/bxdf"
    DOC
        "MaterialX Standard Surface Libraries Path"
)

# Path to python library
find_path(MATERIALX_PYTHON_DIR
    setup.py
    HINTS
        "${MATERIALX_ROOT}"
        "$ENV{MATERIALX_ROOT}"
        "${MATERIALX_BASE_DIR}"
    PATH_SUFFIXES
        "python"
    DOC
        "MaterialX Python Bindings Path"
)

# Path to MaterialX Resources
find_path(MATERIALX_RESOURCES_DIR
    README.md
    HINTS
        "${MATERIALX_ROOT}"
        "$ENV{MATERIALX_ROOT}"
        "${MATERIALX_BASE_DIR}"
    PATH_SUFFIXES
        "resources"
    DOC
        "MaterialX Resources path"
)



# Core Libraries
foreach(MATERIALX_LIB
    Core
    Format
    GenShader
    Render
    Contrib)
    find_library(MATERIALX_${MATERIALX_LIB}_LIBRARY
            MaterialX${MATERIALX_LIB}
        HINTS
            "${MATERIALX_LIB_DIRS}"
        DOC
            "MaterialX's ${MATERIALX_LIB} library path"
        NO_CMAKE_SYSTEM_PATH
    )

    if (MATERIALX_${MATERIALX_LIB}_LIBRARY)
        list(APPEND MATERIALX_CORE_LIBS ${MATERIALX_${MATERIALX_LIB}_LIBRARY})
    endif ()
endforeach()

# Target generator Libraries 
foreach(MATERIALXGEN_LIB
    Glsl
    Osl
    OgsFx
    Arnold)

    # Locate the genetrator library
    find_library(MATERIALX_GEN_${MATERIALXGEN_LIB}_LIBRARY
            MaterialXGen${MATERIALXGEN_LIB}
        HINTS
            "${MATERIALX_LIB_DIRS}"
        DOC
            "MaterialX's ${MATERIALX_LIB} library path"
        NO_CMAKE_SYSTEM_PATH
    )

    if (MATERIALX_GEN_${MATERIALXGEN_LIB}_LIBRARY)
        list(APPEND MATERIALX_GENERATOR_LIBS ${MATERIALX_GEN_${MATERIALXGEN_LIB}_LIBRARY})
    endif ()
endforeach()

# Target render Libraries 
foreach(MATERIALXRENDER_LIB
    Glsl
    Osl
    Hw)

    # Locate the render library
    find_library(MATERIALX_RENDER_${MATERIALXRENDER_LIB}_LIBRARY
            MaterialXRender${MATERIALXRENDER_LIB}
        HINTS
            "${MATERIALX_LIB_DIRS}"
        DOC
            "MaterialX's ${MATERIALX_LIB} library path"
        NO_CMAKE_SYSTEM_PATH
    )

    if (MATERIALX_RENDER_${MATERIALXRENDER_LIB}_LIBRARY)
        list(APPEND MATERIALX_RENDER_LIBS ${MATERIALX_GEN_${MATERIALXRENDER_LIB}_LIBRARY})
    endif ()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MaterialX
    REQUIRED_VARS ${MATERIALX_REQUIRED_VARS}
)