**Contents**

- [Overview](#overview)
- [Usage](#usage)

# lottietoqml

A tool that converts a Lottie document to a QML file.

`lottietoqml` is a command line tool shipped with Qt that converts a Lottie document to a QML file. This QML file can then be used as a component in Qt Quick applications.
You can use `lottietoqml` directly, or via the [qt_target_qml_from_lottie](qt-target-qml-from-lottie.md) CMake command.
For a demonstration of how `lottietoqml` can be used in an application, see the [lottietoqml Example](qtlottie-lottietoqml-example.md).
## Overview

The `lottietoqml` tool will convert a Lottie file to a QML file which uses Qt Quick primitives. Since the Qt Quick is hardware-accelerated, rendering the animation may perform and consume less CPU time than using the built-in software renderer in [Qt Lottie Animation](qtlottieanimation-index.md). In addition, Qt Quick supports scalable vector graphics, so the resulting item will be smoothly transformable as far as this is possible.

## Usage

The basic usage of `lottietoqml` is to provide an input file and an output file: `lottietoqml input.json output.qml`. This will read the `input.json` file and convert it into the corresponding Qt Quick scene in `output.qml`, which can then be used as part of a Qt Quick application.
In addition, it supports the following options:
| Option | Description | 
|--- |--- |
| –copyright-statement <string> | Adds <string> as a comment at the beginning of the generated file. | 
| -c, –curve-renderer | Enables the curve renderer backend for [Qt Quick Shapes](https://doc.qt.io/qt-6/qtquick-shapes-qmlmodule.html). This enables smooth, antialiased shapes in the scene without multi-sampling, but at some extra cost. | 
| -a, --asynchronous-shapes | Enables the {QtQuick.Shapes::Shape::asynchronous}{asynchronous} mode on all [Shape](https://doc.qt.io/qt-6/qml-qtquick-shapes-shape.html) elements in the generated scene. This may improve CPU utilization and responsiveness. | 
| -p, –optimize-paths | Enables optimization of paths before committing them to the QML file, potentially making them faster to load and render later. | 
| –outline-stroke-mode | Stroke the outline (contour) of the filled shape instead of the original path. | 
| -t, –type-name <string> | In place of [Shape](https://doc.qt.io/qt-6/qml-qtquick-shapes-shape.html), the output will use the type name <string> instead. This is enables using a custom item to override the default behavior of [Shape](https://doc.qt.io/qt-6/qml-qtquick-shapes-shape.html) items. | 
| -v, –view | Display a preview of the Qt Quick item as it will be generated. | 




---

*Built with QDoc's template engine.*
