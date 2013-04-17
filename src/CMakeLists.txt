project(FLOBBY)

add_definitions(${CMAKE_CXX_FLAGS} "-std=c++0x")

find_package(Boost COMPONENTS system filesystem regex chrono signals thread REQUIRED)


include_directories (
    ${PROJECT_SOURCE_DIR}
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

add_subdirectory (md5)
add_subdirectory (log)
add_subdirectory (controller)
add_subdirectory (model)
add_subdirectory (gui)
add_subdirectory (test)

install (TARGETS flobby RUNTIME DESTINATION bin)
