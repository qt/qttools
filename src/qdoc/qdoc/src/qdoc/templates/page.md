{#-
Copyright (C) 2025 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
-#}
# {{ fullTitle }}

{% if default(brief, "") != "" %}
{{ brief }}

{% endif %}
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
- {{ inherited.count }} {{ section.plural }} inherited from {% if inherited.href != "" %}[{{ inherited.className }}]({{ inherited.href }}){% else %}{{ inherited.className }}{% endif %}
{% endfor %}
{% endif %}
{% endfor %}
{% endif %}
{% include "partials/md/footer.md" %}
