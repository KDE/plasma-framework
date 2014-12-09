set(test_SRCS
    test.cpp
)
ki18n_wrap_ui(test_SRCS config.ui)

add_library(plasma_containmentactions_test MODULE ${test_SRCS})
target_link_libraries(plasma_containmentactions_test KF5::Plasma KF5::I18n KF5::KIOWidgets KF5::XmlGui)

install(TARGETS plasma_containmentactions_test DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containmentactions-test.desktop DESTINATION ${SERVICES_INSTALL_DIR})
