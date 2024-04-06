#pragma once

#include "Concepts.hpp"
#include "Utility.hpp"
#include "SharedPtr.hpp"
#include "Vector.hpp"
#include "FlatSet.hpp"
#include "Optional.hpp"
#include "PrintInfo.hpp"
#include "FlatMap.hpp"
namespace ARLib {
template <typename T>
class Graph;
template <typename T>
class GraphNode {
    T m_value;
    public:
    GraphNode(T&& value) : m_value{ move(value) } {}
    const T& value() const { return m_value; }
    bool operator==(const GraphNode& other) const { return m_value == other.m_value; }
    bool operator==(const T& other) const { return m_value == other; }
};
template <typename T>
struct Hash<GraphNode<T>> {
    [[nodiscard]] size_t operator()(const GraphNode<T>& node) const { return Hash<T>{}(node.value()); }
};
template <typename T>
struct PrintInfo<GraphNode<T>> {
    const GraphNode<T>& m_node;
    PrintInfo(const GraphNode<T>& node) : m_node{ node } {}
    String repr() const { return print_conditional(m_node.value()); }
};
template <typename T>
class GraphEdge {
    friend Graph<T>;
    SharedPtr<GraphNode<T>> m_source_node;
    SharedPtr<GraphNode<T>> m_dest_node;
    double m_weight{ 1.0 };

    public:
    GraphEdge(SharedPtr<GraphNode<T>> source, SharedPtr<GraphNode<T>> dest) :
        m_source_node{ move(source) }, m_dest_node{ move(dest) } {}
    GraphEdge(SharedPtr<GraphNode<T>> source, SharedPtr<GraphNode<T>> dest, double weight) :
        m_source_node{ move(source) }, m_dest_node{ move(dest) }, m_weight{ weight } {}
    bool operator==(const GraphEdge& other) const {
        return *m_source_node == other.source() && *m_dest_node == other.dest() && m_weight == other.m_weight;
    }
    const GraphNode<T>& source() const { return *m_source_node; }
    const GraphNode<T>& dest() const { return *m_dest_node; }
    double weight() const { return m_weight; }
};
template <typename T>
struct PrintInfo<GraphEdge<T>> {
    const GraphEdge<T>& m_edge;
    PrintInfo(const GraphEdge<T>& edge) : m_edge{ edge } {}
    String repr() const {
        return "Edge { "_s + print_conditional(m_edge.source()) + " -> " + print_conditional(m_edge.dest()) + " }"_s;
    }
};
template <typename T>
concept HasGraphKeyType = requires(const T& val) {
    typename T::GraphKeyType;
    requires Hashable<typename T::GraphKeyType>;
    requires EqualityComparable<typename T::GraphKeyType>;
    { val.identifier() } -> SameAs<AddConstT<AddLvalueReferenceT<typename T::GraphKeyType>>>;
};
template <HasGraphKeyType T>
struct Hash<T> {
    [[nodiscard]] size_t operator()(const T& node) const { return Hash<typename T::GraphKeyType>{}(node.identifier()); }
};
template <HasGraphKeyType T>
bool operator==(const T& lhs, const T& rhs) {
    return lhs.identifier() == rhs.identifier();
}
template <typename KeyType>
class GraphValueTypeBase {
    KeyType m_identifier;
    public:
    using GraphKeyType = KeyType;
    GraphValueTypeBase(KeyType&& identifier) : m_identifier{ move(identifier) } {}
    const KeyType& identifier() const { return m_identifier; }
};
template <typename T>
class Graph {
    using NodeType = SharedPtr<GraphNode<T>>;
    struct GraphNodeHasher {
        [[nodiscard]] size_t operator()(const NodeType& ptr) const noexcept { return Hash<T>{}(ptr->value()); }
    };
    struct GraphNodeComparer {
        [[nodiscard]] bool operator()(const NodeType& lhs, const NodeType& rhs) const {
            return lhs->value() == rhs->value();
        }
        [[nodiscard]] bool operator()(const NodeType& lhs, const T& rhs) const { return lhs->value() == rhs; }
        [[nodiscard]] bool operator()(const T& lhs, const NodeType& rhs) const { return lhs == rhs->value(); }
        [[nodiscard]] bool operator()(const NodeType& lhs, const GraphNode<T>& rhs) const {
            return lhs->value() == rhs.value();
        }
        [[nodiscard]] bool operator()(const GraphNode<T>& lhs, const NodeType& rhs) const {
            return lhs.value() == rhs->value();
        }
    };
    using HasherType   = ConditionalT<Hashable<T>, GraphNodeHasher, Hash<NodeType>>;
    using ComparerType = ConditionalT<EqualityComparable<T>, GraphNodeComparer, DefaultKeyComparer<NodeType>>;
    FlatSet<SharedPtr<GraphNode<T>>, HasherType, ComparerType> m_nodes{};
    Vector<GraphEdge<T>> m_edges{};

