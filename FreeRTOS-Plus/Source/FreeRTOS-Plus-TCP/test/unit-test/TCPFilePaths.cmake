# This file is to add source files and include directories
# into variables so that it can be reused from different repositories
# in their Cmake based build system by including this file.

# TCP library source files.
set( TCP_SOURCES
     "../../FreeRTOS_ARP.c"
     "../../FreeRTOS_DNS.c"
     "../../FreeRTOS_DHCP.c"
     "../../FreeRTOS_IP.c"
     "../../FreeRTOS_Sockets.c"
     "../../FreeRTOS_Stream_Buffer.c"
     "../../FreeRTOS_TCP_IP.c"
     "../../FreeRTOS_TCP_WIN.c"
     "../../FreeRTOS_UDP_IP.c" )

# TCP library Include directories.
set( TCP_INCLUDE_DIRS
     "../../include"
     "../../portable/BufferManagement"
     "../../portable/Compiler/MSVC"
     "${CMAKE_CURRENT_LIST_DIR}/stubs" )

set( KERNEL_SOURCES
     "${CMAKE_CURRENT_LIST_DIR}/../FreeRTOS-Kernel/croutine.c"
     "${CMAKE_CURRENT_LIST_DIR}/../FreeRTOS-Kernel/event_groups.c"
     "${CMAKE_CURRENT_LIST_DIR}/../FreeRTOS-Kernel/list.c"
     "${CMAKE_CURRENT_LIST_DIR}/../FreeRTOS-Kernel/queue.c"
     "${CMAKE_CURRENT_LIST_DIR}/../FreeRTOS-Kernel/stream_buffer.c"
     "${CMAKE_CURRENT_LIST_DIR}/../FreeRTOS-Kernel/tasks.c"
     "${CMAKE_CURRENT_LIST_DIR}/../FreeRTOS-Kernel/timers.c" )
