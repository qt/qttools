# qtlottieviewer Example

An example demonstrating the use of Qt LottieAnimation in an application.

This example demonstrates how to use the [LottieAnimation](LottieAnimation) item of the [Qt.labs.lottieqt](Qt.labs.lottieqt) module to directly display and control a Lottie animated vector graphics file in a Qt Quick application.
# Overview

The main part of the example window is a Rectangle containing a [LottieAnimation](LottieAnimation) item which plays a Lottie animation.
Below this is a row controls. The user can control the animation speed with the Frame Rate slider, and zoom in and out using the Scale slider. The animation can be stopped and restarted using the Play/Pause button. When paused, the user can select the exact frame to be displayed using the Goto Frame slider. The Open button will display a file dialog, allowing the user to select a Lottie file to load and display.
[LottieAnimation](LottieAnimation) is a [QQuickPaintedItem](QQuickPaintedItem), employing software rasterization of the vector graphics. For alternative ways displaying Lottie files in Quick applications, utilizing graphics hardware rendering, see [lottietoqml](lottietoqml) and the [VectorImage](VectorImage) item.


---

*Built with QDoc's template engine.*
