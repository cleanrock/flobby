add_definitions(${CMAKE_CXX_FLAGS} "-std=c++0x")

set(FLTK_SKIP_OPENGL true)
set(FLTK_SKIP_FORMS true)
# set(FLTK_SKIP_IMAGES true)
set(FLTK_SKIP_FLUID true)
find_package(FLTK REQUIRED)
find_package(Boost COMPONENTS system filesystem regex chrono signals thread REQUIRED)

ADD_CUSTOM_TARGET(FlobbyConfig
    ${CMAKE_COMMAND} -D FLOBBY_ROOT=${CMAKE_SOURCE_DIR}
                     -D FLOBBY_CONFIG_H_IN=${CMAKE_CURRENT_SOURCE_DIR}/cmake/FlobbyConfig.h.in
                     -D FLOBBY_CONFIG_H=${CMAKE_CURRENT_SOURCE_DIR}/FlobbyConfig.h
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/FlobbyConfig.cmake
)


include_directories (
    ${CMAKE_SOURCE_DIR}/src
    ${FLTK_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
)

add_executable (flobby
    main.cpp
    FlobbyDirs.cpp
)

add_dependencies(flobby FlobbyConfig)

target_link_libraries (flobby
    controller
    gui
    model
    log
    ${Boost_LIBRARIES}
    pthread
) 

# TODO link pr-d static when it is safe (91.0 unitsync)
#    pr-downloader_static

add_subdirectory (md5)
add_subdirectory (log)
add_subdirectory (controller)
add_subdirectory (model)
add_subdirectory (gui)
add_subdirectory (test)

install (TARGETS flobby RUNTIME DESTINATION bin)
