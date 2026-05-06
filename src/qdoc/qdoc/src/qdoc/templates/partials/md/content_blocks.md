{#-
Copyright (C) 2025 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
-#}
{% if length(content.blocks) > 0 %}
{% for block in content.blocks %}
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
{% else if existsIn(content, "text") %}
{{ content.text }}

{% endif %}
