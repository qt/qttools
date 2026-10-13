[QML Link Criteria](testmodule-qmlmodule.md)> Item

**Contents**

- [Properties](#properties)
- [Attached Properties](#attached-properties)
- [Methods](#methods)
- [Detailed Description](#details)
- [Property Documentation](#property-documentation)
- [Attached Property Documentation](#attached-property-documentation)
- [Method Documentation](#method-documentation)

# Item QML Type

A type with regular and attached properties of the same name.

| | |
| --- | --- |
| Import Statement | `import TestModule` |
| Inherited By | [Panel](qml-testmodule-panel.md)|

- [List of all members, including inherited members](qml-testmodule-item-members.md)

## Detailed Description
- Regular C++ notation: [Item::name](qml-testmodule-item.md#name-prop)
- Attached C++ notation: [Item::name](qml-testmodule-item.md#name-attached-prop)
- Regular QML notation: [Item.name](qml-testmodule-item.md#name-prop)
- Attached QML notation: [Item.name](qml-testmodule-item.md#name-attached-prop)
- Qualified attached property: [TestModule.Item.name](qml-testmodule-item.md#name-attached-prop)
- Unique attached property: [Item.tag](qml-testmodule-item.md#tag-attached-prop)
- Scoped attached property: [Item.name](qml-testmodule-item.md#name-attached-prop)
- Property chain: [Item.child.name](qml-testmodule-item.md#name-prop)
- Grouped property: [Item.values.integer](qml-testmodule-item.md#values.integer-prop)
- Group after dereferencing: [Item.child.values.integer](qml-testmodule-item.md#values.integer-prop)
- Integer signature: [Item.select(int, int)](qml-testmodule-item.md#select-method-1)
- Floating signature: [Item.select(int, float)](qml-testmodule-item.md#select-method)
- Method after dereferencing: [Item.child.select(int, float)](qml-testmodule-item.md#select-method)
- Method fragment: [Item.select(int, float)#Selection](qml-testmodule-item.md#selection)
- Scoped method: [Item.select(int, float)](qml-testmodule-item.md#select-method)

Invalid paths must fail:
- Missing attached property: Item.child
- Invalid property tail: Item.name.missing
- Invalid method tail: Item.name.missing()
- Missing fragment: Item.select(int, float)#Missing
- Wrong signature: Item.select(string)



## Properties

| Member | Description |
| --- | --- |
| `child : Item` | An object-typed property. |
| `name : string` | A regular name. |
| `values.integer : int` | A grouped primitive property. |

## Attached Properties

| Member | Description |
| --- | --- |
| `name : string` | An attached name declared before the regular name. |
| `tag : string` | An attached property without a regular namesake. |

## Methods

| Member | Description |
| --- | --- |
| `void select(int start, float length)` | A floating-point selection. |
| `void select(int start, int length)` | An integer selection. |

## Property Documentation

<a id="child-prop"></a>
### child : Item

An object-typed property.
<a id="name-prop"></a>
### name : string

A regular name.
The attached name is [name](qml-testmodule-item.md#name-attached-prop).
<a id="values.integer-prop"></a>
### values.integer : int

A grouped primitive property.
## Attached Property Documentation

<a id="name-attached-prop"></a>
### name : string

An attached name declared before the regular name.
The regular name is [name](qml-testmodule-item.md#name-prop).
<a id="tag-attached-prop"></a>
### tag : string

An attached property without a regular namesake.
## Method Documentation

<a id="select-method"></a>
### void select(int start, float length)

A floating-point selection.
# Selection

Details of floating-point selection.

<a id="select-method-1"></a>
### void select(int start, int length)

An integer selection.

---

*Built with QDoc's template engine.*
