add_executable (unittest
    test_main.cpp
    Test.cpp
)

target_link_libraries (unittest
    cppunit
    model
    gui
    log
    dl
    boost_system
) 

add_custom_target(test
    DEPENDS unittest
    COMMAND unittest
)
