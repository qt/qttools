{#-
Copyright (C) 2026 The Qt Company Ltd.
SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
-#}
# {{ fullTitle }}

{% if hasCppRef %}
{% if length(cppRef.templateDeclSpans) > 0 or cppRef.typeWord != "" %}
{% if length(cppRef.templateDeclSpans) > 0 %}{{ render_signature_spans(cppRef.templateDeclSpans) }} {% endif %}{{ cppRef.typeWord }}{{ " " }}{{ title }}

{% endif %}
{% endif %}
{% if default(brief, "") != "" %}
{{ brief }}

{% endif %}
{% if hasCppRef and cppRef.isPartialNamespace %}
The {{ title }} namespace is documented in the module [{{ cppRef.fullNamespaceModuleName }}]({{ cppRef.fullNamespaceHref }}).

{% endif %}
{% if hasCppRef and not cppRef.isInnerClass %}
| Key | Value |
| --- | --- |
{% if existsIn(cppRef, "headerInclude") and cppRef.headerInclude != "" %}| Header | `{{ cppRef.headerInclude }}` |
{% endif %}{% if existsIn(cppRef, "cmakeFindPackage") and cppRef.cmakeFindPackage != "" %}| CMake | `{{ cppRef.cmakeFindPackage }}` `{{ cppRef.cmakeTargetLinkLibraries }}` |
{% endif %}{% if existsIn(cppRef, "qmakeVariable") and cppRef.qmakeVariable != "" %}| qmake | `{{ cppRef.qmakeVariable }}` |
{% endif %}{% if default(since, "") != "" %}| Since | Qt {{ since }} |
{% endif %}{% if existsIn(cppRef, "qmlNativeType") %}| In QML | {% if cppRef.qmlNativeType.href != "" %}[{{ cppRef.qmlNativeType.name }}]({{ cppRef.qmlNativeType.href }}){% else %}{{ cppRef.qmlNativeType.name }}{% endif %} |
{% endif %}{% if not cppRef.suppressInheritance and length(cppRef.baseClasses) > 0 %}| Inherits | {% for base in cppRef.baseClasses %}{% if not loop.is_first %}, {% endif %}{% if base.href != "" %}[{{ base.name }}]({{ base.href }}){% else %}{{ base.name }}{% endif %}{% if base.access.id != "public" %} ({{ base.access.label }}){% endif %}{% endfor %} |
{% endif %}{% if length(cppRef.derivedClasses) > 0 %}| Inherited By | {% for derived in cppRef.derivedClasses %}{% if not loop.is_first %}, {% endif %}{% if derived.href != "" %}[{{ derived.name }}]({{ derived.href }}){% else %}{{ derived.name }}{% endif %}{% endfor %} |
{% endif %}{% if existsIn(cppRef, "statusText") and cppRef.statusText != "" %}| Status | {{ cppRef.statusText }} |
{% endif %}
{% endif %}

{% if hasCppRef and cppRef.isInnerClass %}
{% if default(since, "") != "" %}
| Key | Value |
| --- | --- |
| Since | Qt {{ since }} |

{% endif %}
{% endif %}
{% if membersPageUrl != "" %}
- [List of all members, including inherited members]({{ membersPageUrl }})

{% endif %}
{% if hasCppRef and cppRef.hasObsoleteMembers %}
- [Obsolete members]({{ cppRef.obsoleteMembersUrl }})

{% endif %}
{% if hasCppRef and length(cppRef.groups) > 0 %}
{{ title }} is part of {% for group in cppRef.groups %}{% if not loop.is_first %}, {% endif %}[{{ group.name }}]({{ group.href }}){% endfor %}.

{% endif %}
{% if hasCppRef and existsIn(cppRef, "threadSafety") %}
{% if cppRef.threadSafety.level == "thread-safe" %}
> **Note:** All functions in this {{ cppRef.typeWord }} are [thread-safe](threads-reentrancy.html).{% if length(cppRef.threadSafety.nonReentrantExceptions) > 0 %} with the following exceptions: {% for exc in cppRef.threadSafety.nonReentrantExceptions %}{% if not loop.is_first %}, {% endif %}{% if exc.href != "" %}[{{ exc.name }}]({{ exc.href }}){% else %}{{ exc.name }}{% endif %}{% endfor %}{% endif %}

{% else if cppRef.threadSafety.level == "reentrant" %}
> **Note:** All functions in this {{ cppRef.typeWord }} are [reentrant](threads-reentrancy.html).{% if length(cppRef.threadSafety.threadSafeExceptions) > 0 %} These functions are also [thread-safe](threads-reentrancy.html): {% for exc in cppRef.threadSafety.threadSafeExceptions %}{% if not loop.is_first %}, {% endif %}{% if exc.href != "" %}[{{ exc.name }}]({{ exc.href }}){% else %}{{ exc.name }}{% endif %}{% endfor %}.{% endif %}{% if length(cppRef.threadSafety.nonReentrantExceptions) > 0 %} These functions aren't [reentrant](threads-reentrancy.html): {% for exc in cppRef.threadSafety.nonReentrantExceptions %}{% if not loop.is_first %}, {% endif %}{% if exc.href != "" %}[{{ exc.name }}]({{ exc.href }}){% else %}{{ exc.name }}{% endif %}{% endfor %}.{% endif %}

{% else if cppRef.threadSafety.level == "non-reentrant" %}
> **Warning:** All functions in this {{ cppRef.typeWord }} are [non-reentrant](threads-reentrancy.html).{% if length(cppRef.threadSafety.reentrantExceptions) > 0 %} These functions are [reentrant](threads-reentrancy.html): {% for exc in cppRef.threadSafety.reentrantExceptions %}{% if not loop.is_first %}, {% endif %}{% if exc.href != "" %}[{{ exc.name }}]({{ exc.href }}){% else %}{{ exc.name }}{% endif %}{% endfor %}.{% endif %}

{% endif %}
{% endif %}
{% if hasCppRef and length(cppRef.comparisonEntries) > 0 %}

| Category | Type(s) | Description |
| --- | --- | --- |
{% for entry in cppRef.comparisonEntries %}
| {{ entry.category }} | {% for t in entry.comparableTypes %}{% if not loop.is_first %}, {% endif %}{{ t }}{% endfor %} | {{ entry.description }} |
{% endfor %}

{% endif %}
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

{% include "partials/md/content_blocks.md" %}

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
---

*Built with QDoc's template engine.*
