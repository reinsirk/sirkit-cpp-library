// SPDX-License-Identifier: CC0-1.0
#include "../Math/Pollard_Rho.cpp"
#include <iostream>
int main() {
    PollardRho64 engine;
    for (const auto& [prime, exponent] : engine.factorize(360))
        std::cout << prime << '^' << exponent << ' ';
    std::cout << '\n';
    for (auto divisor : engine.divisors(36)) std::cout << divisor << ' ';
    std::cout << '\n';
    std::cout << engine.totient(36) << ' ' << engine.mobius(30) << ' ' << engine.divisor_count(36) << '\n';
    std::cout << PollardRho64::is_prime(998244353) << ' ' << EulerPhi64(17) << '\n';
}
