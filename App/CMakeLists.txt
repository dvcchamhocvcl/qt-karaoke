target_sources(${CMAKE_PROJECT_NAME} PUBLIC
    "main.cpp"
    "audiopassthrough.cpp"
    "audiopassthrough.h"
    "audiomixer.cpp"
    "audiomixer.h"
    "mediaplayer.cpp"
    "mediaplayer.h"
    "customaudiooutput.cpp"
    "customaudiooutput.h"
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets)
