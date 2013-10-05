INCLUDE_DIRECTORIES(.. .)

include(ECMMarkAsTest)

add_executable(fullmodelaccesstest columnproxymodeltest.cpp ../columnproxymodel.cpp ../../core/tests/modeltest.cpp)
target_link_libraries(fullmodelaccesstest
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    ${KCore_LIBRARIES}
    #${KDE4Support_LIBRARIES}
    KF5::KI18n
    )

add_test(plasma-fullmodelaccesstest fullmodelaccesstest)
ecm_mark_as_test(fullmodelaccesstest)
