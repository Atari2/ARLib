#pragma once
#include "Concepts.hpp"
#include "Algorithm.hpp"
#include "Array.hpp"
namespace ARLib {
namespace v2 {

    template <size_t Size, size_t Align>
    struct VariantStorage {
        alignas(Align) uint8_t m_memory[Size];
        template <typename T>
        void initialize(T&& value) {
            new (m_memory) T{ Forward<T>(value) };
        }
        const uint8_t* raw_memory() const { return m_memory; }
        uint8_t* raw_memory() { return m_memory; }
        template <typename T>
        const T& as() const {
            return *reinterpret_cast<const T*>(m_memory);
        }
        template <typename T>
        T& as() {
            return *reinterpret_cast<T*>(m_memory);
        }
    };
    template <typename... Args>
    struct PackCarrier {};
    struct InvalidConstructionType {};
    template <typename T, typename... Args, template <typename...> typename Carrier>
    constexpr const auto& first_constructible_of(Carrier<Args...>) {
        using Type = ConditionalT<Constructible<T, Args...>, T, InvalidConstructionType>;
        Type* ptr{ nullptr };
        return *ptr;
    }
    template <typename T, typename... Rest, typename... Args, template <typename...> typename Carrier>
    requires(sizeof...(Rest) > 0)
    constexpr const auto& first_constructible_of(Carrier<Args...> carrier) {
        using Type =
        ConditionalT<Constructible<T, Args...>, T, RemoveCvRefT<decltype(first_constructible_of<Rest...>(carrier))>>;
        Type* ptr{ nullptr };
        return *ptr;
    }
    template <typename... Types, typename... Args>
    constexpr bool variant_constructible(PackCarrier<Types...>, PackCarrier<Args...>) {
        return (Constructible<Types, Args...> || ...);
    }
    template <typename Type, typename... Types>
    concept VariantAnyOf = IsAnyOfV<Type, Types...>;

    using GenericFnPtr    = void (*)(uint8_t*);
    using GenericBiFnPtr  = void (*)(uint8_t*, uint8_t*);
    using GenericCBiFnPtr = void (*)(uint8_t*, const uint8_t*);
    using CmpFnPtr        = Ordering (*)(const uint8_t*, const uint8_t*);
    template <typename T>
    void generic_type_destructor(uint8_t* memory) {
        T& obj = *reinterpret_cast<T*>(memory);
        obj.~T();
    }
    template <typename T>
    void generic_type_copy_ctor(uint8_t* dst, const uint8_t* src) {
        const T& srcobj = *reinterpret_cast<const T*>(src);
        new (dst) T{ srcobj };
    }
    template <typename T>
    void generic_type_move_ctor(uint8_t* dst, uint8_t* src) {
        T& srcobj = *reinterpret_cast<T*>(src);
        new (dst) T{ move(srcobj) };
    }
    template <typename T>
    Ordering generic_cmp_func(const uint8_t* lhs, const uint8_t* rhs) {
        if constexpr (Orderable<T>) {
            const T& lhsobj = *reinterpret_cast<const T*>(lhs);
            const T& rhsobj = *reinterpret_cast<const T*>(rhs);
            return lhsobj <=> rhsobj;
        } else {
            return unordered;
        }
    }
    struct VariantTypeFns {
        GenericFnPtr destructor;
        GenericCBiFnPtr copy_ctor;
        GenericBiFnPtr move_ctor;
        CmpFnPtr cmp_func;
    };
    template <typename T>
    constexpr VariantTypeFns var_fns_creator() {
        return VariantTypeFns{ .destructor = generic_type_destructor<T>,
                               .copy_ctor  = generic_type_copy_ctor<T>,
                               .move_ctor  = generic_type_move_ctor<T>,
                               .cmp_func   = generic_cmp_func<T> };
    }
    template <typename... Types>
    requires(sizeof...(Types) >= 0)
    class Variant {
        using VariantArray                = TypeArray<Types...>;
        constexpr static Array functions  = { var_fns_creator<Types>()... };
        constexpr static size_t ntypes    = sizeof...(Types);
        constexpr static size_t max_size  = *max(Array{ sizeof(Types)... });
        constexpr static size_t max_align = *max(Array{ alignof(Types)... });
        VariantStorage<max_size, max_align> m_storage{};
        constexpr static size_t no_type           = static_cast<size_t>(-1);
        constexpr static PackCarrier type_carrier = PackCarrier<Types...>{};
        size_t m_current_type{ no_type };
        template <size_t... Vals>
        void destroy_current() {
            if (m_current_type == no_type) return;
            functions[m_current_type].destructor(m_storage.raw_memory());
        }
        template <typename Callable, typename Type>
        bool visit_if(Callable&& callable) const {
            constexpr size_t index = VariantArray::template IndexOf<Type>;
            if (m_current_type == index) {
                if constexpr (CallableWith<Callable, Type>) {
                    invoke(Forward<Callable>(callable), m_storage.template as<Type>());
                    return true;
                }
            }
            return false;
        }

