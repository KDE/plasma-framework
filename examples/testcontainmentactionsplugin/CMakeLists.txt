add_library(plasma_containmentactions_test MODULE test.cpp)
ki18n_wrap_ui(plasma_containmentactions_test config.ui)

target_link_libraries(plasma_containmentactions_test KF6::Plasma KF6::I18n KF6::KIOWidgets KF6::XmlGui)

install(TARGETS plasma_containmentactions_test DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES plasma-containmentactions-test.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})
