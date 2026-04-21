{#-
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
-#}
{% if hasNavigation and length(navigation.breadcrumbs) > 1 %}
{% for crumb in navigation.breadcrumbs %}{% if crumb.state == "link" %}[{{ crumb.title }}]({{ crumb.href }}){% else %}{{ crumb.title }}{% endif %}{% if not loop.is_last %} > {% endif %}{% endfor %}

{% endif %}
{% if hasNavigation and navigation.tocDepth != 0 and length(navigation.tocEntries) > 0 %}
**Contents**

{% for entry in navigation.tocEntries %}
{% if entry.level <= navigation.tocDepth or navigation.tocDepth < 0 %}
{% if entry.level == 2 %}- [{{ entry.title }}](#{{ entry.anchorId }})
{% else if entry.level == 3 %}  - [{{ entry.title }}](#{{ entry.anchorId }})
{% else if entry.level == 4 %}    - [{{ entry.title }}](#{{ entry.anchorId }})
{% else if entry.level == 5 %}      - [{{ entry.title }}](#{{ entry.anchorId }})
{% else if entry.level == 6 %}        - [{{ entry.title }}](#{{ entry.anchorId }})
{% else %}- [{{ entry.title }}](#{{ entry.anchorId }})
{% endif %}
{% endif %}
{% endfor %}

{% endif %}
