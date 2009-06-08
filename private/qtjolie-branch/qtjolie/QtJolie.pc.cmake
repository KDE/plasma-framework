prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@EXEC_INSTALL_PREFIX@
libdir=@LIB_INSTALL_DIR@
includedir=@INCLUDE_INSTALL_DIR@

Name: QtJolie
Description: A QtDbus like API for JOLIE
Version: 1.0.0
Requires: QtCore
Libs: -L${libdir} -llibQtJolie
Cflags: -I${includedir}
