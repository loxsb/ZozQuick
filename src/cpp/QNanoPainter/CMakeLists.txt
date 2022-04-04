cmake_minimum_required(VERSION 3.14)

project(QNanoPainter VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)

find_package(Qt5 COMPONENTS Quick REQUIRED)
if(ANDROID)
    find_path(GLES2_INCLUDE_DIR GLES2/gl2.h
        HINTS ${ANDROID_NDK})
    find_library(GLES2_LIBRARY libGLESv2.so
        HINTS ${GLES2_INCLUDE_DIR}/../lib)
endif()

set(
    PROJECT_SOURCES

    qnanopainter.cpp
    qnanocolor.cpp
    qnanolineargradient.cpp
    qnanoimagepattern.cpp
    qnanoimage.cpp
    qnanofont.cpp
    qnanoradialgradient.cpp
    qnanoboxgradient.cpp
    qnanowindow.cpp
    private/qnanodebug.cpp

    qnanoquickitem.cpp
    qnanoquickitempainter.cpp

    nanovg/nanovg.c

    $<$<OR:$<BOOL:${ANDROID}>,$<BOOL:${IOS}>,$<BOOL:${WIN32}>>:private/qnanobackendgles2.cpp>
    $<$<OR:$<BOOL:${ANDROID}>,$<BOOL:${IOS}>,$<BOOL:${WIN32}>>:private/qnanobackendgles3.cpp>
    $<$<NOT:$<BOOL:${ANDROID}>>:private/qnanobackendgl2.cpp>
    $<$<NOT:$<BOOL:${ANDROID}>>:private/qnanobackendgl3.cpp>

    libQNanoPainterData.qrc
)


add_library(
    QNanoPainter
    STATIC
    ${PROJECT_SOURCES}
)

target_compile_definitions(
    QNanoPainter
    PUBLIC
    QNANO_ENABLE_GLES3
    QNANO_QT_GL_INCLUDE
    QNANO_BUILD_GLES_BACKENDS
    $<$<NOT:$<BOOL:${ANDROID}>>:QNANO_BUILD_GL_BACKENDS>
)

target_include_directories(
    QNanoPainter
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<$<BOOL:${ANDROID}>:${GLES2_INCLUDE_DIR}>
)

target_link_libraries(
    QNanoPainter
    PRIVATE
    Qt5::Quick
#    $<$<NOT:$<BOOL:${WIN32}>>:GL>
    $<$<BOOL:${WIN32}>:Qt5::Gui_GLESv2>
    $<$<BOOL:${ANDROID}>:${GLES2_LIBRARY}>
)



