
set (plasmapkg_SRCS
     main.cpp
     plasmapkg.cpp
)

ecm_qt_declare_logging_category(plasmapkg_SRCS HEADER debug_p.h IDENTIFIER LOG_PLASMAPKG CATEGORY_NAME org.kde.plasmapkg)

add_executable(plasmapkg2
    ${plasmapkg_SRCS}
)

target_link_libraries(plasmapkg2 KF5::Plasma KF5::I18n KF5::Service KF5::CoreAddons Qt5::DBus)

install(TARGETS plasmapkg2 ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

