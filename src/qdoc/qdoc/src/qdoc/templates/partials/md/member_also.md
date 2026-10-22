{#-
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
-#}
{% if length(also) > 0 %}
**See also** {% for block in also %}{% for i in block.inlines %}{% if i.type == "link" %}{% if existsIn(i, "href") %}[{% for c in i.children %}{{ c.text }}{% endfor %}]({{ i.href }}){% else %}{% for c in i.children %}{{ c.text }}{% endfor %}{% endif %}{% else if i.type == "text" %}{{ i.text }}{% endif %}{% endfor %}{{ list_separator(loop.index, length(also)) }}{% endfor %}{{ "" }}

{% endif %}
