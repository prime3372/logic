#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <tuple>
#include <utility>


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


class PropBase;

template <class P>
concept PropType = std::is_base_of_v<PropBase, P>;

class PropBase {
protected:
    consteval PropBase() {}

    template <PropType P>
    static constexpr P object = P();
};


class VarBase {};

template <class T>
concept VarType = std::is_base_of_v<VarBase, T>;

class BoundVarBase : VarBase {};

template <class T>
concept BoundVarType = std::is_base_of_v<BoundVarBase, T>;

class FreeVarBase : VarBase {};

template <class T>
concept FreeVarType = std::is_base_of_v<FreeVarBase, T>;


class False final : PropBase {
public:
    template <class _ = void>
    using Template = False;
    using TemplateArgs = std::tuple<>;

    consteval False(const False& other) {
        initialized = other.initialized;
    }

    template <PropType P>
    consteval P explode() const {
        return PropBase::object<P>;
    }

    static void* operator new(size_t) = delete;
    static void* operator new[](size_t) = delete;

private:
    friend class PropBase;
    consteval False() : initialized(true) {}

    bool initialized;
};


template <PropType P>
class Not final : PropBase {
public:
    template <PropType T>
    using Template = Not<T>;
    using TemplateArgs = std::tuple<P>;

    consteval Not(auto f) {
        False q = f(PropBase::object<P>);
        initialized = true;
    }
    consteval Not(const Not& other) {
        initialized = other.initialized;
    }

    consteval False operator()(P) const {
        return PropBase::object<False>;
    }

    static void* operator new(size_t) = delete;
    static void* operator new[](size_t) = delete;

private:
    friend class PropBase;
    consteval Not() : initialized(true) {}

    bool initialized;
};


template <PropType P, PropType Q>
class And final : PropBase {
public:
    template <PropType T, PropType U>
    using Template = And<T, U>;
    using TemplateArgs = std::tuple<P, Q>;

    consteval And(P, Q) : initialized(true) {}
    consteval And(const And& other) {
        initialized = other.initialized;
    }

    const P left = PropBase::object<P>;
    const Q right = PropBase::object<Q>;

    static void* operator new(size_t) = delete;
    static void* operator new[](size_t) = delete;

private:
    friend class PropBase;
    consteval And() : initialized(true) {}

    bool initialized;
};


template <PropType P, PropType Q>
class Or final : PropBase {
public:
    template <PropType T, PropType U>
    using Template = Or<T, U>;
    using TemplateArgs = std::tuple<P, Q>;

    consteval Or() requires (std::same_as<Q, Not<P>> || std::same_as<P, Not<Q>>) : initialized(true) {}
    consteval Or(P) : initialized(true) {}
    consteval Or(Q) requires (!std::same_as<P, Q>) : initialized(true) {}
    consteval Or(const Or& other) {
        initialized = other.initialized;
    }

    consteval auto elim(auto f, auto g) const {
        auto rf = f(PropBase::object<P>);
        auto rg = g(PropBase::object<Q>);
        static_assert(std::same_as<decltype(rf), decltype(rg)>);
        return rf;
    }

    static void* operator new(size_t) = delete;
    static void* operator new[](size_t) = delete;

private:
    friend class PropBase;
    consteval Or() : initialized(true) {}

    bool initialized;
};


template <PropType P, PropType Q>
class Impl final : PropBase {
public:
    template <PropType T, PropType U>
    using Template = Impl<T, U>;
    using TemplateArgs = std::tuple<P, Q>;

    consteval Impl(auto f) {
        Q q = f(PropBase::object<P>);
        initialized = true;
    }
    consteval Impl(const Impl& other) {
        initialized = other.initialized;
    }

    consteval Q operator()(P) const {
        return PropBase::object<Q>;
    }

    static void* operator new(size_t) = delete;
    static void* operator new[](size_t) = delete;

private:
    friend class PropBase;
    consteval Impl() : initialized(true) {}

    bool initialized;
};


template <PropType P, PropType Q>
class Equiv final : PropBase {
public:
    template <PropType T, PropType U>
    using Template = Equiv<T, U>;
    using TemplateArgs = std::tuple<P, Q>;

    consteval Equiv(auto f, auto g) {
        Q q = f(PropBase::object<P>);
        P p = g(PropBase::object<Q>);
        initialized = true;
    }
    consteval Equiv(const Equiv& other) {
        initialized = other.initialized;
    }

    consteval Q operator()(P) const {
        return PropBase::object<Q>;
    }

    consteval P operator()(Q) const requires (!std::same_as<P, Q>) {
        return PropBase::object<P>;
    }

    static void* operator new(size_t) = delete;
    static void* operator new[](size_t) = delete;

private:
    friend class PropBase;
    consteval Equiv() : initialized(true) {}

    bool initialized;
};


template <BoundVarType x, PropType P>
class All final : PropBase {
public:
    template <BoundVarType t, PropType T>
    using Template = All<t, T>;
    using TemplateArgs = std::tuple<x, P>;

    consteval All(auto f) {
        auto p = f.template operator()<a>();
        static_assert(std::same_as<decltype(p), ReplaceType<P, x, a>>);
        initialized = true;
    }
    consteval All(const All& other) {
        initialized = other.initialized;
    }

    template <FreeVarType t>
    consteval ReplaceType<P, x, t> elim() const {
        return PropBase::object<ReplaceType<P, x, t>>;
    }

    static void* operator new(size_t) = delete;
    static void* operator new[](size_t) = delete;

private:
    class a final : FreeVarBase {
    public:
        template <class _ = void>
        using Template = a;
        using TemplateArgs = std::tuple<>;
        consteval a() {}
    };

    friend class PropBase;
    consteval All() : initialized(true) {}

    bool initialized;
};


template <BoundVarType x, PropType P>
class Exist final : PropBase {
public:
    template <BoundVarType t, PropType T>
    using Template = Exist<t, T>;
    using TemplateArgs = std::tuple<x, P>;

    template <FreeVarType t>
    consteval Exist(t, ReplaceType<P, x, t>) : initialized(true) {}

    consteval Exist(const Exist& other) {
        initialized = other.initialized;
    }

    consteval auto elim(auto f) const {
        return f.template operator()<a>(PropBase::object<ReplaceType<P, x, a>>);
    }

    static void* operator new(size_t) = delete;
    static void* operator new[](size_t) = delete;

private:
    class a : FreeVarBase {
    public:
        template <class _ = void>
        using Template = a;
        using TemplateArgs = std::tuple<>;
        consteval a() {}
    };

    friend class PropBase;
    consteval Exist() : initialized(true) {}

    bool initialized;
};


template <size_t id, VarType... P>
class Pred final : PropBase {
public:
    template <VarType... Args>
    using Template = Pred<id, Args...>;
    using TemplateArgs = std::tuple<P...>;

    consteval Pred(const Pred& other) {
        initialized = other.initialized;
    }

    static void* operator new(size_t) = delete;
    static void* operator new[](size_t) = delete;

private:
    friend class PropBase;
    consteval Pred() : initialized(true) {}

    bool initialized;
};

template <size_t id>
using Prop = Pred<id>;


template <size_t id>
class Bound final : BoundVarBase {
public:
    template <class _ = void>
    using Template = Bound;
    using TemplateArgs = std::tuple<>;

    consteval Bound() {}
};