# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

project(lottietoqmlexample VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick LottieTools)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(applottietoqmlexample
    WIN32
    MACOSX_BUNDLE
    main.cpp
)

target_link_libraries(applottietoqmlexample PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

qt_add_qml_module(applottietoqmlexample
    URI lottietoqmlexample
    VERSION 1.0
    QML_FILES
        Main.qml
        GridIcon.qml
)

#![lottietoqml]
qt_target_qml_from_lottie(applottietoqmlexample
    CURVE_RENDERER
    OPTIMIZE_PATHS
    FILES
        original/FingerprintIcon.json
        original/GoogleIcons.json
        original/HappyStar.json
        original/HeartMedical.json
        original/SurprisedBoy.json
        original/USAMapWithOutlines.json
        original/UserAuthentication.json
        original/UserInteractionAnimation.json
        original/UserInterface.json
    OUTPUTS
        generated/FingerprintIcon.qml
        generated/GoogleIcons.qml
        generated/HappyStar.qml
        generated/HeartMedical.qml
        generated/SurprisedBoy.qml
        generated/USAMapWithOutlines.qml
        generated/UserAuthentication.qml
        generated/UserInteractionAnimation.qml
        generated/UserInterface.qml
)
#![lottietoqml]

install(TARGETS applottietoqmlexample
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET applottietoqmlexample
    OUTPUT_SCRIPT deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
