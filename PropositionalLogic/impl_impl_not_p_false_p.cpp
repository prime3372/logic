#include "PropositionalLogic.hpp"

using P = Prop<0>;

consteval Impl<Impl<Not<P>, False>, P> solve() {
    return [=](Impl<Not<P>, False> impl_not_p_false) -> P {
        return Or<P, Not<P>>().elim(
            [=](P p) -> P { return p; },
            [=](Not<P> not_p) -> P { return impl_not_p_false(not_p).explode<P>(); }
        );
    };
}

int main() {
    solve();
}