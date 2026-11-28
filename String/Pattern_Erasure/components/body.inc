#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#ifndef SIRKIT_PATTERN_ERASURE_DEFINED
#define SIRKIT_PATTERN_ERASURE_DEFINED

inline std::size_t erase_pattern_inplace(std::string &text, std::string_view pattern)
{
    const auto length = pattern.size();
    if (length == 0)
    {
        throw std::invalid_argument("erase_pattern: empty pattern");
    }
    const auto input_size = text.size();
    if (length > input_size)
    {
        return 0;
    }

    // 書き換えるtext内を指すpatternも、先にコピーして参照先を保つ。
    const std::string needle(pattern);
    const auto border = prefix_function(needle);
    std::vector<std::size_t> states(input_size);
    std::size_t write = 0, matched = 0;
    for (std::size_t read = 0; read < input_size; ++read)
    {
        const char byte = text[read];
        while (matched != 0 && byte != needle[matched])
        {
            matched = border[matched - 1];
        }
        matched += byte == needle[matched];
        if (matched == length)
        {
            write -= length - 1;
            matched = write == 0 ? 0 : states[write - 1];
        }
        else
        {
            text[write] = byte;
            states[write] = matched;
            ++write;
        }
    }
    text.resize(write);
    return (input_size - write) / length;
}

[[nodiscard]] inline std::string erase_pattern(std::string_view text, std::string_view pattern)
{
    std::string result(text);
    erase_pattern_inplace(result, pattern);
    return result;
}

#endif
