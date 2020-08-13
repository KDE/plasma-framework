# Finds 7z.
#
#  7z_FOUND     - True if 7z is found.
#  7z_EXECUTABLE - Path to executable

#=============================================================================
# SPDX-FileCopyrightText: 2019 Friedrich W. H. Kossebau <kossebau@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_program(7z_EXECUTABLE NAMES 7z.exe 7za.exe)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(7z
    FOUND_VAR
        7z_FOUND
    REQUIRED_VARS
        7z_EXECUTABLE
)
mark_as_advanced(7z_EXECUTABLE)

set_package_properties(7z PROPERTIES
    URL "https://www.7-zip.org/"
    DESCRIPTION "Data (de)compression program"
)
