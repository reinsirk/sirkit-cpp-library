// SPDX-License-Identifier: CC0-1.0
#include "../../../String/Prefix_Bitmap_Table.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <random>

static std::size_t brute(const std::vector<std::string> &rows, std::string_view prefix)
{
    return static_cast<std::size_t>(std::count_if(rows.begin(), rows.end(), [prefix](std::string_view row) {
        return row.starts_with(prefix);
    }));
}

int main()
{
    const sirkit::PrefixBitmapTable<> vacant;
    assert(vacant.size() == 0 && vacant.width() == 0);
    assert(vacant.prefix_count("") == 0 && vacant.prefix_count("x") == 0);
    const std::vector<std::string> no_rows;
    const sirkit::PrefixBitmapTable<> no_data(no_rows);
    assert(no_data.size() == 0 && no_data.width() == 0);
    const std::array<std::string_view, 2> empty_rows{"", ""};
    const sirkit::PrefixBitmapTable<> zero_width(empty_rows);
    assert(zero_width.size() == 2 && zero_width.width() == 0 && zero_width.prefix_count("") == 2);

    std::vector<std::string> data{"aaa", "aba", "bbb", "ccc"};
    sirkit::PrefixBitmapTable<> table(data);
    assert(table.size() == 4 && table.width() == 3);
    assert(table.prefix_count("aa") == 1 && table.prefix_count("") == 4);
    assert(table.prefix_count("aaaa") == 0 && table.prefix_count("a!") == 0);
    assert(table.prefix_count("aZ") == 0 && table.prefix_count("z") == 0);
    table.replace(1, 'b', 'a');
    assert(table.prefix_count("aa") == 2 && table.prefix_count("baa") == 0);
    table.replace(1, 'a', 'a');
    table.replace(1, 'z', 'a');
    assert(table.prefix_count("aa") == 2);
    data[0].assign("ccc");
    assert(table.prefix_count("aa") == 2);

    auto copy = table;
    copy.replace(0, 'a', 'b');
    assert(copy.prefix_count("aa") == 0 && table.prefix_count("aa") == 2);
    auto moved = std::move(copy);
    assert(copy.size() == 0 && copy.width() == 0 && copy.prefix_count("") == 0);
    assert(moved.prefix_count("ba") == 3);
    copy = table;
    assert(copy.prefix_count("aa") == 2);
    copy = std::move(moved);
    assert(moved.size() == 0 && copy.prefix_count("ba") == 3);
    auto &alias = copy;
    copy = std::move(alias);
    assert(copy.prefix_count("ba") == 3);

    const char nul = 0;
    const char high = static_cast<char>(255);
    const std::string key{nul, high};
    const std::array<std::string_view, 3> binary_rows{key, key, "az"};
    sirkit::PrefixBitmapTable<256, 0> byte_table(binary_rows);
    assert(byte_table.prefix_count(key) == 2);
    byte_table.replace(0, 0, 255);
    assert(byte_table.prefix_count(key) == 0);
    assert(byte_table.prefix_count(std::string{high, high}) == 2);
    const std::array<std::string_view, 2> fixed_rows{"01", "11"};
    sirkit::PrefixBitmapTable<2, '0'> binary(fixed_rows);
    assert(binary.prefix_count("00") == 0 && binary.prefix_count("0") == 1);
    binary.replace(1, '1', '0');
    assert(binary.prefix_count("00") == 1 && binary.prefix_count("10") == 1);

#ifndef SKIP_RUNTIME_CHECKS
    bool ragged = false;
    try
    {
        const std::vector<std::string> broken{"a", "aa"};
        (void)sirkit::PrefixBitmapTable<>(broken);
    }
    catch (const std::invalid_argument &)
    {
        ragged = true;
    }
    assert(ragged);
    bool bad_symbol = false;
    try
    {
        const std::vector<std::string> broken{"xY"};
        (void)sirkit::PrefixBitmapTable<>(broken);
    }
    catch (const std::out_of_range &)
    {
        bad_symbol = true;
    }
    assert(bad_symbol);
    bool bad_column = false;
    try
    {
        table.replace(3, 'a', 'b');
    }
    catch (const std::out_of_range &)
    {
        bad_column = true;
    }
    assert(bad_column && table.prefix_count("aa") == 2);
    bool invalid_replacement = false;
    try
    {
        table.replace(0, 'a', 'B');
    }
    catch (const std::out_of_range &)
    {
        invalid_replacement = true;
    }
    assert(invalid_replacement && table.prefix_count("aa") == 2);
#endif
    bool oversized = false;
    try
    {
        (void)sirkit::detail::PrefixBitmapStorageSize(9, 26, 2, 100);
    }
    catch (const std::length_error &)
    {
        oversized = true;
    }
    assert(oversized);

    std::mt19937 rng(6967);
    for (int trial = 0; trial < 300; ++trial)
    {
        const auto n = rng() % 144;
        const std::size_t width = 1 + rng() % 12;
        std::vector<std::string> rows(n, std::string(width, 'a'));
        for (auto &row : rows)
        {
            for (auto &c : row)
            {
                c = static_cast<char>('a' + rng() % 4);
            }
        }
        sirkit::PrefixBitmapTable<> fast(rows);
        for (int query = 0; query < 100; ++query)
        {
            if (n != 0 && query % 3 == 0)
            {
                const auto column = rng() % width;
                const char from = static_cast<char>('a' + rng() % 4);
                const char to = static_cast<char>('a' + rng() % 4);
                fast.replace(column, from, to);
                for (auto &row : rows)
                {
                    if (row[column] == from)
                    {
                        row[column] = to;
                    }
                }
            }
            else
            {
                std::string prefix;
                if (!rows.empty())
                {
                    prefix = rows[rng() % n].substr(0, rng() % (width + 1));
                }
                if (query % 5 == 0)
                {
                    prefix += 'z';
                }
                assert(fast.prefix_count(prefix) == brute(rows, prefix));
            }
        }
    }

    for (const std::size_t n : {63, 64, 65, 127, 128, 129})
    {
        const std::vector<std::string> rows(n, "abc");
        sirkit::PrefixBitmapTable<> bits(rows);
        assert(bits.prefix_count("abc") == n);
        bits.replace(0, 'a', 'c');
        assert(bits.prefix_count("cbc") == n && bits.prefix_count("abc") == 0);
    }
}
