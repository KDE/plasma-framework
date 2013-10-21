project(plasma-containmentactions-test)

project(plasma_testengine_dataengine)

find_package(KDE4 REQUIRED)

include(KDE4Defaults)

set(test_SRCS
    test.cpp
)
qt5_wrap_ui(test_SRCS config.ui)

plasma_add_plugin(plasma_containmentactions_test ${test_SRCS})
target_link_libraries(plasma_containmentactions_test ${Plasma_LIBRARIES} KF5::KIOCore KF5::KIOWidgets)

install(TARGETS plasma_containmentactions_test DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containmentactions-test.desktop DESTINATION ${SERVICES_INSTALL_DIR})
