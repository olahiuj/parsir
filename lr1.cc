#include "lr1.hh"
#include "table.hh"

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

auto LR1Builder::resolver(TableT::Action x, TableT::Action y, Symbol symbol) -> TableT::Action {
    // assume no conflict
    if (x == y) {
        return x;
    }
    std::abort();
}

auto LR1Builder::genTable() const -> TableT {
    auto table = TableT{nItemSet_, resolver};
    for (auto &&[handle, itemSet] : handleMap_) {
        for (auto &&item : itemSet) {
            if (item.isDone()) {
                auto lookAhead = item.getLookAhead();
                if (item.getRule() == grammar_.getStartRule()) {
                    // [S'->S*, $]
                    table.setAction(handle, lookAhead, TableT::Action::mkAccept());
                } else {
                    // [A->α*, a]
                    table.setAction(handle, lookAhead, TableT::Action::mkReduce(item.getRule()));
                }
            } else {
                auto currentSymbol = item.getCurrentSymbol();
                if (currentSymbol.isTerminal()) {
                    auto nextState = getNext(handle, currentSymbol).value();
                    table.setAction(handle, currentSymbol, TableT::Action::mkShift(nextState));
                }
            }
        }
        for (auto &&symbol : grammar_.getNTerms()) {
            auto nextState = getNext(handle, symbol);
            if (nextState.has_value()) {
                table.setTransition(handle, symbol, nextState.value());
            }
        }
    }
    return table;
}
