#pragma once
#include "Concepts.hpp"
#include "PrintInfo.hpp"
#include "UniquePtr.hpp"
namespace ARLib {

template <typename T>
class Tree;

enum class TreeStrategy { PreOrder, PostOrder, InOrder };
namespace detail {
    template <typename T>
    requires(MoreComparable<T> && EqualityComparable<T>)
    class TreeNode {
        friend Tree<T>;

        public:
        using NodePtr = UniquePtr<TreeNode<T>>;

        private:
        T m_value;
        NodePtr m_left_child;
        NodePtr m_right_child;

        protected:
        NodePtr& find_largest() {
            if (m_right_child->right_child().exists()) {
                return m_right_child->find_largest();
            } else {
                return m_right_child;
            }
        }
        void set_value(TreeNode<T>* node) {
            m_value = move(node->m_value);
            delete node;
        }
        NodePtr& left_child() { return m_left_child; }
        NodePtr& right_child() { return m_right_child; }

        public:
        TreeNode(T&& value, TreeNode* left_child, TreeNode* right_child) :
            m_value(move(value)), m_left_child(left_child), m_right_child(right_child) {}
        const T& value() const { return m_value; }
        const NodePtr& left_child() const { return m_left_child; }
        const NodePtr& right_child() const { return m_right_child; }
        void insert_leaf(TreeNode&& node) {
            if (node.value() > m_value || node.value() == m_value) {
                // right
                if (!m_right_child.exists()) {
                    m_right_child = NodePtr{ move(node) };
                } else {
                    m_right_child->insert_leaf(Forward<TreeNode>(node));
                }
            } else {
                // left
                if (!m_left_child.exists()) {
                    m_left_child = NodePtr{ move(node) };
                } else {
                    m_left_child->insert_leaf(Forward<TreeNode>(node));
                }
            }
        }
        const NodePtr& find(const T& value) const {
            static NodePtr empty_node{};
            if (value > m_value) {
                if (!m_right_child.exists()) return empty_node;
                if (m_right_child->value() == value)
                    return m_right_child;
                else
                    return m_right_child->find(value);
            } else {
                if (!m_left_child.exists()) return empty_node;
                if (m_left_child->value() == value)
                    return m_left_child;
                else
                    return m_left_child->find(value);
            }
        }
        void remove(const T& value) {
            if (value > m_value) {
                // search in right subtree
                if (!m_right_child.exists()) return;
                if (m_right_child->value() == value) {
                    delete_algorithm(m_right_child);
                } else {
                    m_right_child->remove(value);
                }
            } else {
                // search in left subtree
                if (!m_left_child.exists()) return;
                if (m_left_child->value() == value) {
                    delete_algorithm(m_left_child);
                } else {
                    m_left_child->remove(value);
                }
            }
        }
        template <typename Functor>
        void visit(Functor func, TreeStrategy strategy) {
            if (strategy == TreeStrategy::PreOrder) {
                // pre-order
                func(m_value);
                if (m_left_child.exists()) m_left_child->visit(func, strategy);
                if (m_right_child.exists()) m_right_child->visit(func, strategy);
            } else if (strategy == TreeStrategy::PostOrder) {
                // post-order
                if (m_left_child.exists()) m_left_child->visit(func, strategy);
                if (m_right_child.exists()) m_right_child->visit(func, strategy);
                func(m_value);
            } else {
                // in-order
                if (m_left_child.exists()) m_left_child->visit(func, strategy);
                func(m_value);
                if (m_right_child.exists()) m_right_child->visit(func, strategy);
            }
        }
        static void delete_algorithm(NodePtr& node) {
            // we have to remove the head
            bool left_exists  = node->left_child().exists();
            bool right_exists = node->right_child().exists();
            if (left_exists && right_exists) {
                // two children
                // 1. find largest in left subtree
                // 2. swap
                if (node->left_child()->right_child().exists()) {
                    auto& largest = node->left_child()->find_largest();
                    node->set_value(largest.release());
                } else {
                    auto& largest = node->left_child();
                    node->set_value(largest.release());
                }
            } else if (left_exists || right_exists) {
                // one child
                if (left_exists) {
                    node = move(node->left_child());
                } else {
                    node = move(node->right_child());
                }
            } else {
                // no children, simply remove head
                node.reset();
            }
        }
    };
}    // namespace detail
template <typename T>
class Tree {
    using Node = UniquePtr<detail::TreeNode<T>>;
    Node m_head;

    public:
    Tree() = default;
    void insert_leaf(T&& value) {
        if (!m_head.exists()) {
            m_head = Node{
                detail::TreeNode<T>{Forward<T>(value), nullptr, nullptr}
            };
        } else {
            m_head->insert_leaf(detail::TreeNode<T>{ Forward<T>(value), nullptr, nullptr });
        }
    }
    const Node& find(const T& value) const {
        static Node empty_node{};
        if (!m_head.exists()) return empty_node;
        if (m_head->value() == value)
            return m_head;
        else
            return m_head->find(value);
    }
    void remove(const T& value) {
        if (!m_head.exists()) return;
        if (m_head->value() == value) {
            // we have to remove the head
            detail::TreeNode<T>::delete_algorithm(m_head);
        } else {
            return m_head->remove(value);
        }
    }
    const Node& head() const { return m_head; }
    template <typename Functor>
    void visit(Functor func, TreeStrategy strategy = TreeStrategy::PreOrder) {
        if (!m_head.exists()) return;
        m_head->visit(func, strategy);
    }
};
template <Printable T>
struct PrintInfo<detail::TreeNode<T>> {
    const detail::TreeNode<T>& m_node;
    String repr() const { return "Node { "_s + PrintInfo<T>{ m_node.value() }.repr() + " }"_s; }
};
}    // namespace ARLib
