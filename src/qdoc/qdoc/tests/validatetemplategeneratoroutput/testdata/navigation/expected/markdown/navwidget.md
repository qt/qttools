[Home](navigation-test-home.md)> [Landing](navigation-landing.md)> [C++ Classes](cpp-classes.md)> [TestNavigation](testnavigation-module.md)> NavWidget

**Contents**

- [Public Functions](#public-functions)
- [Detailed Description](#details)
- [Basic Usage](#basic-usage)
- [Configuration](#configuration)
- [Advanced Options](#advanced-options)
- [Thread Safety](#thread-safety)
- [Performance](#performance)
- [Benchmarks](#benchmarks)
- [Member Function Documentation](#member-function-documentation)

# NavWidget

class NavWidget

A widget for navigation testing.

| Key | Value |
| --- | --- |
| Header | `classpage.h` |

- [List of all members, including inherited members](navwidget-members.md)


## Public Functions

| Member | Description |
| --- | --- |
| `NavWidget()` |  |
| `void navigate(const QString &target)` |  |

## Detailed Description
The [NavWidget](navwidget.md) class demonstrates breadcrumb chains for C++ class pages and sidebar TOC rendering with multiple section levels.
### Basic Usage

Create a [NavWidget](navwidget.md) and call [navigate()](navwidget.md#navigate).

#### Configuration

Configuration is straightforward.

##### Advanced Options

For advanced use cases, subclass [NavWidget](navwidget.md).

### Thread Safety

[NavWidget](navwidget.md) is reentrant but not thread-safe.

### Performance

Performance depends on navigation target complexity.

#### Benchmarks

Benchmarks show sub-millisecond navigation times.


## Member Function Documentation

<a id="NavWidget"></a>
### NavWidget()

Constructs a [NavWidget](navwidget.md).
<a id="navigate"></a>
### void navigate(const QString &target)

Navigates to the specified _target_.

---

*Built with QDoc's template engine.*
