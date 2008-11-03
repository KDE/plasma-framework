
########### next target ###############

add_subdirectory( lsofui )
add_subdirectory( processcore )
add_subdirectory( processui )
add_subdirectory( tests )

check_include_files(sys/ptrace.h HAVE_SYS_PTRACE_H)

configure_file(config-ksysguard.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksysguard.h )


