cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

########################################################################################################################

etn_target(shared ${PROJECT_NAME}
    SOURCES
        src/*.cpp
        src/*.h
    PUBLIC_INCLUDE_DIR
        include_public
    PUBLIC_HEADERS
        fty_proto.h
    USES
        czmq
)

set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

########################################################################################################################

etn_test_target(${PROJECT_NAME}
    SOURCES
        tests/main.cpp
        tests/mlm_test.cpp
        tests/proto.cpp
    PREPROCESSOR
        -DCATCH_CONFIG_FAST_COMPILE
    SUBDIR
        tests
    USES
        mlm
)

########################################################################################################################
