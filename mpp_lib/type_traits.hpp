#ifndef _MPP_LIB_TYPE_TRAITS_HPP_
#define _MPP_LIB_TYPE_TRAITS_HPP_
#include "./defs.hpp"

// TODO: Map compiler's built-in type-traits to MPP_MACRO_COMPILER_TYPE_TRAIT_*
//       If a particular compiler cannot implement it, abort.

MPP_LIB_NAMESPACE_BEGIN

// Type-Traits: integral_constant
template <typename T, T v>
struct integral_constant {
    static constexpr T value = v;
    using value_type = T;
    using type = integral_constant;
    constexpr operator value_type() const noexcept { return value; }
    constexpr value_type operator()() const noexcept { return value; }
};

using true_type  = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;

// Type-Traits: is_union
template <typename T>
struct is_union : integral_constant<bool, __is_union(T)> {};
template <typename T>
inline constexpr bool is_union_v = is_union<T>::value;

// Type-Traits: is_class
template <typename T>
struct is_class : integral_constant<bool, __is_class(T)> {};
template <typename T>
inline constexpr bool is_class_v = is_class<T>::value;

// Type-Traits: is_enum
template <typename T>
struct is_enum : integral_constant<bool, __is_enum(T)> {};
template <typename T>
inline constexpr bool is_enum_v = is_enum<T>::value;

// Type-Traits: is_abstract
template <typename T>
struct is_abstract : integral_constant<bool, __is_abstract(T)> {};
template <typename T>
inline constexpr bool is_abstract_v = is_abstract<T>::value;

// Type-Traits: is_final
template <typename T>
struct is_final : integral_constant<bool, __is_final(T)> {};
template <typename T>
inline constexpr bool is_final_v = is_final<T>::value;

// Type-Traits: remove_const
template <typename T> struct remove_const          { using type = T; };
template <typename T> struct remove_const<const T> { using type = T; };
template <typename T> using remove_const_t = typename remove_const<T>::type;

// Type-Traits: remove_volatile
template <typename T> struct remove_volatile             { using type = T; };
template <typename T> struct remove_volatile<volatile T> { using type = T; };
template <typename T> using remove_volatile_t = typename remove_volatile<T>::type;

// Type-Traits: remove_cv
template <typename T>
struct remove_cv {
    using type = remove_volatile_t<remove_const_t<T>>;
};
template <typename T> using remove_cv_t = typename remove_cv<T>::type;

// Type-Traits: remove_reference
template <typename T> struct remove_reference      { using type = T; };
template <typename T> struct remove_reference<T&>  { using type = T; };
template <typename T> struct remove_reference<T&&> { using type = T; };
template <typename T> using remove_reference_t = typename remove_reference<T>::type;

// Type-Traits: is_same
template <typename T, typename U> struct is_same : false_type {};
template <typename T>             struct is_same<T, T> : true_type {};
template <typename T, typename U> inline constexpr bool is_same_v = is_same<T, U>::value;

// Type-Traits-Helper: is_pointer_helper
template <typename T> struct is_pointer_helper     : false_type {};
template <typename T> struct is_pointer_helper<T*> : true_type {};

// Type-Trait: is_pointer
template <typename T> 
struct is_pointer : is_pointer_helper<remove_cv_t<T>> {};
template <typename T> inline constexpr bool is_pointer_v = is_pointer<T>::value;

// Type-Traits: is_base_of
template <typename Base, typename Derived>
struct is_base_of : integral_constant<bool, __is_base_of(Base, Derived)> {};
template <typename Base, typename Derived>
inline constexpr bool is_base_of_v = is_base_of<Base, Derived>::value;


// Type-Traits: conditional
template <bool B, typename T, typename F> struct conditional { using type = T; };
template <typename T, typename F>        struct conditional<false, T, F> { using type = F; };
template <bool B, typename T, typename F> using conditional_t = typename conditional<B, T, F>::type;

// Type-Traits: enable_if
template <bool B, typename T = void> struct enable_if {};
template <typename T>                struct enable_if<true, T> { using type = T; };
template <bool B, typename T = void> using enable_if_t = typename enable_if<B, T>::type;

// Type-Traits-Helper: declval
template <typename T>
typename remove_reference<T>::type&& declval() noexcept;

// Type-Traits: is_lvalue_reference
template <typename T> struct is_lvalue_reference      : false_type {};
template <typename T> struct is_lvalue_reference<T&>  : true_type {};

