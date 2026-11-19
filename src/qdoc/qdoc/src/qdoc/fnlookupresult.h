// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef FNLOOKUPRESULT_H
#define FNLOOKUPRESULT_H

#include <QtCore/qset.h>

#include <type_traits>

QT_BEGIN_NAMESPACE

class Node;

/*
  The outcome of resolving a function signature against the
  documentation node database. Lookup either finds one node, finds no
  match, finds ambiguous fallback nodes, or deliberately ignores a symbol.
  These states are mutually exclusive. A matched result always carries a
  non-null node; an ambiguous result carries none, because an ambiguous
  resolution must not silently pick one of the candidates.
 */
template <typename NodeType>
class FnLookupResultFor
{
public:
    enum class Status
    {
        NoMatch, // No candidate matches the signature.
        Matched, // Exactly one node is selected.
        Ambiguous, // Multiple distinct fallback nodes match.
        Ignored // The symbol is explicitly excluded from lookup.
    };

    FnLookupResultFor() = default;

    Status status() const { return m_status; }
    bool hasMatch() const { return m_status == Status::Matched; }
    bool isAmbiguous() const { return m_status == Status::Ambiguous; }

    // Only a matched outcome exposes a candidate.
    NodeType *node() const
    {
        return m_status == Status::Matched ? m_node : nullptr;
    }

    void reset()
    {
        m_status = Status::NoMatch;
        m_node = nullptr;
    }

    void setMatched(NodeType *node)
    {
        Q_ASSERT(node != nullptr);
        m_status = Status::Matched;
        m_node = node;
    }

    void setAmbiguous()
    {
        m_status = Status::Ambiguous;
        m_node = nullptr;
    }

    void setIgnored()
    {
        m_status = Status::Ignored;
        m_node = nullptr;
    }

private:
    Status m_status = Status::NoMatch;
    NodeType *m_node = nullptr; // valid only in the Matched state
};

// The outcome depends on node identity, not on the node graph implementation.
using FnLookupResult = FnLookupResultFor<Node>;

enum class FnDiagnosticPhase
{
    Prepare, // A later generate pass owns unresolved-command diagnostics.
    Generate // Includes single-execution mode, where there is no later pass.
};

template <typename NodeType>
bool shouldDiagnoseFnLookup(const FnLookupResultFor<NodeType> &result, FnDiagnosticPhase phase)
{
    using Status = typename FnLookupResultFor<NodeType>::Status;
    return phase == FnDiagnosticPhase::Generate
            && (result.status() == Status::NoMatch || result.isAmbiguous());
}

/*
  Reduces a collection of matching candidates to a lookup result:
  identical node pointers are deduplicated, a single distinct node
  yields a matched result, two or more distinct nodes yield an
  ambiguous result, and an empty collection yields no match.
 */
template <typename Container>
auto resolveFnLookupMatches(const Container &matches)
{
    QSet<typename Container::value_type> distinct(matches.cbegin(), matches.cend());
    FnLookupResultFor<std::remove_pointer_t<typename Container::value_type>> result;
    if (distinct.size() == 1)
        result.setMatched(*distinct.cbegin());
    else if (distinct.size() > 1)
        result.setAmbiguous();
    return result;
}

// Prefer existing ordinary matches over renamed template parameters. Both
// passes preserve first-match behavior; only the fallback requires uniqueness.
template <typename Container, typename OrdinaryMatcher, typename RenamedMatcher,
          typename FallbackMatcher>
auto resolveFnLookupCandidates(const Container &candidates, OrdinaryMatcher ordinary,
                               RenamedMatcher renamed, FallbackMatcher fallback)
{
    FnLookupResultFor<std::remove_pointer_t<typename Container::value_type>> result;
    for (auto *candidate : candidates) {
        if (ordinary(candidate)) {
            result.setMatched(candidate);
            return result;
        }
    }

    for (auto *candidate : candidates) {
        if (renamed(candidate)) {
            result.setMatched(candidate);
            return result;
        }
    }

    QSet<typename Container::value_type> matches;
    for (auto *candidate : candidates) {
        if (fallback(candidate))
            matches.insert(candidate);
    }
    return resolveFnLookupMatches(matches);
}

QT_END_NAMESPACE

#endif // FNLOOKUPRESULT_H
