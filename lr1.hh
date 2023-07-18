#pragma once

#include "cst.hh"
#include "grammar.hh"
#include "table.hh"
#include "utils.hh"

#include <bits/ranges_base.h>
#include <cassert>
#include <concepts>
#include <functional>
#include <map>
#include <memory>
#include <numeric>
#include <ostream>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>

template <typename T>
concept input_stream = requires(T &t) {
    { *std::ranges::begin(t) } -> std::same_as<Symbol>;
    requires std::ranges::range<T>;
};

class Item {
  public:
    struct hash {
        auto operator()(const Item &item) const -> size_t {
            return Rule::hash{}(item.rule_)
                   * Symbol::hash{}(item.lookAhead_)
                   * (item.dot_ + 233);
        }
    };

    Item(Rule rule, Symbol lookAhead) :
      Item(rule, lookAhead, 0) {
    }

    auto advance() const -> Item {
        assert(dot_ < rule_.getBody().size());
        return {rule_, lookAhead_, dot_ + 1};
    }
    auto isDone() const -> bool { return dot_ == rule_.getBody().size(); }
    auto getRule() const -> Rule { return rule_; }
    auto getLookAhead() const -> Symbol { return lookAhead_; }
    auto getCurrentSymbol() const -> Symbol {
        return isDone() ? ""_sym : rule_.getBody().at(dot_);
    }
    auto getRestSymbols() const -> std::vector<Symbol> {
        return {rule_.getBody().begin() + dot_ + 1,
                rule_.getBody().end()};
    }
    auto operator<=>(const Item &) const = default;

  private:
    Item(Rule rule, Symbol lookAhead, size_t dot) :
      rule_(rule),
      lookAhead_(lookAhead),
      dot_(dot) {
    }

    Rule   rule_;
    Symbol lookAhead_;
    size_t dot_;
};

class LR1Parser {
  public:
    using ItemSetHandle = size_t;
    using ItemSet       = std::unordered_set<Item, Item::hash>;
    using TableT        = Table<ItemSetHandle>;

    struct hash {
        auto operator()(const ItemSet &items) const -> size_t {
            return std::transform_reduce(
                items.cbegin(),
                items.cend(),
                1,
                [](auto &&x, auto &&y) {
                    return x * y;
                },
                Item::hash{});
        }
    };

    LR1Parser(const Grammar &grammar) :
      grammar_(grammar),
      fSolver_(grammar),
      nItemSet_(0) {
        ItemSet startSet = closure({Item{grammar.getStartRule(), "$"_sym}});

        std::queue<ItemSetHandle> workList{};
        workList.push(getStartHandle());

        std::unordered_map<ItemSet, ItemSetHandle, hash> itemSetMap{};
        itemSetMap.emplace(startSet, getStartHandle());

        handleMap_ = {{getStartHandle(), startSet}};
        nItemSet_++;

        while (!workList.empty()) {
            auto &&items = workList.front();
            workList.pop();
            for (auto &&symbol : grammar.getSymbols()) {
                for (auto &&symbol : grammar.getSymbols()) {
                    ItemSet newSet = computeNext(handleMap_.at(items), symbol);
                    if (!newSet.empty()) {
                        if (!itemSetMap.contains(newSet)) {
                            itemSetMap.emplace(newSet, nItemSet_);
                            handleMap_.emplace(nItemSet_, newSet);
                            workList.push(nItemSet_++);
                        }
                        transitionMap_[{items, symbol}] = itemSetMap.at(newSet);
                    }
                }
            }
        }
    }

    auto getStartHandle() const -> ItemSetHandle { return 0; }
    auto getItemSet(ItemSetHandle handle) -> ItemSet { return handleMap_.at(handle); }
    auto getNext(ItemSetHandle handle, Symbol symbol) const noexcept -> std::optional<ItemSetHandle> {
        if (transitionMap_.contains({handle, symbol})) {
            return transitionMap_.at({handle, symbol});
        }
        return {};
    }

    static auto resolver(TableT::Action x, TableT::Action y, Symbol symbol) -> TableT::Action;

    auto genTable() -> void;
    auto getTable() const -> const TableT & { return *table_; }

    template <typename RangeT>
    auto parse(const RangeT &input) const -> cst::Node {
        using namespace cst;

        auto stateStack = std::stack<ItemSetHandle>{{getStartHandle()}};
        auto nodeStack  = std::stack<Node>{};
        for (auto it = input.begin(); it != input.end();) {
            auto symbol = *it;
            auto action = table_->getAction(stateStack.top(), symbol).value();
            switch (action.kind) {
                case TableT::SHIFT: {
                    stateStack.push(action.state);
                    nodeStack.push(Node{symbol.getName()});
                    it++;
                    break;
                }
                case TableT::REDUCE: {
                    auto body = std::deque<Node>{};
                    auto rule = action.rule.value();
                    for (size_t i = 0; i < rule.getBody().size(); i++) {
                        body.push_front(nodeStack.top());
                        nodeStack.pop();
                        stateStack.pop();
                    }
                    stateStack.push(table_->getTransition(stateStack.top(), rule.getHead()).value());

                    auto node = Node{rule.getHead().getName()};
                    for (auto child : body) {
                        node.getChildren()
                            .push_back(child);
                    }
                    nodeStack.push(node);
                    break;
                }
                case TableT::ACCEPT: {
                    return nodeStack.top();
                    break;
                }
            }
        }
        std::abort();
    }

  private:
    auto computeNext(const ItemSet &items, Symbol symbol) -> ItemSet;
    auto closure(const ItemSet &items) -> ItemSet;

    const Grammar &grammar_;
    First          fSolver_;
    size_t         nItemSet_ = 0;
    std::unique_ptr<TableT>
        table_;

    std::unordered_map<ItemSetHandle, ItemSet> handleMap_;
    std::map<std::pair<ItemSetHandle, Symbol>, ItemSetHandle>
        transitionMap_;
};
