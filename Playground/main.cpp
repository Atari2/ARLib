#include "Regex.hpp"
#include "Printer.hpp"
#include "Matrix.hpp"
#include "Chrono.hpp"
#include "Graph.hpp"

using namespace ARLib;

struct Test : public GraphValueTypeBase<String> {

};
int main(int argc, char** argv) {
    Graph<String> g{};
    Graph<Test> g2{};
    const auto& nodeg1 = g2.add_node(Test{ "hello"_s });
    const auto& nodeg2 = g2.add_node(Test{ "world"_s });
    const auto& edgeg  = g2.add_edge(nodeg1, nodeg2);
    const auto& node   = g.add_node("Hello World"_s);
    const auto& edge   = g.add_edge("Hello World"_s, "This is a test"_s);
    auto node2         = g.find_node("Hello World"_s);
    auto node3         = g.find_node("asdf"_s);
    auto neighs        = g.neighbors("Hello World"_s);
    auto dneighs       = g.neighbors_directed("Hello World"_s);
    Printer::print("{}, {}", neighs, dneighs);
    Printer::print("{} {}", node2.has_value(), node3.has_value());
    auto edge1 = g.find_edge("Hello World"_s, "This is a test"_s);
    Printer::print("{}", edge1.has_value());
    // size_t removed = g.remove_node(node);    
    g.remove_edge(edge1.value());
    Vector<int> v{};
    v[123];
    return 0;
}
