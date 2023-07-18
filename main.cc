#include "grammar.hh"
#include "lr1.hh"
#include "utils.hh"

#include <iostream>

[[maybe_unused]] static auto testFollow() -> void {
    auto grammar = Grammar::mk(
        "E"_sym,
        Rule::mk("E"_sym).of("T"_sym, "A"_sym),
        Rule::mk("A"_sym).of("+"_sym, "T"_sym, "A"_sym),
        Rule::mk("A"_sym).of(""_sym),
        Rule::mk("T"_sym).of("F"_sym, "B"_sym),
        Rule::mk("B"_sym).of("*"_sym, "F"_sym, "B"_sym),
        Rule::mk("B"_sym).of(""_sym),
        Rule::mk("F"_sym).of("("_sym, "E"_sym, ")"_sym),
        Rule::mk("F"_sym).of("x"_sym));

    auto follow = Follow{grammar};
    std::cout << follow.getFollow("E"_sym);
    std::cout << follow.getFollow("A"_sym);
    std::cout << follow.getFollow("T"_sym);
    std::cout << follow.getFollow("B"_sym);
    std::cout << follow.getFollow("F"_sym);
}

[[maybe_unused]] static auto testLR1() -> void {
    auto grammar = Grammar::mk(
        "S'"_sym,
        Rule::mk("S'"_sym).of("S"_sym),
        Rule::mk("S"_sym).of("C"_sym, "C"_sym),
        Rule::mk("C"_sym).of("c"_sym, "C"_sym),
        Rule::mk("C"_sym).of("d"_sym));

    auto builder = LR1Builder{grammar};
    auto table   = builder.genTable();

    std::cout << "\t: ";
    for (auto &&symbol : grammar.getTerms()) {
        std::cout << symbol
                  << "\t\t";
    }
    std::cout << std::endl;
    for (size_t state = 0; state < table.getStateCount(); state++) {
        std::cout << state
                  << "\t: ";
        for (auto &&symbol : grammar.getTerms()) {
            std::cout
                << table.getAction(state, symbol)
                       .transform([](auto &&x) { return x.to_string(); })
                       .value_or("")
                << "\t\t";
        }
        std::cout << std::endl;
    }
}

auto main() -> int {
    testLR1();
    return 0;
}
