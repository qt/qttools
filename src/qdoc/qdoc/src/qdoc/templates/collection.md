{#-
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
-#}
# {{ fullTitle }}

{% if default(brief, "") != "" %}
{{ brief }}

{% endif %}
{% if hasCollection %}
{% if existsIn(collection, "state") %}
**Status:** {{ collection.state }}

{% endif %}
{% if collection.isModule %}
| | |
| --- | --- |
{% if collection.cmakePackage %}| CMake | `find_package({{ collection.cmakePackage }} REQUIRED COMPONENTS {{ collection.cmakeComponent }})` `target_link_libraries(mytarget PRIVATE {{ collection.cmakeTargetItem }})` |
{% endif %}{% if collection.qtVariable %}| qmake | `QT += {{ collection.qtVariable }}` |
{% endif %}
{% endif %}
{% endif %}

{% include "partials/md/content_blocks.md" %}

{% if hasCollection %}{% if not collection.noAutoList %}
{% if collection.isModule %}
{% if length(collection.namespaces) > 0 %}

## Namespaces

| Name | Description |
| --- | --- |
{% for entry in collection.namespaces %}
| {% if entry.href != "" %}[{{ entry.name }}]({{ entry.href }}){% else %}{{ entry.name }}{% endif %} | {{ escape_md_table(entry.brief) }} |
{% endfor %}
{% endif %}
{% if length(collection.classes) > 0 %}

## Classes

| Name | Description |
| --- | --- |
{% for entry in collection.classes %}
| {% if entry.href != "" %}[{{ entry.name }}]({{ entry.href }}){% else %}{{ entry.name }}{% endif %} | {{ escape_md_table(entry.brief) }} |
{% endfor %}
{% endif %}
{% else %}
{% if length(collection.members) > 0 %}

| Name | Description |
| --- | --- |
{% for entry in collection.members %}
| {% if entry.href != "" %}[{{ entry.name }}]({{ entry.href }}){% else %}{{ entry.name }}{% endif %} | {{ escape_md_table(entry.brief) }} |
{% endfor %}
{% endif %}
{% endif %}
{% endif %}{% endif %}
---

*Built with QDoc's template engine.*
