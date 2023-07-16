#pragma once

#include "grammar.hh"

#include <map>
#include <set>
#include <vector>
#include <ranges>

class Nullable {
  public:
    Nullable(const Grammar &_grammar) :
      grammar(_grammar) {
    }

    auto nullable(Symbol symbol) -> bool;
    auto nullable(std::vector<Symbol> body) -> bool;

  private:
    std::map<Symbol, bool> nullableMap;
    const Grammar         &grammar;
};

class First {
  public:
    First(const Grammar &_grammar) :
      grammar(_grammar),
      nSolver(_grammar) {
    }

    auto getFirst(Symbol symbol) -> std::set<Symbol>;
    auto getFirst(std::vector<Symbol> symbol) -> std::set<Symbol>;
    auto getFirst(std::vector<Symbol> symbol, Symbol lookAhead) -> std::set<Symbol> {
        symbol.push_back(lookAhead);
        return getFirst(symbol);
    }

  private:
    auto solve() -> void;
    auto ruleWith(Symbol head) -> std::vector<Rule> {
        auto view = (grammar.getRules()
                     | std::views::filter([head](Rule rule) {
                           return rule.getHead() == head;
                       }));
        return {view.begin(), view.end()};
    }

    std::map<Symbol, std::set<Symbol>> firstMap;
    const Grammar                     &grammar;
    Nullable                           nSolver;
};

class Follow {
  public:
    Follow(const Grammar &_grammar) :
      grammar(_grammar),
      fSolver(_grammar),
      nSolver(_grammar) {
    }

    auto getFollow(Symbol symbol) -> std::set<Symbol>;

  private:
    auto solve() -> void;

    std::map<Symbol, std::set<Symbol>> followMap;
    const Grammar                     &grammar;
    First                              fSolver;
    Nullable                           nSolver;
};

auto find_follow(const Grammar &grammar, Symbol symbol) -> std::set<Symbol>;
