# This file is to add source files and include directories
# into variables so that it can be reused from different repositories
# in their Cmake based build system by including this file.
#
# Files specific to the repository such as test runner, platform tests
# are not added to the variables.


# MQTT Agent library Public Include directories.
set( MQTT_AGENT_INCLUDE_PUBLIC_DIRS
     "source/include" )

# MQTT Agent library source files.
set( MQTT_AGENT_SOURCES
     "source/core_mqtt_agent.c"
     "source/core_mqtt_agent_command_functions.c" )

