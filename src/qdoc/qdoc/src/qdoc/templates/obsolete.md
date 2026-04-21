{#-
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
-#}
# {{ title }}

**The following members of {{ default(typeName, "") }} are deprecated.** They're provided to keep old source code working, but we strongly advise against using them in new code.

{% if typeHref != "" %}[{{ typeName }}]({{ typeHref }}){% else %}{{ typeName }}{% endif %} reference page

{% if length(sections) > 0 %}
{% for section in sections %}

## {{ section.title }}

| Member | Description |
| --- | --- |
{% for member in section.members %}
| `{{ escape_md_table(member.signature) }}` | {{ escape_md_table(default(member.brief, "")) }} |
{% endfor %}
{% endfor %}
{% endif %}

{% if length(detailSections) > 0 %}
{% for section in detailSections %}
{% if length(section.members) > 0 %}
## {{ section.title }}

{% for member in section.members %}
{% include "partials/md/member_detail.md" %}
{% endfor %}
{% endif %}
{% endfor %}
{% endif %}
{% include "partials/md/footer.md" %}
