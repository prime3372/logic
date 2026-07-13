#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <tuple>
#include <utility>

#include "TypeUtil.hpp"

class PropBase;

template <class P>
concept PropType = std::is_base_of_v<PropBase, P>;

class TermBase {};

template <class T>
concept TermType = std::is_base_of_v<TermBase, T>;

class BoundTermBase : TermBase {};

template <class T>
concept BoundTermType = std::is_base_of_v<BoundTermBase, T>;

class FreeTermBase : TermBase {};

template <class T>
concept FreeTermType = std::is_base_of_v<FreeTermBase, T>;

template <class T>
concept arbitrary = FreeTermType<T>;

template <class T>
concept certain = FreeTermType<T>;

class PropBase {
protected:
    consteval PropBase() {}

    template <PropType P>
    static constexpr P object = P();
};


class False final : PropBase {
public:
    template <class _ = void>
    using Template = False;
    using TemplateArgs = std::tuple<>;

    consteval False(const False& other) {
        if (!other.initialized) throw; // prevent illegal initialization
        initialized = true;
    }

    template <PropType P>
    consteval P explode() const { return PropBase::object<P>; }

private:
    friend class PropBase;
    consteval False() : initialized(true) {}

    bool initialized = false;
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
        if (!other.initialized) throw; // prevent illegal initialization
        initialized = true;
    }

    consteval False elim(P) const { return PropBase::object<False>; }

private:
    friend class PropBase;
    consteval Not() : initialized(true) {}

    bool initialized = false;
};


template <PropType P, PropType Q>
class And final : PropBase {
public:
    template <PropType T, PropType U>
    using Template = And<T, U>;
    using TemplateArgs = std::tuple<P, Q>;


    consteval And(P, Q) : initialized(true) {}
    consteval And(const And& other) {
        if (!other.initialized) throw; // prevent illegal initialization
        initialized = true;
    }

    const P left = PropBase::object<P>;
    const Q right = PropBase::object<Q>;

private:
    friend class PropBase;
    consteval And() : initialized(true) {}

    bool initialized = false;
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
        if (!other.initialized) throw; // prevent illegal initialization
        initialized = true;
    }

    consteval auto elim(auto f, auto g) const {
        auto rf(f(PropBase::object<P>));
        auto rg(g(PropBase::object<Q>));
        if (!std::same_as<decltype(rf), decltype(rg)>) throw;
        return PropBase::object<decltype(rf)>;
    }

private:
    friend class PropBase;
    consteval Or() : initialized(true) {}

    bool initialized = false;
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
        if (!other.initialized) throw; // prevent illegal initialization
        initialized = true;
    }

    consteval Q operator()(P) const { return PropBase::object<Q>; }

private:
    friend class PropBase;
    consteval Impl() : initialized(true) {}

    bool initialized = false;
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
        if (!other.initialized) throw; // prevent illegal initialization
        initialized = true;
    }

    consteval Q operator()(P) const { return PropBase::object<Q>; }
    consteval P operator()(Q) const requires (!std::same_as<P, Q>) { return PropBase::object<P>; }

private:
    friend class PropBase;
    consteval Equiv() : initialized(true) {}

    bool initialized = false;
};


template <size_t id, TermType... P>
class Prop final : PropBase {
public:
    template <TermType... Args>
    using Template = Prop<id, Args...>;
    using TemplateArgs = std::tuple<P...>;

    consteval Prop(const Prop& other) {
        if (!other.initialized) throw; // prevent illegal initialization
        initialized = true;
    }

private:
    friend class PropBase;
    consteval Prop() : initialized(true) {}

    bool initialized = false;
};


template <BoundTermType x, PropType P>
class All final : PropBase {
public:
    template <BoundTermType t, PropType T>
    using Template = All<t, T>;
    using TemplateArgs = std::tuple<x, P>;

    consteval All(auto f) {
        auto p(f.template operator()<t>());
        if (!std::same_as<decltype(p), ReplaceType<P, x, t>>) throw;
        initialized = true;
    }

    consteval All(const All& other) {
        if (!other.initialized) throw; // prevent illegal initialization
        initialized = true;
    }

    template <FreeTermType a>
    consteval ReplaceType<P, x, a> elim(a) { return PropBase::object<ReplaceType<P, x, a>>; }

private:
    class t final : FreeTermBase {
    public:
        template <class _ = void>
        using Template = t;
        using TemplateArgs = std::tuple<>;
        consteval t() {}
    };

    friend class PropBase;
    consteval All() : initialized(true) {}

    bool initialized = false;
};


template <BoundTermType x, PropType P>
class Exist final : PropBase {
public:
    template <BoundTermType t, PropType T>
    using Template = Exist<t, T>;
    using TemplateArgs = std::tuple<x, P>;

    template <FreeTermType a>
    consteval Exist(a, ReplaceType<P, x, a>) : initialized(true) {}

    consteval Exist(const Exist& other) {
        if (!other.initialized) throw; // prevent illegal initialization
        initialized = true;
    }

    consteval auto elim(auto f) {
        auto q(f.template operator()<t>(PropBase::object<ReplaceType<P, x, t>>));
        return PropBase::object<decltype(q)>;
    }

private:
    class t : FreeTermBase {
    public:
        template <class _ = void>
        using Template = t;
        using TemplateArgs = std::tuple<>;
        consteval t() {}
    };

    friend class PropBase;
    consteval Exist() : initialized(true) {}

    bool initialized = false;
};


template <size_t id>
class Bound final : BoundTermBase {
public:
    template <class _ = void>
    using Template = Bound;
    using TemplateArgs = std::tuple<>;
    consteval Bound() {}
};

class Free final : FreeTermBase {
public:
    template <class _ = void>
    using Template = Free;
    using TemplateArgs = std::tuple<>;
    consteval Free() {}
};