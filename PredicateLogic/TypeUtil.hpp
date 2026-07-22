#pragma once

#include <cstddef>
#include <tuple>

template <class T, class U, class V>
class ReplaceTypeImplementation;

template <template <class...> class Template, class TemplateArgs, class U, class V>
class ReplaceTypeHelper {};

template <template <class...> class Template, class U, class V, class... Args>
class ReplaceTypeHelper<Template, std::tuple<Args...>, U, V> {
public:
    using result = Template<typename ReplaceTypeImplementation<Args, U, V>::result...>;
};

template <class T, class U, class V>
class ReplaceTypeImplementation {
public:
    using result =
        std::conditional_t<
            std::same_as<T, U>,
            V,
            typename ReplaceTypeHelper<T::template Template, typename T::TemplateArgs, U, V>::result
        >;
};

template <class T, class U, class V>
using ReplaceType = typename ReplaceTypeImplementation<T, U, V>::result;

