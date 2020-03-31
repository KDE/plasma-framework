add_library(plasma_engine_testengine MODULE testengine.cpp)

kcoreaddons_desktop_to_json(
    plasma_engine_testengine plasma-dataengine-testengine.desktop
    SERVICE_TYPES ${CMAKE_SOURCE_DIR}/src/plasma/data/servicetypes/plasma-dataengine.desktop
)


target_link_libraries(plasma_engine_testengine
                      KF5::KIOWidgets KF5::Plasma)
