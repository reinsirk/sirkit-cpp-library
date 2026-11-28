#include "../Math/Poisson_Binomial.cpp"

#include <array>
#include <cassert>
#include <cmath>

int main()
{
    const std::array probabilities{0.30, 0.60, 0.80};
    const auto chance = sirkit::poisson_binomial_at_least(probabilities, 2);
    assert(std::abs(chance - 0.612) < 1e-12);
}
