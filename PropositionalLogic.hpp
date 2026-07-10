#pragma once
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#include <concepts>
#include <cstddef>
#include <type_traits>

// 1. law of excluded middle
// 2. principle of explosion
// 3. ¬ intro
// 4. ¬ elim
// 5. ∧ intro
// 6. ∧ elim
// 7. ∨ intro
// 8. ∨ elim
// 9. → intro
// 10. → elim

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
    consteval P explode() const { return PropBase::fetch<P>; } // principle of explosion

private:
    friend class PropBase;
    consteval False() : auth(true) {}

    bool auth = false;
};

template <PropType P>
class Not final : public PropBase {
public:
    consteval Not(auto f) { // ¬ intro
        False q = f(PropBase::fetch<P>);
        auth = true;
    }
    consteval Not(const Not& other) {
        if (!other.auth) throw; // prevent illegal construction
        auth = true;
    }

    consteval False elim(P) const { return PropBase::fetch<False>; } // ¬ elim

private:
    friend class PropBase;
    consteval Not() : auth(true) {}

    bool auth = false;
};

template <PropType P, PropType Q>
class And final : public PropBase {
public:
    consteval And(P, Q) : auth(true) {} // ∧ intro
    consteval And(const And& other) {
        if (!other.auth) throw; // prevent illegal construction
        auth = true;
    }

    consteval P left() { return PropBase::fetch<P>; } // ∧ elim
    consteval Q right() { return PropBase::fetch<Q>; } // ∧ elim

private:
    friend class PropBase;
    consteval And() : auth(true) {}

    bool auth = false;
};

template <PropType P, PropType Q>
class Or final : public PropBase {
public:
    consteval Or() requires std::same_as<Q, Not<P>> || std::same_as<P, Not<Q>> : auth(true) {} // law of excluded middle
    consteval Or(P) : auth(true) {} // ∨ intro
    consteval Or(Q) requires (!std::same_as<P, Q>) : auth(true) {} // ∨ intro
    consteval Or(const Or& other) {
        if (!other.auth) throw; // prevent illegal construction
        auth = true;
    }

    consteval auto elim(auto f, auto g) const { // ∨ elim
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
    consteval Impl(auto f) { // → intro
        Q q = f(PropBase::fetch<P>);
        auth = true;
    }
    consteval Impl(const Impl& other) {
        if (!other.auth) throw; // prevent illegal construction
        auth = true;
    }

    consteval Q operator()(P p) const { return PropBase::fetch<Q>; } // → elim

private:
    friend class PropBase;
    consteval Impl() : auth(true) {}

    bool auth = false;
};

template <PropType P, PropType Q>
using Equiv = And<Impl<P, Q>, Impl<Q, P>>;

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