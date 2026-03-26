# Qt Lottie Animation

Display Lottie format vector graphics animations.

_Qt Lottie Animation_ provides QML APIs for rendering graphics and animations in Lottie format files. These are typically exported from the [Lottie](https://lottiefiles.com/plugins/after-effects) plugins for Adobe After Effects.
The [LottieAnimation](qml-qt-labs-lottieqt-lottieanimation.md) item renders animations into an intermediate buffer using the [QPainter](https://doc.qt.io/qt-6/qpainter.html) software renderer. This may impose some performance restrictions on the sizes of illustrations, as well as on the target hardware. As an alternative, a tool called [lottietoqml](lottietoqml.md) is included. This tool converts Lottie animations to QML. As a result, they can be rendered using the hardware-accelerated renderer in Qt Quick.
If the Lottie files are known at compile time, you can also convert them to QML using the [lottietoqml](lottietoqml.md) tool, or the [qt_target_qml_from_lottie](qt-target-qml-from-lottie.md) CMake command that wraps the tool.
Finally, the [VectorImage](https://doc.qt.io/qt-6/qml-qtquick-vectorimage.html) Qt Quick item can load Lottie format files directly. It converts them to QML at runtime using the [lottietoqml](lottietoqml.md) functionality in a plugin.
## Getting Started

Import the types using the the following statement:
```cpp
import Qt.labs.lottieqt

```


## Limitations

The implementation follows the [master specification for lottie](https://lottie.github.io/lottie-spec/1.0/). Deviations in exported Lottie files compared to the specification are taken into account as much as possible when differences are noticed.

### General limitations

- expressions are not supported
- the timeline only supports frame-mode, not time-mode

> **Warning:** The input files for Qt Lottie Animation are assumed to be trusted content. Application developers are advised to never pass in data from untrusted sources.


### Animation level limitations

The following properties are not supported:
- `assets` - reusable text and images
- `chars` text


### Layers

The following properties are not supported:
- `ao` (auto-orientation)
- `bm` (blend mode)
- `maskProperties` (masks)
- `sr` (time stretch)


### Shapes

Some more rarely used elements and shapes are not supported, including:
- the `gstroke` (gradient stroke) element
- Nested Repeater shapes

Also note that the behavior when using multiple active trim paths (e.g. trim paths in nested groups) is unpredictable.

### Effects

The only supported effects are `Slide` and `Layer Fill`.

### Examples

- The [qtlottieviewer Example](qtlottie-qtlottieviewer-example.md) demonstrates how to use the [LottieAnimation](qml-qt-labs-lottieqt-lottieanimation.md) item to load, display and control a Lottie file in a Qt Quick application.
- The [lottietoqml Example](qtlottie-lottietoqml-example.md) demonstrates how the [lottietoqml](lottietoqml.md) tool can be used to convert Lottie files into Qt Quick scenes and load these in an application.


## Licenses

Qt Lottie Animation is available under commercial licenses from [The Qt Company](https://www.qt.io/company). In addition, it is available under the [GNU General Public License, version 3](https://www.gnu.org/licenses/gpl-3.0.html). See [Qt Licensing](https://doc.qt.io/qt-6/licensing.html) for further details.

## Reference

- [QML Types](qt-labs-lottieqt-qmlmodule.md)
- [lottietoqml Tool](lottietoqml.md)
- [qt_target_qml_from_lottie CMake Command](qt-target-qml-from-lottie.md)


## Related Information

- [Lottie on GitHub](https://github.com/airbnb/lottie-web)



---

*Built with QDoc's template engine.*