// Type-Traits: is_rvalue_reference
template <typename T> struct is_rvalue_reference      : false_type {};
template <typename T> struct is_rvalue_reference<T&&> : true_type {};

// Type-Traits: is_lvalue
// @note Pass decltype((expression)) as trait
template <typename T>
struct is_lvalue : is_lvalue_reference<T> {};

// @note Pass decltype((expression)) as trait
template <typename T>
inline constexpr bool is_lvalue_v = is_lvalue<T>::value;

// Type-Traits: is_xvalue
// @note Pass decltype((expression)) as trait
template <typename T>
struct is_xvalue : is_rvalue_reference<T> {};

// @note Pass decltype((expression)) as trait
template <typename T>
inline constexpr bool is_xvalue_v = is_xvalue<T>::value;

// Type-Traits: is_prvalue
// @note Pass decltype((expression)) as trait
template <typename T>
struct is_prvalue : integral_constant<bool, !is_lvalue_reference<T>::value && !is_rvalue_reference<T>::value> {};

// @note Pass decltype((expression)) as trait
template <typename T>
inline constexpr bool is_prvalue_v = is_prvalue<T>::value;

// Type-Traits: is_rvalue
// @note Pass decltype((expression)) as trait
template <typename T>
struct is_rvalue : integral_constant<bool, is_prvalue<T>::value || is_xvalue<T>::value> {};

// @note Pass decltype((expression)) as trait
template <typename T>
inline constexpr bool is_rvalue_v = is_rvalue<T>::value;

// Type-Trait-Helper: move
template <typename T>
constexpr remove_reference_t<T>&& move(T&& t) noexcept {
    return static_cast<remove_reference_t<T>&&>(t);
}

/** CPP reference collapsing:

&  (Lvalue)	+ &  (Lvalue) = &  (Lvalue Reference)
&  (Lvalue)	+ && (Rvalue) = &  (Lvalue Reference)
&& (Rvalue)	+ &  (Lvalue) = &  (Lvalue Reference)
&& (Rvalue)	+ && (Rvalue) = && (Rvalue Reference)

Simple trick:
&& is forwarding reference not rvalue reference for types for templates
 */

// Type-Trait-Helper: forward
// Forward an lvalue as an lvalue or rvalue (depending on template parameter T)
template <typename T>
constexpr T&& forward(remove_reference_t<T>& t) noexcept {
    return static_cast<T&&>(t);
}

// Type-Trait-Helper: forward
// Forward an rvalue as an rvalue (prevents accidental lvalue-to-rvalue forwarding)
template <typename T>
constexpr T&& forward(remove_reference_t<T>&& t) noexcept {
    // (e.g. forward<int&>(int&&) is invalid)
    static_assert(!is_lvalue_reference<T>::value, "Cannot forward an rvalue as an lvalue");
    return static_cast<T&&>(t);
}


MPP_LIB_NAMESPACE_END

#endif // _MPP_LIB_TYPE_TRAITS_HPP_

// NOTES:

/**
// Type-Traits: is_constructible
// Checks if T can be constructed from a given set of arguments
template <typename T, typename... Args>
struct is_constructible : integral_constant<bool, __is_constructible(T, Args...)> {};
template <typename T, typename... Args>
inline constexpr bool is_constructible_v = is_constructible<T, Args...>::value;

// Type-Traits: is_trivially_default_constructible
// Checks if T has a trivial default constructor (i.e. it does nothing)
template <typename T>
struct is_trivially_default_constructible : integral_constant<bool, __has_trivial_constructor(T)> {};

// Type-Traits: is_trivially_copy_assignable
template <typename T>
struct is_trivially_copy_assignable : integral_constant<bool, __has_trivial_assign(T)> {};

// Type-Traits: is_trivially_destructible
template <typename T>
struct is_trivially_destructible : integral_constant<bool, __is_trivially_destructible(T)> {};
 */

/**
template <typename...>
using void_t = void;

// Base case: Assume it's not iterable
template <typename T, typename = void>
struct is_iterable : std::false_type {};

// Specialization: Substitution succeeds only if begin() and end() are valid
template <typename T>
struct is_iterable<T, void_t<decltype(std::begin(std::declval<T&>())), 
                             decltype(std::end(std::declval<T&>()))>> 
    : std::true_type {};
 */
