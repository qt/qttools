# LottieAnimation QML Type

A Lottie player for Qt.

| | |
| --- | --- |
| Import Statement | `import Qt.labs.lottieqt 1.0` |
| Inherits | [Item](qml-item.md)|

- [List of all members, including inherited members](qml-qt-labs-lottieqt-lottieanimation-members.md)

The [LottieAnimation](qml-qt-labs-lottieqt-lottieanimation.md) type shows Lottie format files.
[LottieAnimation](qml-qt-labs-lottieqt-lottieanimation.md) is used to load and render Lottie files exported from Adobe After Effects. Currently, only subset of the full Lottie specification is supported. Most notable deviations are:
- Only Shape layer supported
- Only integer frame-mode of a timeline supported (real frame numbers and time are rounded to the nearest integer)
- Expressions are not supported

For the full list of devations, please see see the [Limitations](qtlottieanimation-index.md) section.
## Example Usage

The following example shows a simple usage of the [LottieAnimation](qml-qt-labs-lottieqt-lottieanimation.md) type
```qml
LottieAnimation {
    loops: 2
    quality: LottieAnimation.MediumQuality
    source: "animation.json"
    autoPlay: false
    onStatusChanged: {
        if (status === LottieAnimation.Ready) {
            // any acvities needed before
            // playing starts go here
            gotoAndPlay(startFrame);
        }
    }
    onFinished: {
        console.log("Finished playing")
    }
}

```

> **Note:** Changing width or height of the element does not change the size of the animation within. Also, it is not possible to align the the content inside of a LottieAnimation element. To achieve this, position the animation inside e.g. an Item.


## Rendering Performance

Internally, the rendered frame data is cached to improve performance. You can control the memory usage by setting the QLOTTIE_RENDER_CACHE_SIZE environment variable (default value is 2).
You can monitor the rendering performance by turning on two logging categories:
- `qt.lottieqt.lottie.render` - Provides information how the animation is rendered
- `qt.lottieqt.lottie.render.thread` - Provides information how the rendering process proceeds.

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

## Property Documentation

<a id="autoPlay-prop"></a>
### autoPlay : bool

Defines whether the player will start playing animation automatically after the animation file has been loaded.
The default value is `true`.
<a id="direction-prop"></a>
### direction : enumeration

This property holds the direction of rendering.
| Constant | Description |
| --- | --- |
| `LottieAnimation.Forward` | Forward direction (Default)|
| `LottieAnimation.Reverse` | Reverse direction|

<a id="endFrame-prop"></a>
### endFrame : int

Frame number of the end of the animation. The value is available after the animation has been loaded and ready to play.
<a id="frameRate-prop"></a>
### frameRate : int

This property holds the frame rate value of the Lottie animation.
`frameRate` changes after the asset has been loaded. Changing the frame rate does not have effect before that, as the value defined in the asset overrides the value. To change the frame rate, you can write:
```qml
LottieAnimation {
    source: "animation.json"
    onStatusChanged: {
        if (status === LottieAnimation.Ready)
            frameRate = 60;
    }
}

```

<a id="loops-prop"></a>
### loops : int

This property holds the number of loops the player will repeat. The value `LottieAnimation.Infinite` means that the the player repeats the animation continuously.
The default value is `1`.
<a id="quality-prop"></a>
### quality : enumeration

Speficies the rendering quality of the lottie player. If `LowQuality` is selected the rendering will happen into a frame buffer object, whereas with other options, the rendering will be done onto `QImage` (which in turn will be rendered on the screen).
| Constant | Description |
| --- | --- |
| `LottieAnimation.LowQuality` | Antialiasing or a smooth pixmap transformation algorithm are not used|
| `LottieAnimation.MediumQuality` | Smooth pixmap transformation algorithm is used but no antialiasing (Default)|
| `LottieAnimation.HighQuality` | Antialiasing and a smooth pixmap tranformation algorithm are both used|

<a id="source-prop"></a>
### source : url

The source of the Lottie asset that [LottieAnimation](qml-qt-labs-lottieqt-lottieanimation.md) plays.
[LottieAnimation](qml-qt-labs-lottieqt-lottieanimation.md) can handle any URL scheme supported by Qt. The URL may be absolute, or relative to the URL of the component.
Setting the source property starts loading the animation asynchronously. To monitor progress of loading, connect to the [status](qml-qt-labs-lottieqt-lottieanimation.md#status-prop) change signal.
<a id="startFrame-prop"></a>
### startFrame : int

Frame number of the start of the animation. The value is available after the animation has been loaded and ready to play.
<a id="status-prop"></a>
### status : enumeration

This property holds the current status of the [LottieAnimation](qml-qt-labs-lottieqt-lottieanimation.md) element.
| Constant | Description |
| --- | --- |
| `LottieAnimation.Null` | An initial value that is used when the source is not defined (Default)|
| `LottieAnimation.Loading` | The player is loading a Lottie file|
| `LottieAnimation.Ready` | Loading has finished successfully and the player is ready to play the animation|
| `LottieAnimation.Error` | An error occurred while loading the animation|

For example, you could implement `onStatusChanged` signal handler to monitor progress of loading an animation as follows:
```qml
LottieAnimation {
    source: "animation.json"
    autoPlay: false
    onStatusChanged: {
        if (status === LottieAnimation.Ready)
            start();
    }
}

```

## Signal Documentation

<a id="finished-signal"></a>
### finished()

This signal is emitted when the player has finished playing. In case of looping, the signal is emitted when the last loop has been finished.
## Method Documentation

<a id="getDuration-method"></a>
### double getDuration(bool inFrames)

Returns the duration of the currently playing asset.
If a given _inFrames_ is `true`, the return value is the duration in number of frames. Otherwise, returns the duration in seconds.
<a id="gotoAndPlay-method"></a>
### void gotoAndPlay(int frame)

Plays the asset from the given _frame_.
<a id="gotoAndPlay-method-1"></a>
### bool gotoAndPlay(string frameMarker)

Plays the asset from the frame that has a marker with the given _frameMarker_. Returns `true` if the frameMarker was found, `false` otherwise.
<a id="gotoAndStop-method"></a>
### void gotoAndStop(int frame)

Moves the playhead to the given _frame_ and stops.
<a id="gotoAndStop-method-1"></a>
### bool gotoAndStop(string frameMarker)

Moves the playhead to the given marker and stops. Returns `true` if _frameMarker_ was found, `false` otherwise.
<a id="pause-method"></a>
### void pause()

Pauses the playback.
<a id="play-method"></a>
### void play()

Starts or continues playing from the current position.
<a id="start-method"></a>
### void start()

Starts playing the animation from the beginning.
<a id="stop-method"></a>
### void stop()

Stops the playback and returns to [startFrame](qml-qt-labs-lottieqt-lottieanimation.md#startFrame-prop).
<a id="togglePause-method"></a>
### void togglePause()

Toggles the status of player between playing and paused states.
---

*Built with QDoc's template engine.*
