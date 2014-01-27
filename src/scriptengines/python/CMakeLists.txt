project(plasma-python)

include(PythonMacros)

# install the library, .desktop, and plasma.py
python_install(plasma_importer.py ${DATA_INSTALL_DIR}/plasma_scriptengine_python)
python_install(pydataengine.py ${DATA_INSTALL_DIR}/plasma_scriptengine_python)
python_install(pyrunner.py ${DATA_INSTALL_DIR}/plasma_scriptengine_python)

# remove the following three lines once we branch 4.5, they are only necessary as long
# as people update kdebase but not yet kdelibs (PYTHON_SITE_PACKAGES_INSTALL_DIR has just
# recently been added to FindPYthonLib.cmake and should be used as install destination). Alex
if(NOT PYTHON_SITE_PACKAGES_INSTALL_DIR)
  set(PYTHON_SITE_PACKAGES_INSTALL_DIR ${PYTHON_SITE_PACKAGES_DIR} )
endif()

python_install(plasmascript.py ${PYTHON_SITE_PACKAGES_INSTALL_DIR}/PyKDE4)

install(FILES plasma-scriptengine-dataengine-python.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma-scriptengine-runner-python.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

