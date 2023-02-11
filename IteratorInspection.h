#pragma once
#include "Concepts.h"
namespace ARLib {

template <typename Iterator>
concept IteratorHasIOTypes = requires {
                                 typename Iterator::InputValueType;
                                 typename Iterator::OutputValueType;
                             };
template <typename Iterator>
concept IteratorHasValueType = requires { typename Iterator::ValueType; };
template <typename Iterator, bool HasValueType>
struct IteratorValueType {};
template <typename Iterator>
struct IteratorValueType<Iterator, true> {
    using type = typename Iterator::ValueType;
};
template <typename Iterator>
struct IteratorValueType<Iterator, false> {
    using type = RemoveReferenceT<decltype(*declval<Iterator>())>;
};
template <typename Iterator, bool HasIOTypes>
struct IteratorIOType {};
template <typename Iterator>
struct IteratorIOType<Iterator, true> {
    using itype = typename Iterator::InputValueType;
    using otype = typename Iterator::OutputValueType;
};
template <typename Iterator>
struct IteratorIOType<Iterator, false> {
    using itype = typename IteratorValueType<Iterator, IteratorHasValueType<Iterator>>::type;
    using otype = typename IteratorValueType<Iterator, IteratorHasValueType<Iterator>>::type;
};
template <typename Iter>
using IteratorInputType = typename IteratorIOType<Iter, IteratorHasIOTypes<Iter>>::itype;

template <typename Iter>
using IteratorOutputType = typename IteratorIOType<Iter, IteratorHasIOTypes<Iter>>::otype;

template <typename Cont>
using ContainerValueTypeT = IteratorInputType<decltype(declval<Cont>().begin())>;

}    // namespace ARLib
