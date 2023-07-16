#include "lr1.hh"

auto LR1Builder::computeNext(const ItemSet &items, Symbol symbol) -> ItemSet {
    ItemSet result{};
    for (auto &&item : items) {
        if (item.getCurrentSymbol() == symbol) {
            result.insert(item.advance());
        }
    }
    return closure(result);
}

auto LR1Builder::closure(const ItemSet &items) -> ItemSet {
    ItemSet result{items};

    bool changed;
    do {
        changed = false;
        for (auto &&item : result) {
            Symbol symbol = item.getCurrentSymbol();

            for (auto &&rule : grammar_.getRulesWith(symbol)) {
                for (auto &&lookAhead : fSolver_.getFirst(item.getRestSymbols(), item.getLookAhead())) {
                    changed |= result.insert({rule, lookAhead}).second;
                }
            }
        }
    } while (changed);
    return result;
}
