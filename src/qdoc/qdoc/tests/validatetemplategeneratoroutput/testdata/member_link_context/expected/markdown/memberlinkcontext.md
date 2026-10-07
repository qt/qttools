[MemberLink](memberlink-module.md)> MemberLinkContext

**Contents**

- [Public Functions](#public-functions)
- [Detailed Description](#details)
- [Member Function Documentation](#member-function-documentation)

# MemberLinkContext

class MemberLinkContext

A class that carries the member link context test cases.

| Key | Value |
| --- | --- |
| Header | `member_link_context.h` |

- [List of all members, including inherited members](memberlinkcontext-members.md)


## Public Functions

| Member | Description |
| --- | --- |
| `void alpha()` |  |
| `void beta()` |  |
| `void sharedOne()` |  |
| `void sharedTwo()` |  |

## Detailed Description
The detailed description of the page links to the beta member: [beta](memberlinkcontext.md#beta)
That link is authored in the page body, so it must keep resolving in the page context and reach the member anchor of this page.

## Member Function Documentation

<a id="alpha"></a>
### void alpha()

Alpha does its own work. The other member, [beta](memberlinkcontext.md#beta), does related work.
# Alpha Notes

Follow [member-detail](memberlinkcontext.md#alpha-notes).

**See also** [beta](memberlinkcontext.md#beta) and [member-detail](memberlinkcontext.md#alpha-notes).

<a id="beta"></a>
### void beta()

Beta does related work. This member also links to a member that does not exist: MissingMember
# Beta Notes

Follow [member-detail](memberlinkcontext.md#beta-notes).

**See also** [member-detail](memberlinkcontext.md#beta-notes).

<a id="sharedOne"></a>
### void sharedOne()

Both shared operations do the same work. They refer to [beta](memberlinkcontext.md#beta) from the shared comment that documents the pair.
<a id="sharedTwo"></a>
### void sharedTwo()

Both shared operations do the same work. They refer to [beta](memberlinkcontext.md#beta) from the shared comment that documents the pair.

---

*Built with QDoc's template engine.*
