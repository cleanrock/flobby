find_package(Git)

if (GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} -C ${FLOBBY_ROOT} describe --dirty
        OUTPUT_VARIABLE FLOBBY_VERSION
        ERROR_VARIABLE GIT_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
    set(FLOBBY_VERSION "UNKNOWN")
endif()

configure_file( ${FLOBBY_CONFIG_H_IN} ${FLOBBY_CONFIG_H} @ONLY )


