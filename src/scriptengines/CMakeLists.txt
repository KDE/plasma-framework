add_subdirectory(qml)

add_subdirectory(ruby)

if (PYTHONLIBRARY_FOUND AND NOT WIN32)
    add_subdirectory(python)
endif ()
