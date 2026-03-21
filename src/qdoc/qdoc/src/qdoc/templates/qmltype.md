{#-
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
-#}
# {{ fullTitle }}

{% if default(brief, "") != "" %}
{{ brief }}

{% endif %}
{% if hasQmlType %}
| | |
| --- | --- |
{% if existsIn(qmlType, "importStatement") %}| Import Statement | `{{ qmlType.importStatement }}` |
{% endif %}{% if existsIn(qmlType, "inherits") %}| Inherits | {% if qmlType.inherits.href != "" %}[{{ qmlType.inherits.name }}]({{ qmlType.inherits.href }}){% else %}{{ qmlType.inherits.name }}{% endif %} |
{% endif %}{% if existsIn(qmlType, "inheritedBy") %}| Inherited By | {% for sub in qmlType.inheritedBy %}{% if not loop.is_first %}, {% endif %}{% if sub.href != "" %}[{{ sub.name }}]({{ sub.href }}){% else %}{{ sub.name }}{% endif %}{% endfor %} |
{% endif %}{% if existsIn(qmlType, "nativeType") %}| In C++ | {% if qmlType.nativeType.href != "" %}[{{ qmlType.nativeType.name }}]({{ qmlType.nativeType.href }}){% else %}{{ qmlType.nativeType.name }}{% endif %} |
{% endif %}{% if qmlType.isSingleton %}| Status | Singleton |
{% endif %}
{% endif %}

{% if hasQmlType %}{% if qmlType.isSingleton %}
> **Note:** This type is a QML singleton. The type isn't creatable and only a single instance exists.

{% endif %}{% endif %}
{% include "partials/md/content_blocks.md" %}

{% if length(sections) > 0 %}
{% for section in sections %}

## {{ section.title }}

| Member | Description |
| --- | --- |
{% for member in section.members %}
| `{{ escape_md_table(member.signature) }}` | {{ escape_md_table(default(member.brief, "")) }} |
{% endfor %}
{% if existsIn(section, "reimplementedMembers") %}

### Reimplemented {{ section.plural }}

| Member | Description |
| --- | --- |
{% for member in section.reimplementedMembers %}
| `{{ escape_md_table(member.signature) }}` | {{ escape_md_table(default(member.brief, "")) }} |
{% endfor %}
{% endif %}
{% if existsIn(section, "inheritedMembers") %}
{% for inherited in section.inheritedMembers %}
- {{ inherited.count }} {{ section.plural }} inherited from [{{ inherited.className }}]({{ inherited.href }})
{% endfor %}
{% endif %}
{% endfor %}
{% endif %}
---

*Built with QDoc's template engine.*
