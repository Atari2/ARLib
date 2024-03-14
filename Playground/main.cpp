#include "Regex.hpp"
#include "Printer.hpp"
#include "Matrix.hpp"
#include "Chrono.hpp"
#include "Graph.hpp"

using namespace ARLib;
int main(int argc, char** argv) {
    Graph<String> g{};
    const auto& node = g.add_node("Hello World"_s);
    const auto& edge = g.add_edge("Hello World"_s, "This is a test"_s);
    auto node2       = g.find_node("Hello World"_s);
    auto node3       = g.find_node("asdf"_s);
    auto neighs      = g.neighbors("Hello World"_s);
    auto dneighs     = g.neighbors_directed("Hello World"_s);
    Printer::print("{}, {}", neighs, dneighs);
    Printer::print("{} {}", node2.has_value(), node3.has_value());
    auto edge1     = g.find_edge("Hello World"_s, "This is a test"_s);
    Printer::print("{}", edge1.has_value());
    // size_t removed = g.remove_node(node);
    g.remove_edge(edge1.value());
    return 0;
}
