#pragma once
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#include <concepts>
#include <cstddef>
#include <type_traits>

class PropBase {
protected:
    consteval PropBase() {}

    template <class P>
    static constexpr P fetch = P();
};

template <class P>
concept PropType = std::is_base_of_v<PropBase, P>;

class False final : public PropBase {
public:
    consteval False(const False& other) {
        if (!other.auth) throw; // prevent illegal construction
        auth = true;
    }

    template <PropType P>
    consteval P explode() const { return PropBase::fetch<P>; }

private:
    friend class PropBase;
    consteval False() : auth(true) {}

    bool auth = false;
};

template <PropType P>
class Not final : public PropBase {
public:
    consteval Not(auto f) {
        False q = f(PropBase::fetch<P>);
        auth = true;
    }
    consteval Not(const Not& other) {
        if (!other.auth) throw; // prevent illegal construction
        auth = true;
    }

    consteval False elim(P) const { return PropBase::fetch<False>; }

private:
    friend class PropBase;
    consteval Not() : auth(true) {}

    bool auth = false;
};

template <PropType P, PropType Q>
class And final : public PropBase {
public:
    consteval And(P, Q) : auth(true) {}
    consteval And(const And& other) {
        if (!other.auth) throw; // prevent illegal construction
        auth = true;
    }

    const P left = PropBase::fetch<P>;
    const Q right = PropBase::fetch<Q>;

private:
    friend class PropBase;
    consteval And() : auth(true) {}

    bool auth = false;
};

template <PropType P, PropType Q>
class Or final : public PropBase {
public:
    consteval Or() requires std::same_as<Q, Not<P>> || std::same_as<P, Not<Q>> : auth(true) {}
    consteval Or(P) : auth(true) {}
    consteval Or(Q) requires (!std::same_as<P, Q>) : auth(true) {}
    consteval Or(const Or& other) {
        if (!other.auth) throw; // prevent illegal construction
        auth = true;
    }

    consteval auto elim(auto f, auto g) const {
        auto rf(f(PropBase::fetch<P>));
        auto rg(g(PropBase::fetch<Q>));
        if (!std::same_as<decltype(rf), decltype(rg)>) throw; // the return types of f and g must be the same
        return PropBase::fetch<decltype(rf)>;
    }

private:
    friend class PropBase;
    consteval Or() : auth(true) {}

    bool auth = false;
};

template <PropType P, PropType Q>
class Impl final : public PropBase {
public:
    consteval Impl(auto f) {
        Q q = f(PropBase::fetch<P>);
        auth = true;
    }
    consteval Impl(const Impl& other) {
        if (!other.auth) throw; // prevent illegal construction
        auth = true;
    }

    consteval Q operator()(P) const { return PropBase::fetch<Q>; }

private:
    friend class PropBase;
    consteval Impl() : auth(true) {}

    bool auth = false;
};

template <PropType P, PropType Q>
class Equiv final : public PropBase {
public:
    consteval Equiv(auto f, auto g) {
        Q q = f(PropBase::fetch<P>);
        P p = g(PropBase::fetch<Q>);
        auth = true;
    }
    consteval Equiv(const Equiv& other) {
        if (!other.auth) throw; // prevent illegal construction
        auth = true;
    }

    consteval Q operator()(P) const { return PropBase::fetch<Q>; }
    consteval P operator()(Q) const requires (!std::same_as<P, Q>) { return PropBase::fetch<P>; }

private:
    friend class PropBase;
    consteval Equiv() : auth(true) {}

    bool auth = false;
};

template <size_t id>
class Prop final : public PropBase {
public:
    consteval Prop(const Prop& other) {
        if (!other.auth) throw; // prevent illegal construction
        auth = true;
    }

private:
    friend class PropBase;
    consteval Prop() : auth(true) {}

    bool auth = false;
};