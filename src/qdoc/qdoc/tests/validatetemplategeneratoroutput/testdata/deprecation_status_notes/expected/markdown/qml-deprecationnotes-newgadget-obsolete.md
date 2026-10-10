# Obsolete Members for NewGadget

**The following members of NewGadget are deprecated.** They're provided to keep old source code working, but we strongly advise against using them in new code.

[NewGadget](qml-deprecationnotes-newgadget.md)reference page


## Properties

| Member | Description |
| --- | --- |
| `serialNumber : string` | The gadget serial number. |

## Signals

| Member | Description |
| --- | --- |
| `void broke()` | Emitted when the gadget breaks. |

## Methods

| Member | Description |
| --- | --- |
| `void close()` | Closes the gadget. |

## Property Documentation

<a id="serialNumber-prop"></a>
### serialNumber : string

This property is deprecated since DeprecationNotes 6.5. We strongly advise against using it in new code.

The gadget serial number.
The serial number the gadget reports. New code should use the platform-assigned identifier instead.
## Signal Documentation

<a id="broke-signal"></a>
### void broke()

This signal is deprecated since DeprecationNotes 6.10. We strongly advise against using it in new code.

Emitted when the gadget breaks.
Notifies listeners that the gadget broke. New code should track the status property instead.
## Method Documentation

<a id="close-method"></a>
### void close()

This method is deprecated since DeprecationNotes 6.5. We strongly advise against using it in new code.

Closes the gadget.

---

*Built with QDoc's template engine.*
