cmake_minimum_required( VERSION 3.10 )

set(CMAKE_CXX_STANDARD 20)

project( cellworld_vr
        VERSION 2021.1.0
        DESCRIPTION "Cellworld vr utilities"
        LANGUAGES CXX)

find_package(Cellworld_tools REQUIRED)
find_package(Easy-tcp REQUIRED)
find_package(Threads)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

###
### MAIN LIBRARY SETUP
###

set(cellworld_vr_files
        src/location3.cpp
        src/rotation3.cpp
        src/state.cpp
        src/world_conversion.cpp
        src/vr_episode.cpp
        src/vr_experiment.cpp
        src/vr_service.cpp)

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
        ${CELLWORLD_TOOLS_LIBRARIES}
        easy-tcp)


###
### UTILITIES SETUP
###

include_directories( include )

add_executable(cellworld_server src/server.cpp)
target_link_libraries( cellworld_server PUBLIC cellworld_vr )

add_executable(cellworld_convert src/convert_experiment.cpp)
target_link_libraries( cellworld_convert PUBLIC cellworld_vr )

add_executable(set_world src/console/set_world.cpp)
target_link_libraries( set_world PUBLIC cellworld_vr )

add_executable(get_participants src/console/get_participants.cpp)
target_link_libraries( get_participants PUBLIC cellworld_vr )

add_executable(new_experiment src/console/new_experiment.cpp)
target_link_libraries( new_experiment PUBLIC cellworld_vr )

add_executable(start_training src/console/start_training.cpp)
target_link_libraries( start_training PUBLIC cellworld_vr )

add_executable(stop_training src/console/stop_training.cpp)
target_link_libraries( stop_training PUBLIC cellworld_vr )

add_executable(set_speed src/console/set_speed.cpp)
target_link_libraries( set_speed PUBLIC cellworld_vr )

add_executable(set_turning_speed src/console/set_turning_speed.cpp)
target_link_libraries( set_turning_speed PUBLIC cellworld_vr )

add_executable(show_visibility src/console/show_visibility.cpp)
target_link_libraries( show_visibility PUBLIC cellworld_vr )

add_executable(hide_visibility src/console/hide_visibility.cpp)
target_link_libraries( hide_visibility PUBLIC cellworld_vr )

add_executable(set_view_angle src/console/set_view_angle.cpp)
target_link_libraries( set_view_angle PUBLIC cellworld_vr )

add_executable(set_min_ghost_distance src/console/set_min_ghost_distance.cpp)
target_link_libraries( set_min_ghost_distance PUBLIC cellworld_vr )

add_executable(set_incognito_mode src/console/set_incognito_mode.cpp)
target_link_libraries( set_incognito_mode PUBLIC cellworld_vr )

add_executable(set_participant_name src/console/set_participant_name.cpp)
target_link_libraries( set_participant_name PUBLIC cellworld_vr )

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