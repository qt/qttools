[GroupQml](groupqml-qmlmodule.md)> Trio

**Contents**

- [Properties](#properties)
- [Methods](#methods)
- [Detailed Description](#details)
- [Property Documentation](#property-documentation)
- [Method Documentation](#method-documentation)

# Trio QML Type

A QML type that carries a shared property group.

| | |
| --- | --- |
| Import Statement | `import GroupQml 1.0` |

- [List of all members, including inherited members](qml-groupqml-trio-members.md)

## Detailed Description
The alpha, beta, and gamma properties share a comment; solo stands on its own. Other groups exercise status sentences.


## Properties

| Member | Description |
| --- | --- |
| `alpha : double` |  |
| `beta : double` |  |
| `font.family : string` |  |
| `font.pixelSize : int` |  |
| `gamma : double` |  |
| `solo : double` |  |

## Methods

| Member | Description |
| --- | --- |
| `void close()` |  |
| `void open()` |  |

## Property Documentation

<a id="alpha-prop"></a>
### alpha : double

<a id="beta-prop"></a>
### beta : double

<a id="gamma-prop"></a>
### gamma : double

These three properties do the same work together. The shared comment that documents them refers to the ungrouped property [GroupQml::Trio::solo](qml-groupqml-trio.md#solo-prop), to the named property group [GroupQml::Trio::font](qml-groupqml-trio.md#font-prop), and to a target that does not exist: BrokenTarget
**See also** [GroupQml::Trio::solo](qml-groupqml-trio.md#solo-prop).

<a id="font-prop"></a>
<a id="font.family-prop"></a>
### font.family : string

<a id="font.pixelSize-prop"></a>
### font.pixelSize : int

This property group is scheduled for deprecation in version 7.0.

The font settings form one named property group: [GroupQml::Trio::font.family](qml-groupqml-trio.md#font.family-prop) and [GroupQml::Trio::font.pixelSize](qml-groupqml-trio.md#font.pixelSize-prop).
**See also** [GroupQml::Trio::solo](qml-groupqml-trio.md#solo-prop).

<a id="solo-prop"></a>
### solo : double

The ungrouped property stands on its own.
It links back to one of the grouped properties: [GroupQml::Trio::alpha](qml-groupqml-trio.md#alpha-prop)
This qml property was introduced in Qt 6.1.

**See also** [GroupQml::Trio::alpha](qml-groupqml-trio.md#alpha-prop).

## Method Documentation

<a id="open-method"></a>
### void open()

<a id="close-method"></a>
### void close()

These methods are scheduled for deprecation in version 7.0.

Opens or closes the trio with one shared description.
**See also** [GroupQml::Trio::solo](qml-groupqml-trio.md#solo-prop).


---

*Built with QDoc's template engine.*
