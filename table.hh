#pragma once

#include "grammar.hh"

#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <string>

template <typename StateT>
class Table {
  public:
    enum ActionKind {
        SHIFT,
        REDUCE,
        ACCEPT,
    };

    struct Action {
        static auto mkShift(StateT state) -> Action { return {SHIFT, {}, state}; }
        static auto mkReduce(Rule rule) -> Action { return {REDUCE, rule, {}}; }
        static auto mkAccept() -> Action { return {ACCEPT, {}, {}}; }

        auto operator<=>(const Action &) const = default;
        auto to_string() const -> std::string {
            switch (kind) {
                case SHIFT: return "s" + std::to_string(state);
                case REDUCE: return "r" + rule.value().to_string();
                case ACCEPT: return "a";
                default: std::abort();
            }
        }

        enum ActionKind     kind;
        std::optional<Rule> rule;
        StateT              state;
    };

    template <typename FuncT>
    Table(size_t nState, FuncT resolver) :
      nState_(nState),
      resolver_(resolver),
      transitionTable_(nState),
      actionTable_(nState) {
    }

    auto getAction(StateT state, Symbol symbol) const -> std::optional<Action> {
        auto &entry = actionTable_[state];
        if (entry.contains(symbol)) {
            return entry.at(symbol);
        }
        return {};
    }
    auto setAction(StateT state, Symbol symbol, Action action) -> void {
        actionTable_[state][symbol] =
            getAction(state, symbol)
                .transform([this, action, symbol](auto &&x) {
                    return resolver_(x, action, symbol);
                })
                .value_or(action);
    }
    auto getTransition(StateT from, Symbol symbol) const -> std::optional<StateT> {
        auto &entry = transitionTable_[from];
        if (entry.contains(symbol)) {
            return entry.at(symbol);
        }
        return {};
    }
    auto setTransition(StateT from, Symbol symbol, StateT to) -> void {
        transitionTable_[from][symbol] = to;
    }
    auto getStateCount() const -> size_t {
        return nState_;
    }

  private:
    size_t nState_;
    std::function<Action(Action, Action, Symbol)>
        resolver_;
    std::vector<std::map<Symbol, StateT>>
        transitionTable_;
    std::vector<std::map<Symbol, Action>>
        actionTable_;
};
