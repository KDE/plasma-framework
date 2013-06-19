INCLUDE_DIRECTORIES(.. .)

include(ECMMarkAsTest)

add_executable(fullmodelaccesstest columnproxymodeltest.cpp ../columnproxymodel.cpp ../../core/tests/modeltest.cpp)
target_link_libraries(fullmodelaccesstest
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    KDE4__kde4support
    )

add_test(plasma-fullmodelaccesstest fullmodelaccesstest)
ecm_mark_as_test(fullmodelaccesstest)
