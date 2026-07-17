#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>

class PropBase;

template <class P>
concept PropType = std::is_base_of_v<PropBase, P>;

class PropBase {
protected:
    consteval PropBase() {}

    template <PropType P>
    static constexpr P object = P();
};


class False final : PropBase {
public:
    consteval False(const False& other) {
        initialized = other.initialized;
    }

    template <PropType P>
    consteval P explode() const { return PropBase::object<P>; }

private:
    friend class PropBase;
    consteval False() : initialized(true) {}

    bool initialized;
};


template <PropType P>
class Not final : PropBase {
public:
    consteval Not(auto f) {
        False q = f(PropBase::object<P>);
        initialized = true;
    }
    consteval Not(const Not& other) {
        initialized = other.initialized;
    }

    consteval False operator()(P) const { return PropBase::object<False>; }

private:
    friend class PropBase;
    consteval Not() : initialized(true) {}

    bool initialized;
};


template <PropType P, PropType Q>
class And final : PropBase {
public:
    consteval And(P, Q) : initialized(true) {}
    consteval And(const And& other) {
        initialized = other.initialized;
    }

    const P left = PropBase::object<P>;
    const Q right = PropBase::object<Q>;

private:
    friend class PropBase;
    consteval And() : initialized(true) {}

    bool initialized;
};


template <PropType P, PropType Q>
class Or final : PropBase {
public:
    consteval Or() requires (std::same_as<Q, Not<P>> || std::same_as<P, Not<Q>>) : initialized(true) {}
    consteval Or(P) : initialized(true) {}
    consteval Or(Q) requires (!std::same_as<P, Q>) : initialized(true) {}
    consteval Or(const Or& other) {
        initialized = other.initialized;
    }

    consteval auto elim(auto f, auto g) const {
        auto rf(f(PropBase::object<P>));
        auto rg(g(PropBase::object<Q>));
        static_assert(std::same_as<decltype(rf), decltype(rg)>);
        return PropBase::object<decltype(rf)>;
    }

private:
    friend class PropBase;
    consteval Or() : initialized(true) {}

    bool initialized;
};


template <PropType P, PropType Q>
class Impl final : PropBase {
public:
    consteval Impl(auto f) {
        Q q = f(PropBase::object<P>);
        initialized = true;
    }
    consteval Impl(const Impl& other) {
        initialized = other.initialized;
    }

    consteval Q operator()(P) const { return PropBase::object<Q>; }

private:
    friend class PropBase;
    consteval Impl() : initialized(true) {}

    bool initialized;
};


template <PropType P, PropType Q>
class Equiv final : PropBase {
public:
    consteval Equiv(auto f, auto g) {
        Q q = f(PropBase::object<P>);
        P p = g(PropBase::object<Q>);
        initialized = true;
    }
    consteval Equiv(const Equiv& other) {
        initialized = other.initialized;
    }

    consteval Q operator()(P) const { return PropBase::object<Q>; }

    consteval P operator()(Q) const requires (!std::same_as<P, Q>) { return PropBase::object<P>; }

private:
    friend class PropBase;
    consteval Equiv() : initialized(true) {}

    bool initialized;
};


template <size_t id>
class Prop final : PropBase {
public:
    consteval Prop(const Prop& other) {
        initialized = other.initialized;
    }

private:
    friend class PropBase;
    consteval Prop() : initialized(true) {}

    bool initialized;
};