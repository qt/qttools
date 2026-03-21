# qtlottieviewer Example

An example demonstrating the use of Qt LottieAnimation in an application.

This example demonstrates how to use the [LottieAnimation](qml-qt-labs-lottieqt-lottieanimation.md) item of the [Qt.labs.lottieqt](qml-qt-labs-lottieqt-qmlmodule.md) module to directly display and control a Lottie animated vector graphics file in a Qt Quick application.
# Overview

The main part of the example window is a Rectangle containing a [LottieAnimation](qml-qt-labs-lottieqt-lottieanimation.md) item which plays a Lottie animation.
Below this is a row controls. The user can control the animation speed with the Frame Rate slider, and zoom in and out using the Scale slider. The animation can be stopped and restarted using the Play/Pause button. When paused, the user can select the exact frame to be displayed using the Goto Frame slider. The Open button will display a file dialog, allowing the user to select a Lottie file to load and display.
[LottieAnimation](qml-qt-labs-lottieqt-lottieanimation.md) is a [QQuickPaintedItem](https://doc.qt.io/qt-6/qquickpainteditem.html), employing software rasterization of the vector graphics. For alternative ways displaying Lottie files in Quick applications, utilizing graphics hardware rendering, see [lottietoqml](lottietoqml.md) and the [VectorImage](https://doc.qt.io/qt-6/qml-qtquick-vectorimage.html) item.


---

*Built with QDoc's template engine.*
