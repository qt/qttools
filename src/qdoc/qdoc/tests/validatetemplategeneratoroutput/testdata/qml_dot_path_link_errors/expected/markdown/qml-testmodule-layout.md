[TestModule](qml-testmodule-qmlmodule.md)> Layout

**Contents**

- [Enumerations](#enumerations)
- [Properties](#properties)
- [Detailed Description](#details)
- [Enumeration Documentation](#enumeration-documentation)
- [Property Documentation](#property-documentation)

# Layout QML Type

A QML type that fills the remaining vertical space.

| | |
| --- | --- |
| Import Statement | `import TestModule 1.0` |
| Inherited By | [Panel](qml-testmodule-panel.md)|

- [List of all members, including inherited members](qml-testmodule-layout-members.md)

## Detailed Description
QML dot-path links written without a genus:
- [Layout.fillHeight](qml-testmodule-layout.md#fillHeight-prop)
- [Layout.DragMode](qml-testmodule-layout.md#DragMode-enum)
- [Layout.values.integer](qml-testmodule-layout.md#values.integer-prop)
- [OtherModule.Layout](qml-othermodule-layout.md)
- [OtherModule.Layout.stretch](qml-othermodule-layout.md#stretch-prop)
- [OtherModule.Button.text](qml-othermodule-button.md#text-prop)
- [OtherModule.Button.press](qml-othermodule-button.md#press-method)

The same links with a forced QML genus:
- [Layout.fillHeight](qml-testmodule-layout.md#fillHeight-prop)
- [Layout.DragMode](qml-testmodule-layout.md#DragMode-enum)
- [OtherModule.Layout](qml-othermodule-layout.md)
- [OtherModule.Button.text](qml-othermodule-button.md#text-prop)

Bracket options precede a link target; brackets written after the target are ordinary text (and the module name autolinks):
- [Layout.fillHeight](qml-testmodule-layout.md#fillHeight-prop) [[TestModule](qml-testmodule-qmlmodule.md)]
- [Layout.values.integer](qml-testmodule-layout.md#values.integer-prop) [[TestModule](qml-testmodule-qmlmodule.md)]

A fragment into the documentation of a member of the competing type:
- [OtherModule.Layout#Details](qml-othermodule-layout.md#details-section)

Links that must fail:
- Layout.missingProperty
- Layout.fillHeight
- Layout.fillHeight#NoSection
- Layout.stretch [[TestModule](qml-testmodule-qmlmodule.md)]



## Enumerations

| Member | Description |
| --- | --- |
| `enum DragMode` | How the type reacts to drags. |

## Properties

| Member | Description |
| --- | --- |
| `fillHeight : double` | The fraction of the available height the type occupies. |
| `values.integer : int` | Integer values associated with the layout. |

## Enumeration Documentation

<a id="DragMode-enum"></a>
### enum DragMode

How the type reacts to drags.
| Constant | Description |
| --- | --- |
| `Layout.StaticDrag` | The type does not move.|
| `Layout.RelativeDrag` | The type moves relative to the drag.|

## Property Documentation

<a id="fillHeight-prop"></a>
### fillHeight : double

The fraction of the available height the type occupies.
# Usage

The property accepts values between 0 and 1.

<a id="values.integer-prop"></a>
### values.integer : int

Integer values associated with the layout.

---

*Built with QDoc's template engine.*
