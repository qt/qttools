[TestModule](testmodule-qmlmodule.md)> Panel

**Contents**

- [Detailed Description](#details)

# Panel QML Type

A QML type documented from a QML source file.

| | |
| --- | --- |
| Import Statement | `import TestModule 1.0` |
| Inherits | [Layout](qml-testmodule-layout.md)|

- [List of all members, including inherited members](qml-testmodule-panel-members.md)

## Detailed Description
The file imports [OtherModule](othermodule-qmlmodule.md) under the alias OM. Import-qualified and unqualified dot-path links; unqualified names resolve against the type's own module:
- [OtherModule.Layout](qml-othermodule-layout.md)
- [OtherModule.Button.text](qml-othermodule-button.md#text-prop)
- [Layout.fillHeight](qml-testmodule-layout.md#fillHeight-prop)
- [Layout.values.integer](qml-testmodule-layout.md#values.integer-prop)

Alias-qualified dot-path links, read through the import alias:
- [OM.Layout](qml-othermodule-layout.md)
- [OM.Button.text](qml-othermodule-button.md#text-prop)




---

*Built with QDoc's template engine.*
