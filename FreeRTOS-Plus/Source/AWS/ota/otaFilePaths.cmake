# This file is to add source files and include directories
# into variables so that it can be reused from different repositories
# in their Cmake based build system by including this file.
#
# Files specific to the repository such as test runner, platform tests
# are not added to the variables.

# 3rdparty source files.
include(source/dependency/coreJSON/jsonFilePaths.cmake)

set( TINYCBOR_SOURCES
    "source/dependency/3rdparty/tinycbor/src/cborpretty.c"
    "source/dependency/3rdparty/tinycbor/src/cborpretty_stdio.c"
    "source/dependency/3rdparty/tinycbor/src/cborencoder.c"
    "source/dependency/3rdparty/tinycbor/src/cborencoder_close_container_checked.c"
    "source/dependency/3rdparty/tinycbor/src/cborerrorstrings.c"
    "source/dependency/3rdparty/tinycbor/src/cborparser.c"
    "source/dependency/3rdparty/tinycbor/src/cborparser_dup_string.c"
)
set(TINYCBOR_INCLUDE_DIRS
    "source/dependency/3rdparty/tinycbor/src"
)
# Use C99 for tinycbor as it is incompatible with C90
if(CMAKE_C_STANDARD LESS 99)
    set_source_files_properties(
        ${TINYCBOR_SOURCES}
        PROPERTIES
        COMPILE_FLAGS "-std=gnu99"
    )
endif()

# OTA library source files, including 3rdparties.
set( OTA_SOURCES
    "source/include/ota.h"
    "source/include/ota_os_interface.h"
    "source/include/ota_platform_interface.h"
    "source/include/ota_private.h"
    "source/include/ota_interface_private.h"
    "source/include/ota_base64_private.h"
    "source/ota.c"
    "source/ota_interface.c"
    "source/ota_base64.c"
    ${JSON_SOURCES}
    ${TINYCBOR_SOURCES}
)

# OTA library public include directories.
set( OTA_INCLUDE_PUBLIC_DIRS
    "source/include"
    "source/portable"
)

# OTA library private include directories.
set( OTA_INCLUDE_PRIVATE_DIRS
    "source"
    ${JSON_INCLUDE_PUBLIC_DIRS}
    ${TINYCBOR_INCLUDE_DIRS}
)

# OTA library POSIX OS porting source files.
# Note: user needs to call find_library(LIB_RT rt REQUIRED) and link with
# ${LIB_RT} because librt is required to use OTA OS POSIX port.
set( OTA_OS_POSIX_SOURCES
    "source/portable/os/ota_os_posix.c"
)

# OTA library POSIX OS porting source files.
set( OTA_INCLUDE_OS_POSIX_DIRS
    "source/portable/os"
)

# OTA library FreeRTOS OS porting source files.
set( OTA_OS_FREERTOS_SOURCES
    "source/portable/os/ota_os_freertos.c"
)

# OTA library FreeRTOS OS porting source files.
set( OTA_INCLUDE_OS_FREERTOS_DIRS
    "source/portable/os"
)

# OTA library MQTT backend source files.
set( OTA_MQTT_SOURCES
    "source/ota_mqtt.c"
    "source/ota_cbor.c"
    "source/include/ota_mqtt_private.h"
    "source/include/ota_cbor_private.h"
)

# OTA library HTTP backend source files.
set( OTA_HTTP_SOURCES
    "source/ota_http.c"
    "source/include/ota_http_private.h"
)
