#pragma once
#include "Assertion.h"
#include "Concepts.h"
#include "TypeTraits.h"
#include "Utility.h"

namespace ARLib {
    struct Monostate {
        bool operator==(const Monostate&) const { return true; }
        bool operator!=(const Monostate&) const { return false; }
    };
    namespace detail {
        struct MonostateT {};

        template <class First, class... Rest>
        class VariantStorage {
            static_assert(!IsAnyOfV<First, Rest...>, "Variant must have all different types");
            union {
                First head;
                VariantStorage<Rest...> tail;
            };
            bool is_active = false;

            public:
            template <class... Args>
            VariantStorage(Args&&... args) requires Constructible<First, Args...> :
                head(Forward<Args>(args)...),
                is_active(true) {}

            template <class... Args>
            VariantStorage(Args&&... args) : tail(Forward<Args>(args)...), is_active(false) {}

            VariantStorage& operator=(VariantStorage&& other) noexcept {
                if (other.is_active)
                    head = move(other.head);
                else
                    tail = move(other.tail);
                is_active = other.is_active;
                return *this;
            }

            VariantStorage() : tail(), is_active(false) {}

            template <class... Args>
            VariantStorage& operator=(Args&&... args) requires Constructible<First, Args...> {
                if (is_active) {
                    // current variant is already active with type, just replace
                    head = First{Forward<Args>(args)...};
                } else {
                    // current variant is active with another type, destroy type and replace
                    tail.deactivate();
                    head = First{Forward<Args>(args)...};
                    is_active = true;
                }
                return *this;
            }

            template <class... Args>
            VariantStorage& operator=(Args&&... args) {
                if (is_active) {
                    head.~First();
                    is_active = false;
                    tail = VariantStorage<Rest...>{Forward<Args>(args)...};
                } else
                    tail.operator=(Forward<Args>(args)...);
                return *this;
            }

            void deactivate() {
                if (is_active) {
                    is_active = false;
                    head.~First();
                } else {
                    tail.deactivate();
                }
            }

            template <class Type, typename = EnableIfT<IsAnyOfV<Type, First, Rest...>>>
            auto& get() const {
                if constexpr (IsSameV<Type, First>) {
                    HARD_ASSERT_FMT(is_active,
                                    "Bad variant access, requested type is `%s` but this type is not active.",
                                    TYPENAME_TO_STRING(Type))
                    return head;
                } else {
                    HARD_ASSERT_FMT(!is_active, "Bad variant access, requested type is `%s` but active type is `%s`",
                                    TYPENAME_TO_STRING(Type), TYPENAME_TO_STRING(First))
                    return tail.template get<Type>();
                }
            }

            template <class Type, typename = EnableIfT<IsAnyOfV<Type, First, Rest...>>>
            auto& get() {
                if constexpr (IsSameV<Type, First>) {
                    HARD_ASSERT_FMT(is_active,
                                    "Bad variant access, requested type is `%s` but this type is not active.",
                                    TYPENAME_TO_STRING(Type))
                    return head;
                } else {
                    HARD_ASSERT_FMT(!is_active, "Bad variant access, requested type is `%s` but active type is `%s`",
                                    TYPENAME_TO_STRING(Type), TYPENAME_TO_STRING(First))
                    return tail.template get<Type>();
                }
            }

            template <class Type>
            bool contains_type() const {
                if constexpr (IsSameV<Type, First>)
                    return is_active;
                else {
                    if (is_active)
                        return false;
                    else
                        return tail.template contains_type<Type>();
                }
            }

            bool active() const {
                if (is_active)
                    return true;
                else
                    return tail.active();
            }

            ~VariantStorage() {
                if (is_active)
                    head.~First();
                else
                    tail.~VariantStorage<Rest...>();
            };
        };

        template <class Type>
        class VariantStorage<Type> {
            union {
                Type head;
            };
            bool is_active = false;

            public:
            template <class... Args>
            VariantStorage(Args&&... args) requires Constructible<Type, Args...> :
                head(move(args)...),
                is_active(true) {}

