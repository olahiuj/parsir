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

    auto builder = LR1Parser{grammar};
    builder.genTable();
    auto &table = builder.getTable();
    std::cout << table;
}

[[maybe_unused]] static auto testExpr() -> void {
    auto grammar = Grammar::mk(
        "S'"_sym,
        Rule::mk("S'"_sym).of("E"_sym),
        Rule::mk("E"_sym).of("E"_sym, "+"_sym, "T"_sym),
        Rule::mk("E"_sym).of("T"_sym),
        Rule::mk("T"_sym).of("T"_sym, "*"_sym, "F"_sym),
        Rule::mk("T"_sym).of("F"_sym),
        Rule::mk("F"_sym).of("("_sym, "E"_sym, ")"_sym),
        Rule::mk("F"_sym).of("x"_sym));

    auto parser = LR1Parser{grammar};
    parser.genTable();
    auto &table = parser.getTable();

    std::cout << table;

    auto node = parser.parse(std::vector<Symbol>{"x"_sym, "*"_sym, "x"_sym, "+"_sym, "x"_sym, "$"_sym});
    std::cout << node;
}

auto main() -> int {
    testExpr();
    return 0;
}
