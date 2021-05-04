cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)

########################################################################################################################

#Create the target
etn_target(shared ${PROJECT_NAME}
    SOURCES
        src/*.c
        src/*.h
    PUBLIC_INCLUDE_DIR
        include_public
    PUBLIC_HEADERS
        fty_proto.h
        ftyproto.h #backward compatibility (deprecated)
    USES
        czmq
)

set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

if (BUILD_TESTING)

    etn_test_target(${PROJECT_NAME}
        SOURCES
            tests/main.cc
            tests/*.c
            tests/*.h
        PREPROCESSOR -DCATCH_CONFIG_FAST_COMPILE
        USES
            czmq
            mlm
            Catch2::Catch2
            stdc++fs
            log4cplus
    )

    ## manual set of include dirs, can't be set in the etn_target_test macro
    get_target_property(INCLUDE_DIRS_TARGET ${PROJECT_NAME} INCLUDE_DIRECTORIES)
    target_include_directories(${PROJECT_NAME}-test PRIVATE ${INCLUDE_DIRS_TARGET})
    target_include_directories(${PROJECT_NAME}-coverage PRIVATE ${INCLUDE_DIRS_TARGET})

endif()
