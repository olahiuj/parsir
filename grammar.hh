#pragma once

#include <initializer_list>
#include <ostream>
#include <ranges>
#include <set>
#include <string>
#include <vector>

struct Symbol {
    static auto mkTerm(std::string name) -> Symbol {
        return {name, true};
    }
    static auto mkNTerm(std::string name) -> Symbol {
        return {name, false};
    }

    auto isEpsilon() const -> bool { return name_ == ""; }
    auto isTerminal() const -> bool { return isTerm_; }
    auto getName() const -> std::string { return name_; }
    auto operator<=>(const Symbol &) const = default;

  private:
    Symbol(std::string name, bool isTerm) :
      name_(name), isTerm_(isTerm) {
    }
    std::string name_;
    bool        isTerm_;
};

struct Rule {
    struct mk {
        mk(Symbol head) :
          head_(head) {
        }

        template <typename... T>
        auto of(T... body) -> Rule { return {head_, {body...}}; }

        Symbol head_;
    };

    auto getHead() const -> Symbol { return head_; }
    auto getBody() const -> const std::vector<Symbol> & { return body_; }
    auto getBody() -> std::vector<Symbol> & { return body_; }

  private:
    Rule(Symbol head, std::initializer_list<Symbol> body) :
      head_(head),
      body_(body) {
    }
    Symbol              head_;
    std::vector<Symbol> body_;
};

struct Grammar {
    template <typename... T>
    static auto mk(Symbol start, T... rules) -> Grammar {
        return {start, {rules...}};
    }

    auto getStart() const -> Symbol { return start_; }
    auto getSymbols() const -> std::set<Symbol> {
        std::set<Symbol> result{getStart()};
        for (auto &&rule : getRules()) {
            result.emplace(rule.getHead());
            for (auto &&symbol : rule.getBody()) {
                result.emplace(symbol);
            }
        }
        return result;
    }
    auto getNTerms() const -> std::set<Symbol> {
        auto range = getSymbols() | std::views::filter([](const Symbol &symbol) {
                         return !symbol.isEpsilon() && !symbol.isTerminal();
                     });
        return {range.begin(), range.end()};
    }
    auto getRules() const -> const std::vector<Rule> & { return rules_; }
    auto getRules() -> std::vector<Rule> & { return rules_; }

  private:
    Grammar(Symbol start, std::initializer_list<Rule> rules) :
      start_(start),
      rules_(rules) {
    }
    Symbol            start_;
    std::vector<Rule> rules_;
};

static inline auto operator""_sym(const char *str, size_t len) -> Symbol {
    if (len == 0) {
        return Symbol::mkTerm("");
    } else if (std::isupper(*str)) {
        return Symbol::mkNTerm(str);
    } else {
        return Symbol::mkTerm(str);
    }
}

static inline auto operator<<(std::ostream &os, Symbol symbol) -> std::ostream & {
    os << symbol.getName();
    return os;
}

static inline auto operator<<(std::ostream &os, Rule rule) -> std::ostream & {
    os << rule.getHead()
       << " -> ";
    for (auto &&x : rule.getBody()) {
        os << x << " ";
    }
    os << std::endl;
    return os;
}

static inline auto operator<<(std::ostream &os, std::set<Symbol> symbols) -> std::ostream & {
    bool first = true;
    os << "{";
    for (auto &&x : symbols) {
        if (!first) {
            os << ", ";
        }
        os << x;
        first = false;
    }
    os << "}";
    return os;
}
