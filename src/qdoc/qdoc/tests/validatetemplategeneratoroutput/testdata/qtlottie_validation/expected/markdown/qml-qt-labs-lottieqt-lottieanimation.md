# LottieAnimation QML Type

A Lottie player for Qt.

| | |
| --- | --- |
| Import Statement | `import Qt.labs.lottieqt 1.0` |
| Inherits | Item|

- [List of all members, including inherited members](qml-qt-labs-lottieqt-lottieanimation-members.md)

The [LottieAnimation](qml-qt-labs-lottieqt-lottieanimation.md) type shows Lottie format files.
[LottieAnimation](qml-qt-labs-lottieqt-lottieanimation.md) is used to load and render Lottie files exported from Adobe After Effects. Currently, only subset of the full Lottie specification is supported. Most notable deviations are:
- Only Shape layer supported
- Only integer frame-mode of a timeline supported (real frame numbers and time are rounded to the nearest integer)
- Expressions are not supported

For the full list of devations, please see see the [Limitations](qtlottieanimation-index.md) section.
# Example Usage

The following example shows a simple usage of the [LottieAnimation](qml-qt-labs-lottieqt-lottieanimation.md) type
```qml
<@type>LottieAnimation</@type> {
    <@name>loops</@name>: <@number>2</@number>
    <@name>quality</@name>: <@name>LottieAnimation</@name>.<@name>MediumQuality</@name>
    <@name>source</@name>: <@string>&quot;animation.json&quot;</@string>
    <@name>autoPlay</@name>: <@number>false</@number>
    <@name>onStatusChanged</@name>: {
        <@keyword>if</@keyword> (<@name>status</@name> <@op>===</@op> <@name>LottieAnimation</@name>.<@name>Ready</@name>) {
            <@comment>// any acvities needed before</@comment>
            <@comment>// playing starts go here</@comment>
            <@name>gotoAndPlay</@name>(<@name>startFrame</@name>);
        }
    }
    <@name>onFinished</@name>: {
        <@name>console</@name>.<@name>log</@name>(<@string>&quot;Finished playing&quot;</@string>)
    }
}

```

> **Note:** Changing width or height of the element does not change the size of the animation within. Also, it is not possible to align the the content inside of a LottieAnimation element. To achieve this, position the animation inside e.g. an Item.


# Rendering Performance

Internally, the rendered frame data is cached to improve performance. You can control the memory usage by setting the [QLOTTIE_RENDER_CACHE_SIZE](QLOTTIE_RENDER_CACHE_SIZE) environment variable (default value is 2).
You can monitor the rendering performance by turning on two logging categories:
- `qt<@op>.</@op>lottieqt<@op>.</@op>lottie<@op>.</@op>render` - Provides information how the animation is rendered
- `qt<@op>.</@op>lottieqt<@op>.</@op>lottie<@op>.</@op>render<@op>.</@op>thread` - Provides information how the rendering process proceeds.

Specifically, you can monitor does the frame cache gets constantly full, or does the rendering process have to wait for frames to become ready. The first case implies that the animation is too complex, and the rendering cannot keep up the pace. Try making the animation simpler, or optimize the QML scene.



## Properties

| Member | Description |
| --- | --- |
| `autoPlay : bool` |  |
| `direction : enumeration` |  |
| `endFrame : int` |  |
| `frameRate : int` |  |
| `loops : int` |  |
| `quality : enumeration` |  |
| `source : url` |  |
| `startFrame : int` |  |
| `status : enumeration` |  |

## Signals

| Member | Description |
| --- | --- |
| `finished()` |  |

## Methods

| Member | Description |
| --- | --- |
| `double getDuration(bool inFrames)` |  |
| `void gotoAndPlay(int frame)` |  |
| `bool gotoAndPlay(string frameMarker)` |  |
| `void gotoAndStop(int frame)` |  |
| `bool gotoAndStop(string frameMarker)` |  |
| `void pause()` |  |
| `void play()` |  |
| `void start()` |  |
| `void stop()` |  |
| `void togglePause()` |  |
---

*Built with QDoc's template engine.*
