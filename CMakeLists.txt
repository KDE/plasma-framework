install(FILES
    dialogs/background.svg
    dialogs/krunner.svg
    dialogs/shutdowndialog.svg
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/dialogs/)

install(FILES
    opaque/dialogs/background.svg
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/opaque/dialogs/)

install(FILES
    opaque/widgets/tooltip.svg
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/opaque/widgets/)

install(FILES
    widgets/background.svg
    widgets/branding.svg
    widgets/tooltip.svg
    widgets/clock.svg
    widgets/pager.svg
    widgets/translucentbackground.svg
    widgets/systemtray.svg
    widgets/tasks.svg
    widgets/toolbox-button.svg
    widgets/connection-established.svg
    widgets/plot-background.svg
    widgets/panel-background.svg
    widgets/analog_meter.svg
    widgets/bar_meter_horizontal.svg
    widgets/bar_meter_vertical.svg
    widgets/containment-controls.svg
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

install(FILES
    colors
    metadata.desktop
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/)
