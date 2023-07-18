#pragma once

#include <ostream>
#include <stack>
#include <string>
#include <vector>

namespace cst {

class Node {
  public:
    Node(std::string type) :
      type_(type) {
    }
    Node(std::string       type,
         std::vector<Node> children) :
      type_(type),
      children_(children) {
    }

    auto getType() const -> std::string { return type_; }
    auto getChildren() -> std::vector<Node> & { return children_; }
    auto getChildren() const -> const std::vector<Node> & { return children_; }

  private:
    std::string       type_;
    std::vector<Node> children_;
};

static inline auto operator<<(std::ostream &os, const Node &node) -> std::ostream & {
    struct data {
        int         depth;
        int         nth;
        const Node &cur;
    };
    auto stack = std::stack<data>{{data{0, 0, node}}};
    while (!stack.empty()) {
        auto top = stack.top();
        stack.pop();

        if (top.nth == 0) {
            os << std::string(top.depth, ' ')
               << top.cur.getType()
               << std::endl;
        }
        if (top.nth < top.cur.getChildren().size()) {
            stack.push(data{top.depth, top.nth + 1, top.cur});
            stack.push(data{top.depth + 2, 0, top.cur.getChildren().at(top.nth)});
        }
    }
    return os;
}

}; // namespace cst
