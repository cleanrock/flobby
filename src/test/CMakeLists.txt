find_package(Boost REQUIRED COMPONENTS system filesystem regex chrono signals thread OPTIONAL_COMPONENTS unit_test_framework)

add_executable (unittest EXCLUDE_FROM_ALL
    Test.cpp
    ../FlobbyDirs.cpp
)

target_link_libraries (unittest
    model
    gui
    log
    dl
    ${Boost_LIBRARIES}
    pthread
) 

add_custom_target(runtest
    DEPENDS unittest
    COMMAND unittest
)
