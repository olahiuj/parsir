/**
 * Utility algorithms to support parsing
 * * nullable(grammar, symbol): returns if symbol can derive ε
 * * find_first(grammar, symbol): returns FIRST(symbol) set
 */

#include "utils.hh"
#include "grammar.hh"

#include <cassert>
#include <functional>
#include <numeric>
#include <queue>
#include <ranges>
#include <map>
#include <set>
#include <vector>

template <typename T>
static auto mergeInto(const std::set<T> &from, std::set<T> &to) -> bool {
    bool changed = false;
    for (auto &&elem : from) {
        changed |= to.insert(elem).second;
    }
    return changed;
}

auto Nullable::nullable(std::vector<Symbol> body) -> bool {
    for (auto &&symbol : body) {
        if (!nullable(symbol)) {
            return false;
        }
    }
    return true;
}

auto Nullable::nullable(Symbol symbol) -> bool {
    if (nullableMap.count(symbol)) {
        return nullableMap.at(symbol);
    }

    if (symbol.isEpsilon()) {
        nullableMap.emplace(symbol, true);
        return true;
    } else if (symbol.isTerminal()) {
        nullableMap.emplace(symbol, false);
        return false;
    }

    for (auto &&rule : ruleWith(symbol)) {
        if (nullable(rule.getBody())) {
            nullableMap.emplace(symbol, true);
            return true;
        }
    }

    nullableMap.emplace(symbol, false);
    return false;
}

auto First::getFirst(std::vector<Symbol> body) -> std::set<Symbol> {
    assert(body.size() > 0);
    std::set<Symbol> result{};

    for (auto &&symbol : body) {
        mergeInto(getFirst(symbol), result);
        if (!nSolver.nullable(symbol)) {
            break;
        }
    }
    return result;
}

auto First::getFirst(Symbol symbol) -> std::set<Symbol> {
    if (firstMap.count(symbol)) {
        return firstMap.at(symbol);
    }
    solve();
    return firstMap.at(symbol);
}

auto First::solve() -> void {
    for (auto &&symbol : grammar.getSymbols()) {
        firstMap.emplace(symbol, std::set<Symbol>{});

        if (symbol.isEpsilon()) {
            continue;
        } else if (symbol.isTerminal()) {
            firstMap.at(symbol).insert(symbol);
        }
    }

    bool changed;
    do {
        changed = false;
        for (auto &&rule : grammar.getRules()) {
            auto &&head = rule.getHead();
            for (auto &&symbol : rule.getBody()) {
                changed |= mergeInto(firstMap.at(symbol), firstMap.at(head));
                if (!nSolver.nullable(symbol)) {
                    break;
                }
            }
        }
    } while (changed);
}

auto Follow::getFollow(Symbol symbol) -> std::set<Symbol> {
    if (followMap.count(symbol)) {
        return followMap.at(symbol);
    }
    solve();
    return followMap.at(symbol);
}

auto Follow::solve() -> void {
    for (auto &&symbol : grammar.getSymbols()) {
        followMap.emplace(symbol, std::set<Symbol>{});
    }
    followMap.at(grammar.getStart()).insert("$"_sym);

    bool changed;
    do {
        changed = false;
        for (auto &&rule : grammar.getRules()) {
            Symbol A = rule.getHead();
            for (auto it = rule.getBody().begin(); it != rule.getBody().end(); it++) {
                Symbol B = *it;
                if (B.isTerminal()) {
                    continue;
                }

                std::vector<Symbol> rest{it + 1, rule.getBody().end()};
                if (rest.empty() || nSolver.nullable(rest)) {
                    // A -> αB or nullable(β)
                    changed |= mergeInto(followMap[A], followMap[B]);
                }
                if (!rest.empty()) {
                    // A -> αBβ
                    changed |= mergeInto(fSolver.getFirst(rest), followMap[B]);
                }
            }
        }
    } while (changed);
}
