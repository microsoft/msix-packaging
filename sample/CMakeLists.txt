# Copyright (C) 2017 Microsoft.  All rights reserved.
# See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

add_subdirectory(ExtractContentsSample)
add_subdirectory(BundleSample)
add_subdirectory(OverrideStreamSample)
add_subdirectory(OverrideLanguageSample)

add_dependencies(ExtractContentsSample msix)
add_dependencies(BundleSample msix)
add_dependencies(OverrideStreamSample msix)
add_dependencies(OverrideLanguageSample msix)
