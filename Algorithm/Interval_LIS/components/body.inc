#include <span>

#ifndef SIRKIT_INTERVAL_LIS_DEFINED
#define SIRKIT_INTERVAL_LIS_DEFINED

struct LISInterval
{
    std::int64_t left;
    std::int64_t right;
};

inline std::vector<std::int64_t> interval_lis_tails(std::span<const LISInterval> intervals)
{
    lazy_implicit_treap<> tails(std::numeric_limits<std::int64_t>::max());
    tails.reserve(intervals.size());
    for (const auto &[left, right] : intervals)
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (left > right)
        {
            throw std::invalid_argument("interval_lis_tails: reversed interval");
        }
#endif
        const auto first = tails.lower_bound(left);
        const auto last = tails.lower_bound(right);
        // left未満の末尾にはleftをつなぎ、[left,right)の末尾には1を加えた値をつなぐ。
        // 後者で更新する長さの直後から、従来の末尾を残す。
        if (last < tails.size())
        {
            tails.erase_at(last);
        }
        tails.apply(first, last, 1);
        tails.insert_at(first, left);
    }
    return tails.to_vector();
}

#endif
