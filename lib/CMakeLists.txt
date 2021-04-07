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
    USES
        czmq
    PRIVATE
)

if (BUILD_TESTING)

    etn_test(${PROJECT_NAME}-test
        SOURCES
            tests/main.cc
            tests/*.c
            tests/*.h
            src/*.c
            src/*.h
        INCLUDE_DIR
            include_public
        PREPROCESSOR -DCATCH_CONFIG_FAST_COMPILE
        USES
            czmq
            mlm
            Catch2::Catch2
            stdc++fs
    )

    #copy selftest-ro, build selftest-rw for test in/out
    #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tests/selftest-ro DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    #file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/selftest-rw)

    #enable coverage
    etn_coverage(${PROJECT_NAME}-test)

    #copy selftest-ro, build selftest-rw for test coverage
    #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tests/selftest-ro DESTINATION ${CMAKE_BINARY_DIR})
    #file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/selftest-rw)

endif()