    public:
    Graph() = default;
    const GraphNode<T>& add_node(T&& value) {
        const auto&& [inserted, v] = m_nodes.insert(SharedPtr{ new GraphNode{ Forward<T>(value) } });
        return *v;
    }
    const GraphEdge<T>& add_edge(T&& source, T&& dest, double weight = 1.0) {
        const auto&& [source_inserted, source_v] = m_nodes.insert(SharedPtr{ new GraphNode{ Forward<T>(source) } });
        const auto&& [dest_inserted, dest_v]     = m_nodes.insert(SharedPtr{ new GraphNode{ Forward<T>(dest) } });
        m_edges.append(GraphEdge{ source_v, dest_v, weight });
        return m_edges.last();
    }
    const GraphEdge<T>& add_edge(const GraphNode<T>& source, const GraphNode<T>& dest, double weight = 1.0) {
        auto sit = m_nodes.find(source);
        auto dit = m_nodes.find(dest);
        HARD_ASSERT(sit != m_nodes.end() && dit != m_nodes.end(), "Invalid nodes when inserting edge");
        m_edges.append(GraphEdge{ *sit, *dit, weight });
        return m_edges.last();
    }
    auto find_node(const T& value) const {
        using RetOpt = Optional<const GraphNode<T>&>;
        auto it      = m_nodes.find(value);
        if (it == m_nodes.end()) return RetOpt{};
        const auto& sptr = *it;
        return RetOpt{ *sptr };
    }
    Optional<GraphEdge<T>> find_edge(const T& n1, const T& n2) const {
        return m_edges.iter()
        .find_if([&](const auto& edge) {
            auto b1 = edge.source().value() == n1 && edge.dest().value() == n2;
            auto b2 = edge.source().value() == n2 && edge.dest().value() == n1;
            return b1 || b2;
        })
        .map([](auto&& opt) { return GraphEdge{ opt }; });
    }
    Optional<GraphEdge<T>> find_directed_edge(const T& source, const T& dest) const {
        return m_edges.iter()
        .find_if([&](const auto& edge) {
            auto b1 = edge.source().value() == source && edge.dest().value() == dest;
            return b1;
        })
        .map([](auto&& opt) { return GraphEdge{ opt }; });
    }
    Optional<GraphEdge<T>> find_edge(const GraphNode<T>& n1, const GraphNode<T>& n2) const {
        return m_edges.iter()
        .find_if([&](const auto& edge) {
            auto b1 = edge.source().value() == n1.value() && edge.dest().value() == n2.value();
            auto b2 = edge.source().value() == n2.value() && edge.dest().value() == n1.value();
            return b1 || b2;
        })
        .map([](auto&& opt) { return GraphEdge{ opt }; });
    }
    Optional<GraphEdge<T>> find_directed_edge(const GraphNode<T>& source, const GraphNode<T>& dest) const {
        return m_edges.iter()
        .find_if([&](const auto& edge) {
            auto b1 = edge.source().value() == source.value() && edge.dest().value() == dest.value();
            return b1;
        })
        .map([](auto&& opt) { return GraphEdge{ opt }; });
    }
    size_t remove_node(const T& value) {
        m_nodes.remove(value);
        return m_edges.remove_matching([&value](const auto& edge) {
            return edge.source().value() == value || edge.dest().value() == value;
        });
    }
    size_t remove_node(const GraphNode<T>& node) {
        size_t edges_removed = m_edges.remove_matching([&node](const auto& edge) {
            return edge.source().value() == node.value() || edge.dest().value() == node.value();
        });
        m_nodes.remove(node);
        return edges_removed;
    }
    void remove_edge(const T& source, const T& dest) {
        m_edges.remove_matching([&source, &dest](const auto& edge) {
            return edge.source().value() == source && edge.dest().value() == dest;
        });
    }
    void remove_edge(const GraphEdge<T>& edge) {
        auto it = m_edges.find(edge);
        if (it != m_edges.end()) { m_edges.remove(it); }
    }
    bool adjacent(const T& n1, const T& n2) const { return find_edge(n1, n2).has_value(); }
    bool adjacent_directed(const T& source, const T& dest) const {
        return find_directed_edge(source, dest).has_value();
    }
    bool adjacent(const GraphNode<T>& n1, const GraphNode<T>& n2) const { return find_edge(n1, n2).has_value(); }
    bool adjacent_directed(const GraphNode<T>& source, const GraphNode<T>& dest) const {
        return find_directed_edge(source, dest).has_value();
    }
    Vector<SharedPtr<GraphNode<T>>> neighbors(const T& node) const {
        return m_edges.iter()
        .filter([&](const auto& edge) { return edge.source().value() == node || edge.dest().value() == node; })
        .map([&](const auto& edge) -> SharedPtr<GraphNode<T>> {
            if (edge.source().value() == node) {
                return edge.m_dest_node;
            } else {
                return edge.m_source_node;
            }
        })
        .template collect<Vector>();
    }
    Vector<SharedPtr<GraphNode<T>>> neighbors(const GraphNode<T>& node) const { return neighbors(node.value()); }
    Vector<SharedPtr<GraphNode<T>>> neighbors_directed(const T& node) const {
        return m_edges.iter()
        .filter([&](const auto& edge) { return edge.source().value() == node; })
        .map([&](const auto& edge) -> SharedPtr<GraphNode<T>> { return edge.m_dest_node; })
        .template collect<Vector>();
    }
    Vector<SharedPtr<GraphNode<T>>> neighbors_directed(const GraphNode<T>& node) const {
        return neighbors_directed(node.value());
    }
    Vector<SharedPtr<GraphNode<T>>> dijkstra(const GraphNode<T>& source, const GraphNode<T>& dest) {
        // FIXME: this implementation is very memory hungry and very inefficient
        //        needs some care and fixing.
        FlatMap<GraphNode<T>, double> dist{};
        FlatMap<GraphNode<T>, Optional<GraphNode<T>>> prev{};
        Vector<GraphNode<T>> queue{};
        queue.reserve(m_nodes.size());
        for (const auto& v : m_nodes) {
            dist.insert(*v, NumericLimits::InfinityD);
            prev.insert(*v, Optional<GraphNode<T>>{});
            queue.append(*v);
        }

        auto find_min_dist = [&queue](const auto& distmap) {
            auto current_min = queue.begin();
            auto end         = queue.end();
            auto current     = queue.begin();
            while (current != end) {
                if (distmap[*current] < distmap[*current_min]) { current_min = current; }
                ++current;
            }
            return *current_min;
        };

        dist[source] = 0.0;
        while (queue.size() != 0) {
            const auto& u = find_min_dist(dist);
            if (u == dest) {
                Optional<GraphNode<T>> optu{ u };
                Vector<SharedPtr<GraphNode<T>>> stack{};
                if (prev[u].has_value() || u == source) {
                    while (optu.has_value()) {
                        stack.append(*m_nodes.find(*optu));
                        optu = prev[*optu];
                    }
                }
                return move(stack).reversed();
            }
            queue.remove(u);
            for (auto&& v : neighbors_directed(u).iter().filter([&queue](const auto& n) {
                     return queue.find(*n) != queue.end();
                 })) {
                double alt =
                dist[u] +
                find_directed_edge(u, *v).map(&GraphEdge<T>::weight).value_or(double{ NumericLimits::InfinityD });
                if (alt < dist[*v]) {
                    dist[*v] = alt;
                    prev[*v] = u;
                }
            }
        }
        return {};
    }
    Vector<SharedPtr<GraphNode<T>>> dijkstra(const T& source, const T& dest) {
        const auto& sourcenode = m_nodes.find(source);
        const auto& destnode   = m_nodes.find(dest);
        if (sourcenode == m_nodes.end() || destnode == m_nodes.end()) return {};
        return dijkstra(*sourcenode, *destnode);
    }
    size_t n_nodes() const { return m_nodes.size(); }
    size_t n_edges() const { return m_edges.size(); }
    const auto& nodes() const { return m_nodes; }
    const auto& edges() const { return m_edges; }
};
}    // namespace ARLib