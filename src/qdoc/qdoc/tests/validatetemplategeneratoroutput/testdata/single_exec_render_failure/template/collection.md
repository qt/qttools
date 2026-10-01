{#-
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

Regression-test template: deliberately accesses a collection field that
does not exist in the IR, so the module page's render fails. QDoc must
report the failure, discard this page, and continue with later pages.
-#}
# {{ fullTitle }}

{% if hasCollection and collection.isModule %}
**Requires:** `{{ collection.definitelyMissingField }}`

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
