INCLUDE_DIRECTORIES(
    ${corebindings_SOURCE_DIR}
    )

include(ECMMarkAsTest)

add_executable(sortfiltermodeltest
    sortfiltermodeltest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../plasma/dataengineconsumer.cpp
    ${corebindings_SOURCE_DIR}/datamodel.cpp
    ${corebindings_SOURCE_DIR}/datasource.cpp
    )

target_link_libraries(sortfiltermodeltest
    Plasma
    Qt5::Gui
    Qt5::Test
    ${KDE4Support_LIBRARIES}
    KF5::KI18n
    Qt5::Qml
    )


add_test(plasma-sortfiltermodeltest sortfiltermodeltest)
ecm_mark_as_test(sortfiltermodeltest)

#set(runnermodeltest_SRCS
#    main.cpp
#    dynamictreemodel.cpp
#    modeltest.cpp
#    ../runnermodel.cpp
#    )

#add_executable(runnermodeltest ${runnermodeltest_SRCS})
#target_link_libraries(runnermodeltest Qt5::Test ${plasma_LIBRARIES} plasma)
#ecm_mark_as_test(runnermodeltest)
