project(plasma-containmentactions-test)

project(plasma_testengine_dataengine)

find_package(KDE4 REQUIRED)

include(KDE4Defaults)

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_BINARY_DIR}
                    ${KDE4_INCLUDES})

set(test_SRCS
    test.cpp
)
kde4_add_ui_files(test_SRCS config.ui)

kde4_add_plugin(plasma_containmentactions_test ${test_SRCS})
target_link_libraries(plasma_containmentactions_test ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_containmentactions_test DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containmentactions-test.desktop DESTINATION ${SERVICES_INSTALL_DIR})
