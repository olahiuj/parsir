#include "grammar.hh"
#include "utils.hh"

#include <iostream>

auto main() -> int {
    Grammar grammar = Grammar::mk(
        "E"_sym,
        Rule::mk("E"_sym).of("T"_sym, "A"_sym),
        Rule::mk("A"_sym).of("+"_sym, "T"_sym, "A"_sym),
        Rule::mk("A"_sym).of(""_sym),
        Rule::mk("T"_sym).of("F"_sym, "B"_sym),
        Rule::mk("B"_sym).of("*"_sym, "F"_sym, "B"_sym),
        Rule::mk("B"_sym).of(""_sym),
        Rule::mk("F"_sym).of("("_sym, "E"_sym, ")"_sym),
        Rule::mk("F"_sym).of("x"_sym));

    Follow follow(grammar);
    std::cout << follow.getFollow("E"_sym);
    std::cout << follow.getFollow("A"_sym);
    std::cout << follow.getFollow("T"_sym);
    std::cout << follow.getFollow("B"_sym);
    std::cout << follow.getFollow("F"_sym);
    return 0;
}
