########### next target ###############

kde4_add_executable(kcmremotewidgetshelper remotewidgetshelper.cpp)
target_link_libraries(kcmremotewidgetshelper ${KDE4_KDECORE_LIBS})
install(TARGETS kcmremotewidgetshelper DESTINATION ${LIBEXEC_INSTALL_DIR})

kde4_install_auth_helper_files(kcmremotewidgetshelper org.kde.kcontrol.kcmremotewidgets root)
kde4_install_auth_actions(org.kde.kcontrol.kcmremotewidgets kcm_remotewidgets.actions)

