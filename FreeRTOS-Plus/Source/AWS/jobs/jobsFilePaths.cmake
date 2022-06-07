# This file adds source files and include directories
# into variables for use from different repositories
# in their Cmake based build systems.  Only the library
# files are added.

# JOBS library source files.
set( JOBS_SOURCES
        source/jobs.c)

# JOBS library Public Include directories.
set( JOBS_INCLUDE_PUBLIC_DIRS
        source/include)
