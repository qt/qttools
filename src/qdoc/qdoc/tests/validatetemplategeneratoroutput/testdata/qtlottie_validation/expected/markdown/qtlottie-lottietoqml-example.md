# lottietoqml Example

An example demonstrating the use of lottietoqml in an application.

This example demonstrates how the [lottietoqml](lottietoqml.md) tool can be used to convert Lottie files into QML and then use these in an application.
# Overview

The example consists of nine vector animations that have been converted using [lottietoqml](lottietoqml.md). They are displayed in a grid in a full screen application window.
If the user clicks on one of the animated images, it will zoom to fill the screen. Since the image is vector-based, it will remain crisp without any scaling artifacts.
The [lottietoqml](lottietoqml.md) does the equivalent to what [svgtoqml](https://doc.qt.io/qt-6/svgtoqml.html) does for .SVG files. It converts as much as possible of the original vector image and animation to [Qt Quick Shapes](https://doc.qt.io/qt-6/qtquick-shapes-qmlmodule.html) and other [Qt Quick](https://doc.qt.io/qt-6/qtquick-module.html) components. The result is a fully scalable and hardware-accelerated rendering of the animation.
For convenience, Lottie files can also be loaded at run-time through the [VectorImage](https://doc.qt.io/qt-6/qml-qtquick-vectorimage.html) component. However, pregenerating the QML file is usually preferable, since this allows preprocessing the QML scene for faster loading.
```cpp
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

```

In this example, the Lottie files are automatically converted to QML using the [CMake](CMake) function [qt_target_qml_from_lottie()](qt-target-qml-from-lottie.md).

# Assets

This example contains animated vector images downloaded from the [Lottiefiles list of free animations](https://lottiefiles.com/free-animations/free). All the images are licensed under the [Lottie Simple License](https://lottiefiles.com/page/license).
- "Fingerprint icon" by Solitudinem
- "Google Icons" by Jeffrey Christopher
- "Happy Star" by Natalia Świerz
- "Heartbeat | Medical" by Sarpreet Kalyan
- "Surprised Boy" by Hizen
- "USA Map with outlines blue" by Marina Trajkovska
- "User Interaction" by Mahendra Bhunwal
- "User Interface" by Bashir Ahmad
- "User Authentication" by Musa Adanur



---

*Built with QDoc's template engine.*
