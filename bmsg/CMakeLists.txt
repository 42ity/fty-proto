cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

########################################################################################################################

#Create the target
etn_target(exe bmsg
    SOURCES
        src/*.cpp
        src/*.h
    USES
        ${PROJECT_NAME}
        czmq
        mlm
        fty_common_logging
)
