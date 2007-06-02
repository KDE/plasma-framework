install(FILES
    dialogs/background.svg
    dialogs/shutdowndlg.svg
    dialogs/shutdowndlgbuttonglow.svg
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/dialogs/)

install(FILES
    widgets/clock.svg
    widgets/default_theme.svg
    widgets/iconbutton.svg
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

install(FILES
    desktoptheme-default.desktop
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme)

