project(FLOBBY)

add_definitions(${CMAKE_CXX_FLAGS} "-std=c++0x" "-march=native")

include_directories (
    ${PROJECT_SOURCE_DIR}
)

add_executable (flobby
    main.cpp
)

target_link_libraries (flobby
    controller
    gui
    model
    log
    boost_signals
    boost_thread
    boost_system
    boost_filesystem
    pthread
) 

add_subdirectory (md5)
add_subdirectory (log)
add_subdirectory (controller)
add_subdirectory (model)
add_subdirectory (gui)
add_subdirectory (test)