        public:
        template <typename T>
        requires VariantAnyOf<T, Types...>
        Variant(T&& value) {
            constexpr size_t index = VariantArray::template IndexOf<T>;
            m_current_type         = index;
            m_storage.initialize(Forward<T>(value));
        }
        Variant(const Variant& other) {
            m_current_type = other.m_current_type;
            functions[m_current_type].copy_ctor(m_storage.raw_memory(), other.m_storage.raw_memory());
        }
        Variant(Variant&& other) {
            m_current_type       = other.m_current_type;
            other.m_current_type = no_type;
            functions[m_current_type].move_ctor(m_storage.raw_memory(), other.m_storage.raw_memory());
        }
        Variant& operator=(const Variant& other) {
            destroy_current();
            m_current_type = other.m_current_type;
            functions[m_current_type].copy_ctor(m_storage.raw_memory(), other.m_storage.raw_memory());
            return *this;
        }
        Variant& operator=(Variant&& other) {
            destroy_current();
            m_current_type = other.m_current_type;
            functions[m_current_type].move_ctor(m_storage.raw_memory(), other.m_storage.raw_memory());
            return *this;
        }
        template <typename... Args>
        requires(variant_constructible(type_carrier, PackCarrier<Args...>{}))
        Variant(Args&&... args) {
            using Type = RemoveCvRefT<decltype(first_constructible_of<Types...>(PackCarrier<Args...>{}))>;
            static_assert(!SameAs<Type, InvalidConstructionType>, "Args can't construct any type in variant");
            constexpr size_t index = VariantArray::template IndexOf<Type>;
            m_current_type         = index;
            m_storage.template initialize<Type>(Type{ Forward<Args>(args)... });
        }
        template <typename T>
        requires VariantAnyOf<T, Types...>
        Variant& operator=(T&& value) {
            constexpr size_t index = VariantArray::template IndexOf<T>;
            destroy_current();
            m_current_type = index;
            m_storage.template initialize<T>(Forward<T>(value));
            return *this;
        }
        template <typename... Args>
        requires(variant_constructible(type_carrier, PackCarrier<Args...>{}))
        Variant& operator=(Args&&... args) {
            using Type = RemoveCvRefT<decltype(first_constructible_of<Types...>(PackCarrier<Args...>{}))>;
            static_assert(!SameAs<Type, InvalidConstructionType>, "Args can't construct any type in variant");
            constexpr size_t index = VariantArray::template IndexOf<Type>;
            destroy_current();
            m_current_type = index;
            m_storage.template initialize<Type>(Type{ Forward<Args>(args)... });
            return *this;
        }
        template <typename T>
        requires VariantAnyOf<T, Types...>
        const T& get() const {
            constexpr size_t index = VariantArray::template IndexOf<T>;
            HARD_ASSERT(index == m_current_type, "get<>() in variant was trying to access not current type");
            return m_storage.template as<T>();
        }
        template <typename T>
        requires VariantAnyOf<T, Types...>
        T& get() {
            constexpr size_t index = VariantArray::template IndexOf<T>;
            HARD_ASSERT(index == m_current_type, "get<>() in variant was trying to access not current type");
            return m_storage.template as<T>();
        }
        template <size_t Index>
        requires(Index < ntypes)
        const auto& get() const {
            using T = VariantArray::template At<Index>;
            HARD_ASSERT(Index == m_current_type, "get<>() in variant was trying to access not current type");
            return m_storage.template as<T>();
        }
        template <size_t Index>
        requires(Index < ntypes)
        auto& get() {
            using T = VariantArray::template At<Index>;
            HARD_ASSERT(Index == m_current_type, "get<>() in variant was trying to access not current type");
            return m_storage.template as<T>();
        }
        template <typename... Args>
        requires(variant_constructible(type_carrier, PackCarrier<Args...>{}))
        void set(Args&&... args) {
            using Type = RemoveCvRefT<decltype(first_constructible_of<Types...>(PackCarrier<Args...>{}))>;
            static_assert(!SameAs<Type, InvalidConstructionType>, "Args can't construct any type in variant");
            constexpr size_t index = VariantArray::template IndexOf<Type>;
            destroy_current();
            m_current_type = index;
            m_storage.template initialize<Type>(Type{ Forward<Args>(args)... });
        }
        template <size_t Index, typename... Args>
        requires(Index < ntypes)
        void set(Args&&... args) {
            using Type = VariantArray::template At<Index>;
            static_assert(Constructible<Type, Args...>, "Args can't construct type at index passed");
            destroy_current();
            m_current_type = Index;
            m_storage.template initialize<Type>(Type{ Forward<Args>(args)... });
        }
        template <typename T>
        bool contains_type() const {
            constexpr size_t index = VariantArray::template IndexOf<T>;
            return m_current_type == index;
        }
        size_t current_type() const { return m_current_type; }
        bool is_empty() const { return m_current_type == no_type; }
        bool is_active() const { return m_current_type != no_type; }
        Ordering operator<=>(const Variant& other) const
        requires(... && Orderable<Types>)
        {
            if (other.m_current_type != m_current_type) return unordered;
            return functions[m_current_type].cmp_func(m_storage.raw_memory(), other.m_storage.raw_memory());
        }
        template <typename Callable>
        requires(CallableWith<Callable, Types> || ...)
        void visit(Callable&& visitor) const {
            (visit_if<Callable, Types>(Forward<Callable>(visitor)) || ...);
        }
        ~Variant() { destroy_current(); }
    };
}    // namespace v2
}    // namespace ARLib