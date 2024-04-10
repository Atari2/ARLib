#include "Regex.hpp"
#include "Printer.hpp"
#include "Matrix.hpp"
#include "Chrono.hpp"
#include "Graph.hpp"
#include "Stream.hpp"
#include "JSONParser.hpp"

using namespace ARLib;
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    Graph<uint32_t> g{};
    auto& pcg = Random::PCG::static_state();
    FlatSet<uint32_t> unique_values{};
    for (size_t i = 0; i < 100; ++i) unique_values.insert(pcg.random());
    Printer::print("{} unique values", unique_values.size());
    Vector<uint32_t> unique_values_vec{};
    unique_values_vec.reserve(unique_values.size());
    for (auto&& [i, v] : enumerate(unique_values)) {
        g.add_node(uint32_t{ v });
        unique_values_vec.append(v);
    }
    const size_t idx_cap = unique_values_vec.size();
    for (size_t i = 0; i < 1000; ++i) {
        auto source = pcg.bounded_random(static_cast<uint32_t>(idx_cap));
        auto dest   = pcg.bounded_random(static_cast<uint32_t>(idx_cap));
        g.add_edge(uint32_t{ unique_values_vec[source] }, uint32_t{ unique_values_vec[dest] });
    }
    Printer::print("{} nodes and {} edges", g.n_nodes(), g.n_edges());
    auto v = "   \t\n  hello world    \f\v\n  "_sv.trim();
    Printer::print("{}", v);
    return 0;
}
