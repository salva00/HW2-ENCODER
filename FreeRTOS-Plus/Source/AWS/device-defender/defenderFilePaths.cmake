# This file is to add source files and include directories
# into variables so that it can be reused from different repositories
# in their Cmake based build system by including this file.
#
# Files specific to the repository such as test runner, platform tests
# are not added to the variables.

# Device Defender library source files.
set( DEFENDER_SOURCES
     "source/defender.c" )

# Device Defender library public include directories.
set( DEFENDER_INCLUDE_PUBLIC_DIRS
     "source/include" )
