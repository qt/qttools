{#-
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
-#}
{% if hasNavigation %}
{% if existsIn(navigation, "prevLink") or existsIn(navigation, "nextLink") %}

---

{% if existsIn(navigation, "prevLink") %}[<< {{ navigation.prevLink.title }}]({{ navigation.prevLink.href }}){% endif %}{% if existsIn(navigation, "prevLink") and existsIn(navigation, "nextLink") %} | {% endif
%}{% if existsIn(navigation, "nextLink") %}[{{ navigation.nextLink.title }} >>]({{ navigation.nextLink.href }}){% endif %}

{% endif %}
{% endif %}

---

*Built with QDoc's template engine.*
