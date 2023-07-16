#pragma once

#include "grammar.hh"
#include "utils.hh"

#include <cassert>
#include <functional>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>

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

class LR1Builder {
  public:
    using ItemSetHandle = size_t;
    using ItemSet       = std::unordered_set<Item, Item::hash>;

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

    LR1Builder(const Grammar &grammar) :
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
    auto getNext(ItemSetHandle handle, Symbol symbol) const -> ItemSetHandle { return transitionMap_.at({handle, symbol}); }

  private:
    auto computeNext(const ItemSet &items, Symbol symbol) -> ItemSet;
    auto closure(const ItemSet &items) -> ItemSet;

    const Grammar &grammar_;
    First          fSolver_;
    size_t         nItemSet_ = 0;

    std::unordered_map<ItemSetHandle, ItemSet> handleMap_;
    std::map<std::pair<ItemSetHandle, Symbol>, ItemSetHandle>
        transitionMap_;
};
