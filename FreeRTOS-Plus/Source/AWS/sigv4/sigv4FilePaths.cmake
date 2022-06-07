# This file sets source files and include directories to variables so that they
# can be reused from different repositories in their CMake based build system by
# including this file.
#
# Files specific to the repository such as test runner and platform tests are
# not added to the variables.

# SigV4 library source files.
set( SIGV4_SOURCES
     "source/sigv4.c"
     "source/sigv4_quicksort.c" )

# SigV4 library public include directories.
set( SIGV4_INCLUDE_PUBLIC_DIRS
     "source/include" )
