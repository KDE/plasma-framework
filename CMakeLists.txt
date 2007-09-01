install(FILES
    dialogs/background.svg
    dialogs/shutdowndlg.svg
    dialogs/shutdowndlgbuttonglow.svg
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/dialogs/)

install(FILES
    widgets/background.svg
    widgets/clock.svg
    widgets/toolbox-button.svg
    widgets/connection-established.svg
    widgets/plot-background.svg
    widgets/iconbutton.svg
    widgets/wallpaper.svg
    widgets/panel-background.svg
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

install(FILES
    desktoptheme-default.desktop
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme)

