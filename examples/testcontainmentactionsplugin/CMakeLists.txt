remove_definitions(-DTRANSLATION_DOMAIN=\"libplasma6\")  # Undo the global domain
add_definitions(-DTRANSLATION_DOMAIN=\"plasma_containmentactions_test\")

set(plasma_containmentactions_test_SRCS
    test.cpp
    test.h
)
ki18n_wrap_ui(plasma_containmentactions_test_SRCS config.ui)

kcoreaddons_add_plugin(plasma_containmentactions_test
    SOURCES ${plasma_containmentactions_test_SRCS}
    INSTALL_NAMESPACE "plasma/containmentactions"
)

set_target_properties(plasma_containmentactions_test PROPERTIES
    OUTPUT_NAME org.kde.test
)

target_link_libraries(plasma_containmentactions_test
    Qt::Widgets
    Plasma::Plasma
    KF6::I18n
)
