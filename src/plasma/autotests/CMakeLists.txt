set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include(ECMMarkAsTest)

include_directories ( ${karchive_SOURCE_DIR}/src
                      ${karchive_BINARY_DIR}/src
)

find_package(KCoreAddons REQUIRED)

# add_definitions( -DKDESRCDIR=${CMAKE_CURRENT_SOURCE_DIR} )

MACRO(PLASMA_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} ${QT_QTTEST_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY}
                                     ${KDE4_KIO_LIBS} plasma ${KArchive_LIBRARIES} ${KCoreAddons_LIBRARIES}
                                     KF5::KConfigGui
                                     KF5::KI18n
				     KF5::KService)
               if(QT_QTOPENGL_FOUND)
                   target_link_libraries(${_testname} ${QT_QTOPENGL_LIBRARY})
               endif(QT_QTOPENGL_FOUND)
               add_test(plasma-${_testname} ${_testname})
               ecm_mark_as_test(${_testname})
       ENDFOREACH(_testname)
ENDMACRO(PLASMA_UNIT_TESTS)

PLASMA_UNIT_TESTS(
    configloadertest
    packagestructuretest
    pluginloadertest
    #    plasmoidpackagetest
    runnercontexttest
)

add_executable(storagetest storagetest.cpp ../private/storage.cpp ../private/storagethread.cpp)
target_link_libraries(storagetest ${QT_QTTEST_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY} ${QT_QTSQL_LIBRARY} ${KDE4_KIO_LIBS} plasma ${KCoreAddons_LIBRARIES} )
add_test(plasma-storagetest storagetest)
ecm_mark_as_test(plasma-storagetest)

if(QT_QTOPENGL_FOUND)
  target_link_libraries(storagetest ${QT_QTOPENGL_LIBRARY})
endif(QT_QTOPENGL_FOUND)

if(QCA2_FOUND)
   #  target_link_libraries(plasmoidpackagetest ${QCA2_LIBRARIES})
endif(QCA2_FOUND)

