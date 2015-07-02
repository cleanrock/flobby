add_executable (unittest EXCLUDE_FROM_ALL
    test_main.cpp
    Test.cpp
    ../FlobbyDirs.cpp
)

target_link_libraries (unittest
    cppunit
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
