{#-
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
-#}
{% if existsIn(member, "sharedGroup") and existsIn(section, "sharedGroups") %}
{% set shared = at(section.sharedGroups, member.sharedGroup) %}
{% if existsIn(shared, "anchorId") and shared.anchorId != "" and shared.firstIndex == loop.index %}
<a id="{{ shared.anchorId }}"></a>
{% endif %}
{% endif %}
<a id="{{ member.anchorId }}"></a>
### {{ member.synopsis }}

{% if existsIn(member, "sharedGroup") and existsIn(section, "sharedGroups") %}
{% set shared = at(section.sharedGroups, member.sharedGroup) %}
{% if shared.firstIndex + shared.count - 1 == loop.index %}
{% if shared.status.id == "deprecated" %}
{% if shared.isPlural %}These{% else %}This{% endif %}{{ " " }}{{ shared.typeNoun }}{{ " " }}{% if shared.isPlural %}are{% else %}is{% endif %}{{ " " }}deprecated{% if existsIn(shared, "deprecatedSince") %}{{ " " }}since {{ shared.deprecatedSinceQualified }}{% endif %}. We strongly advise against using {% if shared.isPlural %}them{% else %}it{% endif %}{{ " " }}in new code.

{% else if shared.status.id == "active" and existsIn(shared, "deprecatedSince") %}
{% if shared.isPlural %}These{% else %}This{% endif %}{{ " " }}{{ shared.typeNoun }}{{ " " }}{% if shared.isPlural %}are{% else %}is{% endif %}{{ " " }}scheduled for deprecation in version {{ shared.deprecatedSince }}.

{% endif %}
{% set blocks = shared.body %}
{% include "partials/md/member_body.md" %}
{% set also = shared.alsoList %}
{% include "partials/md/member_also.md" %}
{% endif %}
{% else %}
{% if member.status.id == "deprecated" %}
This {{ member.typeNoun }} is deprecated{% if existsIn(member, "deprecatedSince") %}{{ " " }}since {{ member.deprecatedSinceQualified }}{% endif %}. We strongly advise against using it in new code.

{% else if member.status.id == "active" and existsIn(member, "deprecatedSince") %}
This {{ member.typeNoun }} is scheduled for deprecation in version {{ member.deprecatedSince }}.

{% endif %}
{% if existsIn(member, "body") %}{% set blocks = member.body %}{% else %}{% set blocks = [] %}{% endif %}
{% include "partials/md/member_body.md" %}
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
{% if not (existsIn(member, "sharedGroup") and existsIn(section, "sharedGroups")) %}
{% if existsIn(member, "alsoList") %}{% set also = member.alsoList %}{% else %}{% set also = [] %}{% endif %}
{% include "partials/md/member_also.md" %}
{% endif %}
