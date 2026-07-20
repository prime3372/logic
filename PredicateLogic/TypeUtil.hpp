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


template <class P, class x, class t>
class ReplaceType_Class;

template <class P, class x, class t>
using ReplaceType = typename ReplaceType_Class<P, x, t>::type;

template <class Tuple, class x, class t, class Is = std::make_index_sequence<std::tuple_size_v<Tuple>>>
class ReplaceTypeHelper_Class {
    static_assert(false);
};

template <class Tuple, class x, class t, size_t... Is>
class ReplaceTypeHelper_Class<Tuple, x, t, std::index_sequence<Is...>> {
public:
    using type =
        std::tuple<
            ReplaceType<std::tuple_element_t<Is, Tuple>, x, t>...
        >;
};

template <class Tuple, class x, class t>
using ReplaceTypeHelper = ReplaceTypeHelper_Class<Tuple, x, t>::type;

template <class P, class x, class t>
class ReplaceType_Class {
public:
    using type =
        std::conditional_t<
            std::same_as<P, x>,
            t,
            ApplyType<
                ReplaceTypeHelper<typename P::TemplateArgs, x, t>,
                P::template Template
            >
        >;
};