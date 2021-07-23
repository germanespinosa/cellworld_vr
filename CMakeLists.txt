cmake_minimum_required( VERSION 3.10 )

set(CMAKE_CXX_STANDARD 20)

project( cellworld_vr
        VERSION 2021.1.0
        DESCRIPTION "Cellworld vr utilities"
        LANGUAGES CXX)

find_package(Cellworld REQUIRED)
find_package(Threads)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

###
### MAIN LIBRARY SETUP
###

set(cellworld_vr_files
        src/location3.cpp
        src/rotation3.cpp
        src/state.cpp
        src/net/tcp_server.cpp
        src/net/client.cpp)

add_library(cellworld_vr ${cellworld_vr_files})

set_target_properties(cellworld_vr
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED On
        CXX_EXTENSIONS Off
        VERSION ${PROJECT_VERSION})

if(MSVC)
    target_compile_options(cellworld_vr PRIVATE /W4)
else(MSVC)
    target_compile_options(cellworld_vr PRIVATE -Wall -Wextra -pedantic)
endif(MSVC)

target_include_directories(cellworld_vr
        SYSTEM INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

target_include_directories(cellworld_vr
        PRIVATE
        include )

target_link_libraries( cellworld_vr
        PUBLIC
        ${CELLWORLD_LIBRARIES})


###
### UTILITIES SETUP
###

include_directories( include )

add_executable(cellworld_server src/server.cpp)
target_link_libraries( cellworld_server PUBLIC cellworld_vr )


###
### TESTS
###
find_package(CatchTests CONFIG QUIET)

###
### LIBRARY INSTALLATION
###

if (CMAKE_BUILD_TYPE MATCHES Release)

    include(GNUInstallDirs)

    install(TARGETS cellworld_vr EXPORT Cellworld_vrConfig
            ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})

    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(EXPORT Cellworld_vrConfig
            DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/Cellworld_vr
            EXPORT_LINK_INTERFACE_LIBRARIES)

    export(TARGETS cellworld_vr FILE Cellworld_vrConfig.cmake)
    install(CODE "FILE(APPEND ${CMAKE_INSTALL_FULL_DATADIR}/cmake/Cellworld_vr/Cellworld_vrConfig.cmake \"find_package(Cellworld REQUIRED)\n\") ")
    install(CODE "FILE(APPEND ${CMAKE_INSTALL_FULL_DATADIR}/cmake/Cellworld_vr/Cellworld_vrConfig.cmake \"set(CELLWORLD_TOOLS_LIBRARIES cellworld_vr ${CELLWORLD_LIBRARIES})\n\") ")


    ###
    ### UTILITIES INSTALLATION
    ###

    include(GNUInstallDirs)

    install(TARGETS cellworld_vr
            RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})

endif()