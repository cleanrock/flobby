add_definitions(${CMAKE_CXX_FLAGS} "-std=c++0x")

find_package(Boost COMPONENTS system filesystem regex chrono signals thread REQUIRED)

include_directories (
    ${CMAKE_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
)

add_executable (flobby
    main.cpp
    FlobbyDirs.cpp
)

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
