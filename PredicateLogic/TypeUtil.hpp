#pragma once

#include <cstddef>
#include <tuple>

template <class Tuple, template <class...> class Target>
class ApplyType_Class {
    static_assert(false);
};

template <class... Args, template <class...> class Target>
class ApplyType_Class<std::tuple<Args...>, Target> {
public:
    using type = Target<Args...>;
};

template <class Tuple, template <class...> class Target>
using ApplyType = typename ApplyType_Class<Tuple, Target>::type;


template <class T, class U, class V>
class ReplaceType_Class;

template <class T, class U, class V>
using ReplaceType = typename ReplaceType_Class<T, U, V>::type;

template <class Tuple, class U, class V, class Is = std::make_index_sequence<std::tuple_size_v<Tuple>>>
class ReplaceTypeHelper_Class {
    static_assert(false);
};

template <class Tuple, class U, class V, size_t... Is>
class ReplaceTypeHelper_Class<Tuple, U, V, std::index_sequence<Is...>> {
public:
    using type =
        std::tuple<
            ReplaceType<std::tuple_element_t<Is, Tuple>, U, V>...
        >;
};

template <class Tuple, class U, class V>
using ReplaceTypeHelper = ReplaceTypeHelper_Class<Tuple, U, V>::type;

template <class T, class U, class V>
class ReplaceType_Class {
public:
    using type =
        std::conditional_t<
            std::same_as<T, U>,
            V,
            ApplyType<
                ReplaceTypeHelper<typename T::TemplateArgs, U, V>,
                T::template Template
            >
        >;
};