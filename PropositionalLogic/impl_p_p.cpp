#include "PropositionalLogic.hpp"

using P = Prop<0>;

consteval Impl<P, P> solve() {
    return [&](P p) -> P { return p; };
};

int main() {
    solve();
}