            VariantStorage() : is_active(false) {}

            VariantStorage& operator=(VariantStorage&& other) noexcept {
                if (other.is_active) head = move(other.head);
                is_active = other.is_active;
                return *this;
            }

            template <class... Args>
            VariantStorage& operator=(Args&&... args) requires Constructible<Type, Args...> {
                deactivate();
                head = Type{Forward<Args>(args)...};
                is_active = true;
                return *this;
            }

            void deactivate() {
                if (is_active) {
                    is_active = false;
                    head.~Type();
                }
            }

            template <class T, typename = EnableIfT<IsSameV<T, Type>>>
            auto& get() const {
                HARD_ASSERT_FMT(is_active, "Bad variant access, requested type is `%s` but this type is not active.",
                                TYPENAME_TO_STRING(Type))
                return head;
            }

            template <class T, typename = EnableIfT<IsSameV<T, Type>>>
            auto& get() {
                HARD_ASSERT_FMT(is_active, "Bad variant access, requested type is `%s` but this type is not active.",
                                TYPENAME_TO_STRING(Type))
                return head;
            }

            template <class T, typename = EnableIf<IsSameV<T, Type>>>
            bool contains_type() const {
                return is_active;
            }

            bool active() const { return is_active; }

            ~VariantStorage() {
                if (is_active) head.~Type();
            };
        };

        template <>
        class VariantStorage<MonostateT> {
            MonostateT head;
            bool is_active = false;

            public:
            VariantStorage(Monostate) : head(), is_active(true) {}

            VariantStorage() : head(), is_active(false) {}

            VariantStorage& operator=(Monostate) {
                head = MonostateT{};
                is_active = true;
                return *this;
            }

            VariantStorage& operator=(VariantStorage&& other) noexcept {
                is_active = other.is_active;
                return *this;
            }

            void deactivate() {
                if (is_active) is_active = false;
            }

            auto& get() const {
                HARD_ASSERT_FMT(is_active, "Bad variant access, requested type is `%s` but this type is not active.",
                                TYPENAME_TO_STRING(MonostateT))
                return head;
            }

            auto& get() {
                HARD_ASSERT_FMT(is_active, "Bad variant access, requested type is `%s` but this type is not active.",
                                TYPENAME_TO_STRING(MonostateT))
                return head;
            }

            bool active() const { return is_active; }

            ~VariantStorage() {
                if (is_active) head.~MonostateT();
            };
        };
    } // namespace detail

    // this class is very fragile, it requires a lot of attention to not access members that are not initialized.
    // currently, it tries to prevent users from triggering UB, and the class itself *doesn't* invoke any.

    template <class... Types>
    class Variant {
        detail::VariantStorage<Types...> m_storage;

        public:
        template <typename... Args>
        Variant(Args&&... args) : m_storage(Forward<Args>(args)...) {}

        template <typename... Args>
        Variant& operator=(Args&&... args) {
            m_storage.operator=(Forward<Args>(args)...);
            return *this;
        }

        template <class T>
        auto& get() const {
            return m_storage.template get<T>();
        }

        template <class T>
        auto& get() {
            return m_storage.template get<T>();
        }

        template <class T>
        bool contains_type() {
            return m_storage.template contains_type<T>();
        }

        bool is_active() { return m_storage.active(); }

        ~Variant() = default;
    };

    template <>
    class Variant<detail::MonostateT> {
        detail::VariantStorage<detail::MonostateT> m_storage;

        public:
        Variant(Monostate state) : m_storage(state) {}

        Variant& operator=(Monostate state) {
            m_storage.operator=(state);
            return *this;
        }

        template <class T>
        requires IsSameV<T, Monostate>
        auto& get() const { return m_storage.get(); }

        template <class T>
        requires IsSameV<T, Monostate>
        auto& get() { return m_storage.get(); }

        template <class T>
        requires IsSameV<T, Monostate>
        bool contains_type() { return m_storage.active(); }

        bool is_active() { return m_storage.active(); }

        ~Variant() = default;
    };

} // namespace ARLib