#include "../Algorithm/Clamped_Translation.cpp"

#include <cassert>

int main()
{
    sirkit::ClampedTranslation filter;
    filter.then_add(3).then_chmax(0).then_chmin(10);
    assert(filter.apply(-6) == 0);
    assert(filter.apply(1) == 4);
    assert(filter.apply(20) == 10);
}
