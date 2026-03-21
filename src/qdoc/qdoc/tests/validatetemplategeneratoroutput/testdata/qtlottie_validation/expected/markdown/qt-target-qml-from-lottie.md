# qt_target_qml_from_lottie

Generates QML code based on a Lottie file.

The command is defined in the `LottieTools` component of the `Qt6` package, which can be loaded like this:
```cpp
find_package(Qt6 REQUIRED COMPONENTS LottieTools)

```

This command was introduced in Qt 6.11.
# Synopsis

```cpp
qt_target_qml_from_lottie(target
                          <@op>[</@op>CURVE_RENDERER<@op>]</@op>
                          <@op>[</@op>ASYNCHRONOUS_SHAPES<@op>]</@op>
                          <@op>[</@op>OPTIMIZE_PATHS<@op>]</@op>
                          <@op>[</@op>OUTLINE_STROKE_MODE<@op>]</@op>
                          <@op>[</@op>TYPE_NAME <@string>&quot;MyShapeName&quot;</@string><@op>]</@op>
                          <@op>[</@op>COPYRIGHT_STATEMENT <@string>&quot;Copyright © Company1&quot;</@string><@op>]</@op>
                          FILES file1<@op>.</@op>json <@op>[</@op>file2<@op>.</@op>json <@op>.</@op><@op>.</@op><@op>.</@op><@op>]</@op>
                          OUTPUTS File1<@op>.</@op>qml <@op>[</@op>File2<@op>.</@op>qml <@op>.</@op><@op>.</@op><@op>.</@op><@op>]</@op>
                          )

```

If [versionless commands](https://doc.qt.io/qt-6/cmake-qt5-and-qt6-compatibility.html#versionless-commands) are disabled, use `qt6_target_qml_from_lottie()` instead. It supports the same set of arguments as this command.
Example:
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


# Description

[qt_target_qml_from_lottie()](qt_target_qml_from_lottie()) creates the build steps to run [lottietoqml](lottietoqml.md) on the list of Lottie images in `FILES`. The names of the generated files should be specified in `OUTPUTS`. The length of this list must be the same as `FILES`, and the first element corresponds to the first element in `FILES` and so forth. The names provided in `OUTPUTS` will be the names of the QML types as they appear in the module. These can in turn be instantiated in the application code.
The generated QML files will be added to the QML module of `target`. You must use [qt_add_qml_module()](https://doc.qt.io/qt-6/qt-add-qml-module.html) to define a module for the `target` first.
Optionally, a `COPYRIGHT_STATEMENT` argument can be provided to insert copyright information into the generated files.
> **Note:** Certain characters must be escaped for command line use or you may see build errors on some platforms.

`TYPE_NAME` is also optional, and can be used to replace all instances of the [Shape](https://doc.qt.io/qt-6/qml-qtquick-shapes-shape.html) type in the generated file with a custom type. This can be useful to make general customizations to all the shapes in the provided Lottie files. The `TYPE_NAME` should refer to a QML type which is available in the `target` QML module.
The options `CURVE_RENDERER`, `ASYNCHRONOUS_SHAPES`, `OPTIMIZE_PATHS` and `OUTLINE_STROKE_MODE` correspond to the `<@op>-</@op><@op>-</@op>curve<@op>-</@op>renderer`, `<@op>-</@op><@op>-</@op>asynchronous<@op>-</@op>shapes`, `<@op>-</@op><@op>-</@op>optimize<@op>-</@op>paths` and `<@op>-</@op><@op>-</@op>outline<@op>-</@op>stroke<@op>-</@op>mode` in [lottietoqml](lottietoqml.md) respectively.


---

*Built with QDoc's template engine.*
