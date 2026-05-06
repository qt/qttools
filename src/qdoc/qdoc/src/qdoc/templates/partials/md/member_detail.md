{#-
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
-#}
<a id="{{ member.anchorId }}"></a>
### {{ member.synopsis }}

{% if existsIn(member, "body") and length(member.body) > 0 %}
{% for block in member.body %}
{% if block.type == "section" %}
{#- Rebind block to each child so the include targets the same dispatch.
    The loop variable shadows the outer block within the loop body. -#}
{% for block in block.children %}
{% include "partials/md/content_block_dispatch.md" %}
{% endfor %}

{% else %}
{% include "partials/md/content_block_dispatch.md" %}
{% endif %}
{% endfor %}
{% endif %}
{% if existsIn(member, "since") and member.since != "" %}
This {{ lower(member.nodeType.label) }} was introduced in Qt {{ member.since }}.

{% endif %}
{% if existsIn(member, "threadSafety") and member.threadSafety != "" %}
> **Note:** This function is {{ member.threadSafety }}.

{% endif %}
{% if existsIn(member, "comparisonCategory") and member.comparisonCategory != "" %}
> **Note:** Comparison category is {{ member.comparisonCategory }}.

{% endif %}
{% if existsIn(member, "noexceptNote") and member.noexceptNote != "" %}
> **Note:** This function is noexcept when {{ member.noexceptNote }}.

{% endif %}
{% if existsIn(member, "alsoList") and length(member.alsoList) > 0 %}
**See also** {% for block in member.alsoList %}{% for i in block.inlines %}{% if i.type == "link" %}{% if existsIn(i, "href") %}[{% for c in i.children %}{{ c.text }}{% endfor %}]({{ i.href }}){% else %}{% for c in i.children %}{{ c.text }}{% endfor %}{% endif %}{% else if i.type == "text" %}{{ i.text }}{% endif %}{% endfor %}{{ list_separator(loop.index, length(member.alsoList)) }}{% endfor %}{{ "" }}

{% endif %}
