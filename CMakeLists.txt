install(FILES
    dialogs/background.svg
    dialogs/shutdowndialog.svg
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/dialogs/)

install(FILES
    widgets/background.svg
    widgets/clock.svg
    widgets/toolbox-button.svg
    widgets/connection-established.svg
    widgets/plot-background.svg
    widgets/iconbutton.svg
    widgets/panel-background.svg
    widgets/analog_meter.svg
    widgets/bar_meter_horizontal.svg
    widgets/bar_meter_vertical.svg
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

install(FILES
    colors
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/)

install(FILES
    metadata.desktop
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme)

