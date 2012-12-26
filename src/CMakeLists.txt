project(FLOBBY)

add_definitions(${CMAKE_CXX_FLAGS} "-std=c++0x" "-march=native")

set(FLTK_SKIP_OPENGL true)
set(FLTK_SKIP_FORMS true)
# set(FLTK_SKIP_IMAGES true)
set(FLTK_SKIP_FLUID true)
find_package(FLTK REQUIRED)

include_directories (
    ${PROJECT_SOURCE_DIR}
    ${FLTK_INCLUDE_DIR}
)

add_executable (flobby
    main.cpp
)

target_link_libraries (flobby
    controller
    gui
    model
    log
    boost_regex
    boost_chrono
    boost_signals
    boost_thread
    boost_filesystem
    boost_system
    pthread
) 

add_subdirectory (md5)
add_subdirectory (log)
add_subdirectory (controller)
add_subdirectory (model)
add_subdirectory (gui)
add_subdirectory (test)

install (TARGETS flobby RUNTIME DESTINATION bin)
