{#-
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
-#}
{% include "partials/md/nav.md" %}
# {{ title }}

This is the complete list of members for {% if typeHref != "" %}[{{ typeName }}]({{ typeHref }}){% else %}{{ typeName }}{% endif %}, including inherited members.

{% if isQmlType %}
{% for group in memberGroups %}
{% if group.typeName != "" %}
The following members are inherited from {% if group.typeHref != "" %}[{{ group.typeName }}]({{ group.typeHref }}){% else %}{{ group.typeName }}{% endif %}.

{% endif %}
{% for entry in group.members %}
- {% if entry.href != "" %}[`{{ entry.signature }}`]({{ entry.href }}){% else %}`{{ entry.signature }}`{% endif %}{% if existsIn(entry, "hints") %} [{% for h in entry.hints %}{% if not loop.is_first %} {% endif %}{{ h }}{% endfor %}]{% endif %}{% if existsIn(entry, "children") %}{% for child in entry.children %}
    - {% if child.href != "" %}[`{{ child.signature }}`]({{ child.href }}){% else %}`{{ child.signature }}`{% endif %}{% if existsIn(child, "hints") %} [{% for h in child.hints %}{% if not loop.is_first %} {% endif %}{{ h }}{% endfor %}]{% endif %}{% endfor %}{% endif %}{{ "" }}
{% endfor %}
{% endfor %}
{% else %}
{% for entry in members %}
- {% if entry.href != "" %}[`{{ entry.signature }}`]({{ entry.href }}){% else %}`{{ entry.signature }}`{% endif %}{{ "" }}
{% endfor %}
{% endif %}

{% include "partials/md/footer.md" %}
