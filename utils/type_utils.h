#pragma once

#include <memory>
#include <type_traits>

namespace gunit {

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template <typename... Args>
struct TypeList {};

namespace utils {

template <template <typename...> class VariadicType, typename TypeList>
struct ApplyTypeListImpl;

template <template <typename...> class VariadicType, typename... Types>
struct ApplyTypeListImpl<VariadicType, TypeList<Types...>> {
  using Result = VariadicType<Types...>;
};

template <template <typename...> class VariadicType, typename TypeList>
using ApplyTypeList =
    typename ApplyTypeListImpl<VariadicType, TypeList>::Result;

template <typename... Types>
struct ToTypeListImpl {
  using Result = TypeList<Types...>;
};

template <typename... Types>
struct ToTypeListImpl<TypeList<Types...>> {
  using Result = TypeList<Types...>;
};

template <typename... Types>
using ToTypeList = typename ToTypeListImpl<Types...>::Result;

template <typename TypeList1, typename TypeList2>
struct ConcatTypeListsImpl;

template <typename... Types1, typename... Types2>
struct ConcatTypeListsImpl<TypeList<Types1...>, TypeList<Types2...>> {
  using Result = TypeList<Types1..., Types2...>;
};

template <typename TypeList1, typename TypeList2>
using ConcatTypeLists =
    typename ConcatTypeListsImpl<TypeList1, TypeList2>::Result;

template <typename TypeList>
struct GetTypeListSizeImpl;

template <typename... Types>
struct GetTypeListSizeImpl<TypeList<Types...>> {
  static constexpr unsigned int Value = sizeof...(Types);
};

template <typename... Types>
constexpr unsigned int getTypeListSize() {
  return GetTypeListSizeImpl<ToTypeList<Types...>>::Value;
}

template <typename Type, unsigned int CurHeadIndex, typename TypeList>
struct GetIndexInTypeListImpl;

template <typename Type, unsigned int CurHeadIndex>
struct GetIndexInTypeListImpl<Type, CurHeadIndex, TypeList<>> {
  static constexpr unsigned int Value = CurHeadIndex;
};

template <typename Type,
          unsigned int CurHeadIndex,
          typename Head,
          typename... Tail>
struct GetIndexInTypeListImpl<Type, CurHeadIndex, TypeList<Head, Tail...>> {
  static constexpr unsigned int Value =
      std::is_same<Type, Head>::value
          ? CurHeadIndex
          : GetIndexInTypeListImpl<Type, CurHeadIndex + 1, TypeList<Tail...>>::
                Value;
};

template <typename Type, typename... Types>
constexpr unsigned int getIndexInTypeList() {
  return GetIndexInTypeListImpl<Type, 0, ToTypeList<Types...>>::Value;
}

template <typename Type, typename TypeList>
constexpr bool isTypeNotInTypeList() {
  return GetIndexInTypeListImpl<std::decay_t<Type>, 0, TypeList>::Value >=
         GetTypeListSizeImpl<TypeList>::Value;
}

template <typename Type, typename... Types>
constexpr bool isTypeInTypeList() {
  return getIndexInTypeList<Type, Types...>() < getTypeListSize<Types...>();
}

}  // namespace utils
}  // namespace gunit
