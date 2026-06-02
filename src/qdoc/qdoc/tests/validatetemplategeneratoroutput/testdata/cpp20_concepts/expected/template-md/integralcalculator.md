[Cpp20Concepts](cpp20concepts-module.md)> IntegralCalculator

**Contents**

- [Public Functions](#public-functions)
- [Detailed Description](#details)
- [Member Function Documentation](#member-function-documentation)

# IntegralCalculator

class IntegralCalculator

A calculator demonstrating concept autolinking in requires clauses.

| Key | Value |
| --- | --- |
| Header | `concepts.h` |

- [List of all members, including inherited members](integralcalculator-members.md)


## Public Functions

| Member | Description |
| --- | --- |
| `void doubleValue(Integral auto value)` |  |
| `T processConcept(T value)` |  |
| `T sum(T a, T b)` |  |

## Detailed Description
[IntegralCalculator](integralcalculator.md) exercises the template-head and trailing requires forms against the documented [Integral](integral.md) concept. The rendered synopses should hyperlink `Integral` to its concept reference page.

## Member Function Documentation

<a id="doubleValue"></a>
### void doubleValue(Integral auto value)

Doubles _value_ using the constrained-auto syntax. Used as a regression check that the existing constrained-auto autolink path is preserved alongside the new requires-clause work.
<a id="processConcept"></a>
### T processConcept(T value)

Doubles _value_ using a trailing requires clause that names the Integral concept directly.
<a id="sum"></a>
### T sum(T a, T b)

Sums _a_ and _b_, constrained by the Integral concept.
Exercises the template-head requires form, which appears before the function declaration.

---

*Built with QDoc's template engine.*
