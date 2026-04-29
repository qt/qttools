**Contents**

- [Group Members in Section](#group-members-in-section)
- [Compact Classes in Section](#compact-classes-in-section)

# Embedded Catalog

Catalog directives embedded inside section blocks.

This page wraps catalog directives in section blocks. The template generator's section dispatch must recurse correctly so every embedded catalog renders as a populated table or list.
## Group Members in Section

Surrounding prose appears before the catalog directive so the section block carries multiple children, with the catalog as one of them.


| Name | Description |
| --- | --- |
| [AlphaWidget](alphawidget.md) | First widget documented by the catalog fixture |
| [BetaWidget](betawidget.md) | Second widget documented by the catalog fixture |
| [GammaWidget](gammawidget.md) | Third widget documented by the catalog fixture |


Trailing prose verifies that content after the catalog still renders normally, anchoring the catalog as a non-terminal child of the section block.

## Compact Classes in Section

A second section confirms that adjacent sections each receive their own catalog dispatch.
- [BetaWidget](betawidget.md)





---

*Built with QDoc's template engine.*
