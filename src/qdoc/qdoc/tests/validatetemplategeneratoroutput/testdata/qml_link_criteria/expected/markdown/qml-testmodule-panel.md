[TestModule](testmodule-qmlmodule.md)> Panel

**Contents**

- [Properties](#properties)
- [Detailed Description](#details)
- [Property Documentation](#property-documentation)

# Panel QML Type

A type using an import alias for shared QML lookup.

| | |
| --- | --- |
| Import Statement | `import TestModule` |
| Inherits | [Item](qml-testmodule-item.md)|

- [List of all members, including inherited members](qml-testmodule-panel-members.md)

## Detailed Description
- Alias-qualified type: [TM.Item](qml-testmodule-item.md)
- Regular alias: [TM.Item.name](qml-testmodule-item.md#name-prop)
- Attached alias: [TM.Item.name](qml-testmodule-item.md#name-attached-prop)
- Scoped attached alias: [TM.Item.name](qml-testmodule-item.md#name-attached-prop)
- Unique attached alias: [TM.Item.tag](qml-testmodule-item.md#tag-attached-prop)
- Alias property chain: [TM.Item.child.name](qml-testmodule-item.md#name-prop)
- Alias grouped property: [TM.Item.values.integer](qml-testmodule-item.md#values.integer-prop)
- Alias signature: [TM.Item.select(int, float)](qml-testmodule-item.md#select-method)
- Alias method chain: [TM.Item.child.select(int, float)](qml-testmodule-item.md#select-method)
- Alias fragment: [TM.Item.select(int, float)#Selection](qml-testmodule-item.md#selection)

Invalid alias references must fail:
- Invalid tail: TM.Item.name.missing
- Missing attached property: TM.Item.child
- Wrong signature: TM.Item.select(string)



## Properties

| Member | Description |
| --- | --- |
| `caption : string` | A property with alias links in its documentation. |

## Property Documentation

<a id="caption-prop"></a>
### caption : string

A property with alias links in its documentation.
Attached alias from a member: [TM.Item.name](qml-testmodule-item.md#name-attached-prop). Method alias from a member: [TM.Item.select(int, float)](qml-testmodule-item.md#select-method).
**See also** [TM.Item.child.name](qml-testmodule-item.md#name-prop) and [TM.Item.select(int, float)](qml-testmodule-item.md#select-method).


---

*Built with QDoc's template engine.*
