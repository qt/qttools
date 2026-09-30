{#-
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

Regression-test template: deliberately accesses collection fields that a
module with no CMake or qmake information does not carry. QDoc must report
the render failure, discard this page, and continue rendering later pages.
-#}
# {{ fullTitle }}

{% if default(brief, "") != "" %}
{{ brief }}

{% endif %}
{% if hasCollection and collection.isModule %}
{% if collection.cmakePackage %}**CMake:** `find_package({{ collection.cmakePackage }} REQUIRED COMPONENTS {{ collection.cmakeComponent }})` `target_link_libraries(mytarget PRIVATE {{ collection.cmakeTargetItem }})`

{% endif %}
{% if collection.qtVariable %}**qmake:** `QT += {{ collection.qtVariable }}`

{% endif %}
{% endif %}
{% if hasCollection and not collection.noAutoList %}
{% if collection.isModule %}
{% if length(collection.namespaces) > 0 %}

## Namespaces

{% for entry in collection.namespaces %}
- {% if entry.href != "" %}[{{ entry.name }}]({{ entry.href }}){% else %}{{ entry.name }}{% endif %}
{% endfor %}

{% endif %}
{% if length(collection.classes) > 0 %}

## Classes

{% for entry in collection.classes %}
- {% if entry.href != "" %}[{{ entry.name }}]({{ entry.href }}){% else %}{{ entry.name }}{% endif %}
{% endfor %}

{% endif %}
{% endif %}
{% endif %}
