# SPDX-License-Identifier: CC0-1.0
"""Independent checks restricted to the small sequence workload fixtures."""
from collections import deque
from itertools import combinations, permutations, product


def _bytes_as_ints(row):
    return [int(word) for word in row.split()]


def _answers(values):
    return " ".join(map(str, values))


def _interval_schedule(rows):
    n, _ = _bytes_as_ints(rows[0])
    jobs = [_bytes_as_ints(row) for row in rows[1:n + 1]]
    best = 0
    for count in range(1, n + 1):
        for selection in combinations(jobs, count):
            ordered = sorted(selection)
            if all(left[1] < right[0] for left, right in zip(ordered, ordered[1:])):
                best = max(best, sum(job[2] for job in selection))
    return str(best)


def _interval_lis(rows):
    intervals = [_bytes_as_ints(row) for row in rows[1:]]
    ending = [set() for _ in intervals]
    for i, (left, right) in enumerate(intervals):
        ending[i] = {(1, value) for value in range(left, right + 1)}
        for earlier in ending[:i]:
            for length, tail in earlier:
                ending[i].update((length + 1, value) for value in range(max(left, tail + 1), right + 1))
    return str(max(length for possibilities in ending for length, _ in possibilities))


def _stamp_rally(rows):
    n, m = _bytes_as_ints(rows[0])
    links = [_bytes_as_ints(row) for row in rows[1:m + 1]]
    result = []
    for row in rows[m + 2:]:
        x, y, target = _bytes_as_ints(row)
        adj = [[] for _ in range(n + 1)]
        for count, (u, v) in enumerate(links, 1):
            adj[u].append(v)
            adj[v].append(u)
            seen = {x, y}
            pending = deque(seen)
            while pending:
                vertex = pending.popleft()
                for other in adj[vertex]:
                    if other not in seen:
                        seen.add(other)
                        pending.append(other)
            if len(seen) >= target:
                result.append(count)
                break
    return _answers(result)


def _lis_count(rows):
    values = _bytes_as_ints(rows[1])
    lengths = [1] * len(values)
    ways = [1] * len(values)
    for i, value in enumerate(values):
        for j in range(i):
            if values[j] >= value:
                continue
            new_length = lengths[j] + 1
            if new_length > lengths[i]:
                lengths[i] = new_length
                ways[i] = ways[j]
            elif new_length == lengths[i]:
                ways[i] += ways[j]
    longest = max(lengths)
    return str(sum(count for length, count in zip(lengths, ways) if length == longest) % 1_000_000_007)


def _mex_histogram(rows):
    n = int(rows[0])
    values = _bytes_as_ints(rows[1])
    counts = [0] * (n + 1)
    for left in range(n):
        for right in range(left + 1, n + 1):
            present = set(values[left:right])
            missing = next(i for i in range(n + 1) if i not in present)
            counts[missing] += 1
    return _answers(counts)


def _rollback_lis(rows):
    n = int(rows[0])
    values = _bytes_as_ints(rows[1])
    adj = [[] for _ in range(n)]
    for row in rows[2:]:
        first, second = (v - 1 for v in _bytes_as_ints(row))
        adj[first].append(second)
        adj[second].append(first)
    result = [0] * n
    pending = [(0, -1, [])]
    while pending:
        vertex, parent, path = pending.pop()
        path = path + [values[vertex]]
        longest = []
        for i, value in enumerate(path):
            longest.append(1 + max((longest[j] for j in range(i) if path[j] < value), default=0))
        result[vertex] = max(longest)
        for other in adj[vertex]:
            if other != parent:
                pending.append((other, vertex, path))
    return _answers(result)


def _subset_sum(rows):
    n, limit = _bytes_as_ints(rows[0])
    values = _bytes_as_ints(rows[1])
    best = 0
    for count in range(1, n + 1):
        for selection in combinations(values, count):
            total = sum(selection)
            if total <= limit:
                best = max(best, total)
    return str(best)


def _concatenation(rows):
    n = int(rows[0])
    pattern = rows[1]
    texts = []
    for row in rows[2:n + 2]:
        fields = row.split()
        if fields[0] != "~":
            texts.append(fields[0])
        else:
            first, second = (int(field) - 1 for field in fields[1:])
            texts.append(texts[first] + texts[second])
    text = texts[-1]
    count = sum(text.startswith(pattern, start) for start in range(len(text) - len(pattern) + 1))
    return str(count % 998244353)


def _cyclic_rotation(rows):
    text = rows[0]
    rotations = [text[i:] + text[:i] for i in range(len(text))]
    return _answers([min(rotations), max(rotations)])


def _dynamic_palindrome(rows):
    text = list(rows[1])
    result = []
    for row in rows[2:]:
        mode, left, arg = row.split()
        first = int(left) - 1
        if mode == "1":
            text[first] = arg
        else:
            word = text[first:int(arg)]
            result.append("Yes" if word == word[::-1] else "No")
    return _answers(result)


def _hamming_distance(rows):
    text, pattern = rows
    distances = [sum(a != b for a, b in zip(text[i:i + len(pattern)], pattern))
                 for i in range(len(text) - len(pattern) + 1)]
    return str(min(distances))


def _offline_prefix_lcp_sum(rows):
    n = int(rows[0])
    words = rows[1:n + 1]
    answers = []
    for row in rows[n + 2:]:
        operation, index, *argument = row.split()
        index = int(index) - 1
        if operation == "1":
            words[index] += argument[0]
        else:
            total = 0
            for word in words:
                for a, b in zip(word, words[index]):
                    if a != b:
                        break
                    total += 1
            answers.append(total)
    return _answers(answers)


def _run_length_z(rows):
    text = "".join(byte * count for byte, count in zip(rows[2], _bytes_as_ints(rows[1])))
    total = 0
    for start in range(len(text)):
        for a, b in zip(text, text[start:]):
            if a != b:
                break
            total += 1
    return str(total % 1_000_000_007)


def _shortest_superstring(rows):
    words = rows[1:]
    longest = sum(len(word) for word in words)
    for order in permutations(words):
        current = ""
        for word in order:
            if word in current:
                continue
            common = max(size for size in range(min(len(current), len(word)) + 1) if current.endswith(word[:size]))
            current += word[common:]
        longest = min(longest, len(current))
    return str(longest)


def _best_representation(rows):
    text = rows[0]
    size = len(text)
    parts = [0] + [size + 1] * size
    ways = [1] + [0] * size
    for right in range(1, size + 1):
        for left in range(right):
            word = text[left:right]
            repeated = any(len(word) % q == 0 and word == word[:q] * (len(word) // q)
                           for q in range(1, len(word)))
            if repeated:
                continue
            count = parts[left] + 1
            if count < parts[right]:
                parts[right] = count
                ways[right] = ways[left]
            elif count == parts[right]:
                ways[right] += ways[left]
    return _answers([parts[-1], ways[-1] % 1_000_000_007])


def _substring_comparison(rows):
    n = int(rows[0])
    words = rows[1:n + 1]
    q, seed, delta = _bytes_as_ints(rows[n + 1])
    total = 0
    for _ in range(q):
        first, second = divmod(seed, n - 1)
        if first > second:
            first, second = second, first
        else:
            second += 1
        for a, b in zip(words[first], words[second]):
            if a != b:
                break
            total += 1
        seed = (seed + delta) % (n * (n - 1))
    return str(total)


def _suffix_repeat(rows):
    n = int(rows[0])
    text = rows[1]
    for length in range(n // 2, 0, -1):
        starts = range(n - length + 1)
        if any(j - i >= length and text[i:i + length] == text[j:j + length]
               for i, j in combinations(starts, 2)):
            return str(length)
    return "0"


def _state_cycle(rows):
    count, state, modulus = _bytes_as_ints(rows[0])
    answer = 0
    for _ in range(count):
        answer += state
        state = state * state % modulus
    return str(answer)


def _dynamic_substring(rows):
    text = list(rows[1])
    answers = []
    for query in rows[2:]:
        kind, value, *replacement = query.split()
        if kind == "1":
            text[int(value) - 1] = replacement[0]
        else:
            answers.append("Yes" if value in "".join(text) else "No")
    return _answers(answers)


def _adjacent_swap(rows):
    source = _bytes_as_ints(rows[1])
    target = _bytes_as_ints(rows[2])
    swaps = 0
    for i, value in enumerate(target):
        at = i
        while at < len(source) and source[at] + at != value + i:
            at += 1
        if at == len(source):
            return "-1"
        for j in range(at, i, -1):
            source[j - 1], source[j] = source[j] + 1, source[j - 1] - 1
            swaps += 1
    assert source == target
    return str(swaps)


def _modular_pair_sum(rows):
    _, rank, modulus = _bytes_as_ints(rows[0])
    left, right = (_bytes_as_ints(line) for line in rows[1:])
    residues = sorted((a + b) % modulus for a in left for b in right)
    return str(residues[rank - 1])


def _pair_product(rows):
    _, rank = _bytes_as_ints(rows[0])
    products = sorted(a * b for a, b in combinations(_bytes_as_ints(rows[1]), 2))
    return str(products[rank - 1])


def _bracket_sequence(rows):
    text = list(rows[1])
    answers = []
    for row in rows[2:]:
        kind, left, right = _bytes_as_ints(row)
        if kind == 1:
            text[left - 1] = "(" if right == 1 else ")"
        else:
            unmatched = []
            for glyph in text[left - 1:right]:
                if glyph == ")" and unmatched and unmatched[-1] == "(":
                    unmatched.pop()
                else:
                    unmatched.append(glyph)
            answers.append(right - left + 1 - len(unmatched))
    return _answers(answers)


def _distinct_subsequence_order(rows):
    distinct = {""}
    for byte in rows[0]:
        distinct |= {prefix + byte for prefix in distinct}
    distinct.remove("")
    rank = int(rows[1])
    return sorted(distinct)[rank - 1] if rank <= len(distinct) else "Eel"


def _weighted_monotone_matching(rows):
    heights, weights = (_bytes_as_ints(row) for row in rows[1:])
    best_end = []
    for i, (height, weight) in enumerate(zip(heights, weights)):
        best_end.append(weight + max((best_end[j] for j in range(i) if heights[j] < height), default=0))
    return str(max(best_end))


def _shortest_absent_subsequence(rows):
    text = rows[0]
    alphabet = "abcdefghijklmnopqrstuvwxyz"
    length = 1
    while True:
        for letters in product(alphabet, repeat=length):
            start = 0
            for letter in letters:
                at = text.find(letter, start)
                if at < 0:
                    return "".join(letters)
                start = at + 1
        length += 1


def _monge_baseball(rows):
    n, k = _bytes_as_ints(rows[0])
    weights = _bytes_as_ints(rows[1])
    assert len(rows) == 2 and len(weights) == n and 1 <= k <= n <= 50000
    assert all(0 <= p <= 100000 for p in weights) and 1 <= sum(weights) <= 100000
    support = [(site, weight) for site, weight in enumerate(weights) if weight]
    answer = min(sum(weight * min(abs(site - center) for center in centers) for site, weight in support)
                 for centers in combinations(range(n), k))
    return str(answer)


def _coordinate_compression(lines):
    height, width, n = map(int, lines[0].split())
    cards = [tuple(map(int, line.split())) for line in lines[1:]]
    assert 1 <= n <= 100000 and len(set(cards)) == len(cards) == n
    assert all(1 <= row <= height and 1 <= column <= width for row, column in cards)
    rows, columns = {r for r, _ in cards}, {c for _, c in cards}
    return " ".join(f"{1 + sum(x < row for x in rows)} {1 + sum(y < column for y in columns)}"
                    for row, column in cards)


def _point_set_mex(lines):
    n, q = map(int, lines[0].split())
    values = list(map(int, lines[1].split()))
    assert 1 <= n <= 200000 and 1 <= q <= 200000
    assert len(values) == n and len(lines) == q + 2
    assert all(0 <= value <= 10**9 for value in values)
    answers = []
    for row in lines[2:]:
        position, value = map(int, row.split())
        assert 1 <= position <= n and 0 <= value <= 10**9
        values[position - 1] = value
        present = set(values)
        answers.append(next(i for i in range(n + 1) if i not in present))
    return _answers(answers)


def _sliding_window_extrema(lines):
    n, width = map(int, lines[0].split())
    values = list(map(int, lines[1].split()))
    assert 1 <= width <= n <= 1000000 and len(values) == n
    assert len(lines) == 2 and all(1 <= value <= 10**9 for value in values)
    return _answers(min(values[i:i + width]) for i in range(n - width + 1))


def _pair_sum_handshake(lines):
    n, rank = map(int, lines[0].split())
    values = list(map(int, lines[1].split()))
    assert 1 <= n <= 100000 and 1 <= rank <= n * n
    assert len(lines) == 2 and len(values) == n and all(1 <= value <= 100000 for value in values)
    pairs = sorted((a + b for a in values for b in values), reverse=True)
    return str(sum(pairs[:rank]))


def _string_radix_order(lines):
    alphabet, count, *words = lines
    assert len(alphabet) == 26 and set(alphabet) == set("abcdefghijklmnopqrstuvwxyz")
    assert 2 <= int(count) == len(words) <= 50000 and len(set(words)) == len(words)
    rank = {byte: position for position, byte in enumerate(alphabet)}
    assert all(1 <= len(word) <= 10 and all(byte in rank for byte in word) for word in words)
    return " ".join(sorted(words, key=lambda word: [rank[byte] for byte in word]))


def _dynamic_prefix_filter(lines):
    q = int(lines[0])
    assert 1 <= q <= 200000 and len(lines) == q + 1
    words, forbidden, answers = [], set(), []
    occupied = 0
    for row in lines[1:]:
        kind, word = row.split()
        assert kind in ("1", "2") and 1 <= len(word) <= 500000
        assert all("a" <= byte <= "z" for byte in word)
        occupied += len(word)
        if kind == "1":
            forbidden.add(word)
        else:
            words.append(word)
        answers.append(sum(not any(value.startswith(prefix) for prefix in forbidden) for value in words))
    assert occupied <= 500000
    return _answers(answers)


def _prefix_bitmap_table(lines):
    n, width, q = map(int, lines[0].split())
    assert 1 <= n <= 1000 and 1 <= width <= 3000 and 1 <= q <= 4000
    assert len(lines) == 1 + n + q
    rows = lines[1:1 + n]
    assert all(len(row) == width and row.isascii() and row.isalpha() and row.islower() for row in rows)
    answers = []
    for line in lines[1 + n:]:
        tag, *args = line.split()
        if tag == "1":
            column, from_byte, to_byte = args
            column = int(column) - 1
            assert 0 <= column < width and len(from_byte) == len(to_byte) == 1 and from_byte != to_byte
            assert "a" <= from_byte <= "z" and "a" <= to_byte <= "z"
            rows = [row[:column] + to_byte + row[column+1:] if row[column] == from_byte else row for row in rows]
        else:
            assert tag == "2" and len(args) == 1 and 1 <= len(args[0]) <= width
            prefix = args[0]
            assert prefix.isascii() and prefix.isalpha() and prefix.islower()
            answers.append(sum(row.startswith(prefix) for row in rows))
    return _answers(answers)


def _interval_map_paint(lines):
    n, q = map(int, lines[0].split())
    assert 1 <= n <= 500000 and 1 <= q <= 200000 and len(lines) == q + 1
    colors = list(range(1, n + 1))
    answers = []
    for line in lines[1:]:
        kind, *args = map(int, line.split())
        assert all(1 <= value <= n for value in args)
        if kind == 2:
            assert len(args) == 1
            answers.append(colors.count(args[0]))
        else:
            assert kind == 1 and len(args) == 2
            x, new_color = args
            old_color = colors[x - 1]
            left, right = x - 1, x
            while left > 0 and colors[left - 1] == old_color:
                left -= 1
            while right < n and colors[right] == old_color:
                right += 1
            colors[left:right] = [new_color] * (right - left)
    assert answers
    return _answers(answers)


def _subarray_median_order(lines):
    n = int(lines[0])
    values = list(map(int, lines[1].split()))
    assert len(lines) == 2 and 1 <= n == len(values) <= 100000
    assert all(1 <= value <= 10**9 for value in values)
    medians = sorted(sorted(values[left:right])[(right-left)//2]
                     for left in range(n) for right in range(left + 1, n + 1))
    return str(medians[len(medians) // 2])


def _range_equal_runs(lines):
    n, q = map(int, lines[0].split())
    values = list(map(int, lines[1].split()))
    assert 1 <= n <= 200000 and 1 <= q <= 200000 and len(lines) == q + 2
    assert len(values) == n and all(0 <= value <= 10**9 for value in values)
    answers = []
    for line in lines[2:]:
        kind, left, right = map(int, line.split())
        assert 1 <= left <= n
        if kind == 1:
            assert 0 <= right <= 10**9
            values[left - 1] = right
        else:
            assert kind == 2 and left <= right <= n
            longest = current = 1
            for position in range(left, right):
                current = current + 1 if values[position - 1] == values[position] else 1
                longest = max(longest, current)
            answers.append(longest)
    assert answers
    return _answers(answers)


def _lexicographic_subsequence(lines):
    n, k = map(int, lines[0].split())
    text = lines[1]
    assert len(lines) == 2 and 1 <= k <= n <= 100000 and len(text) == n
    assert all("a" <= letter <= "z" for letter in text)
    return min("".join(word) for word in combinations(text, k))


def _range_nondecreasing_subsequence(lines):
    n, q = map(int, lines[0].split())
    values = list(lines[1])
    assert 1 <= n <= 200000 and 1 <= q <= 200000 and len(lines) == q + 2
    assert len(values) == n and all(letter in "ABCD" for letter in values)
    answers = []
    for line in lines[2:]:
        kind, position, value = line.split()
        left = int(position) - 1
        assert 0 <= left < n
        if kind == "1":
            assert len(value) == 1 and value in "ABCD"
            values[left] = value
        else:
            right = int(value)
            assert kind == "2" and left < right <= n
            window = values[left:right]
            lengths = []
            for i, letter in enumerate(window):
                lengths.append(1 + max((lengths[j] for j in range(i) if window[j] <= letter), default=0))
            answers.append(len(window) - max(lengths))
    assert answers
    return _answers(answers)


def _cyclic_rotation_pairs(lines):
    n = int(lines[0])
    first, second = lines[1:]
    assert 1 <= n <= 200000 and len(first) == len(second) == n
    assert all("a" <= letter <= "z" for letter in first + second)
    first_rotations = [first[i:] + first[:i] for i in range(n)]
    second_rotations = [second[j:] + second[:j] for j in range(n)]
    return str(sum(a <= b for a in first_rotations for b in second_rotations))


def _cyclic_rotation_inversions(lines):
    n = int(lines[0])
    perm = list(map(int, lines[1].split()))
    assert len(lines) == 2 and 2 <= n <= 300000 and sorted(perm) == list(range(n))
    answers = []
    for shift in range(n):
        rotated = perm[shift:] + perm[:shift]
        answers.append(sum(a > b for a, b in combinations(rotated, 2)))
    return _answers(answers)


def _cyclic_rotation_inversions_adjacent_swaps(lines):
    n, q = map(int, lines[0].split())
    perm = list(map(int, lines[1].split()))
    assert 2 <= n <= 200000 and 1 <= q <= 200000 and len(lines) == q + 2
    assert sorted(perm) == list(range(1, n + 1))
    answers = []
    mod = 998244353
    for line in lines[2:]:
        position, steps = map(int, line.split())
        assert 1 <= position < n and 1 <= steps <= 10**9
        perm[position - 1], perm[position] = perm[position], perm[position - 1]
        ways = {tuple(perm): 1}
        for _ in range(steps):
            new_ways = {}
            for prior, count in ways.items():
                for cut in range(1, n):
                    next_perm = prior[:cut][::-1] + prior[cut:][::-1]
                    new_ways[next_perm] = (new_ways.get(next_perm, 0) + count) % mod
            ways = new_ways
        answers.append(sum(count * sum(a > b for a, b in combinations(outcome, 2))
                           for outcome, count in ways.items()) % mod)
    return _answers(answers)


def _range_add_threshold_min(lines):
    n, k, q = map(int, lines[0].split())
    rents = list(map(int, lines[1].split()))
    parking = list(map(int, lines[2].split()))
    assert 1 <= k <= n <= 100000 and 1 <= q <= 50000 and len(lines) == q + 3
    assert len(rents) == len(parking) == n and all(1 <= rent <= 10**9 for rent in rents)
    assert all(value in (0, 1) for value in parking)
    answers = []
    for line in lines[3:]:
        kind, x, y = map(int, line.split())
        if kind == 1:
            assert 1 <= x <= n and 1 <= y <= 10**9
            rents[x - 1] = y
        else:
            assert kind == 2 and 0 <= x <= k and y == 0
            eligible = [sum(rents[i:i + k]) for i in range(n - k + 1) if sum(parking[i:i + k]) >= x]
            answers.append(min(eligible) if eligible else "IMPOSSIBLE")
    assert answers
    return _answers(answers)


def _bounded_zigzag_subsequence(lines):
    n, low, high = map(int, lines[0].split())
    items = [tuple(map(int, line.split())) for line in lines[1:]]
    assert len(items) == n and 1 <= n <= 200000 and 1 <= low <= high <= 10**9
    assert all(1 <= height <= 10**9 and abs(weight) <= 10**9 for height, weight in items)
    ends = []
    for i, (height, weight) in enumerate(items):
        rise = max((ends[j][1] for j in range(i) if low <= height - items[j][0] <= high), default=0)
        fall = max((ends[j][0] for j in range(i) if low <= items[j][0] - height <= high), default=0)
        ends.append((weight + max(rise, 0), weight + max(fall, 0)))
    return str(max(0, max(max(pair) for pair in ends)))


def _pattern_erasure(lines):
    assert len(lines) == 1
    text = lines[0]
    assert 1 <= len(text) <= 200000 and all(letter in "ABC" for letter in text)
    while "ABC" in text:
        text = text.replace("ABC", "", 1)
    return text


def _pattern_avoidance(lines):
    n, m = map(int, lines[0].split())
    banned = lines[1:]
    assert 1 <= n <= 10**18 and 1 <= m <= 126 and len(banned) == len(set(banned)) == m
    assert all(1 <= len(word) <= 6 and set(word) <= {"a", "b"} for word in banned)
    answer = 0
    for letters in product("ab", repeat=n):
        word = "".join(letters)
        answer += all(pattern not in word for pattern in banned)
    return str(answer % 998244353)


def _sorted_substring_index(lines):
    n, q = int(lines[0]), int(lines[2])
    text = list(lines[1])
    assert 1 <= n <= 100000 and 1 <= q <= 100000 and len(lines) == q + 3
    assert len(text) == n and all("a" <= c <= "z" for c in text)
    answers = []
    for line in lines[3:]:
        kind, left, third = line.split()
        left = int(left)
        assert 1 <= left <= n
        if kind == "1":
            assert len(third) == 1 and "a" <= third <= "z"
            text[left - 1] = third
        else:
            right = int(third)
            assert kind == "2" and left <= right <= n
            target = "".join(text[left - 1:right])
            answers.append("Yes" if target in "".join(sorted(text)) else "No")
    assert answers
    return _answers(answers)


def _forbidden_sum_partitions(lines):
    n, forbidden = map(int, lines[0].split())
    values = list(map(int, lines[1].split()))
    assert len(lines) == 2 and len(values) == n and 1 <= n <= 200000 and abs(forbidden) <= 10**15
    assert all(abs(value) <= 10**9 for value in values)
    ways = [1] + [0] * n
    for right in range(1, n + 1):
        subtotal = 0
        for left in range(right - 1, -1, -1):
            subtotal += values[left]
            if subtotal != forbidden:
                ways[right] += ways[left]
        ways[right] %= 998244353
    return str(ways[n])


def _lattice_shape_transforms(lines):
    n = int(lines[0])
    assert 1 <= n <= 200 and len(lines) == 2 * n + 1
    shapes = []
    for offset in (1, n + 1):
        rows = lines[offset:offset + n]
        assert all(len(row) == n and set(row) <= {"#", "."} for row in rows)
        points = {(x, y) for y, row in enumerate(rows) for x, char in enumerate(row) if char == "#"}
        assert points
        shapes.append(points)

    def normalized(points):
        min_x = min(x for x, _ in points)
        min_y = min(y for _, y in points)
        return {(x - min_x, y - min_y) for x, y in points}

    current, goal = shapes
    target = normalized(goal)
    for _ in range(4):
        if normalized(current) == target:
            return "Yes"
        current = {(-y, x) for x, y in current}
    return "No"


def _cyclic_xor_matches(lines):
    n = int(lines[0])
    source, target = (list(map(int, line.split())) for line in lines[1:])
    assert 1 <= n <= 200000 and len(source) == len(target) == n
    assert all(0 <= value < 2**30 for value in source + target)
    answers = []
    for shift in range(n):
        mask = source[shift] ^ target[0]
        if all(source[(i + shift) % n] ^ mask == value for i, value in enumerate(target)):
            answers.extend([str(shift), str(mask)])
    return " ".join(answers)


def _ordered_multiset_extrema(lines):
    from collections import Counter
    q = int(lines[0])
    assert 1 <= q <= 200000 and len(lines) == q + 1
    values = Counter()
    answer = []
    for line in lines[1:]:
        operation, *arguments = map(int, line.split())
        if operation == 1:
            (key,) = arguments
            assert 0 <= key <= 10**9
            values[key] += 1
        elif operation == 2:
            key, copies = arguments
            assert 0 <= key <= 10**9 and 1 <= copies <= q
            if key in values:
                values[key] -= min(copies, values[key])
                if not values[key]:
                    del values[key]
        else:
            assert operation == 3 and not arguments and values
            answer.append(str(max(values) - min(values)))
    return " ".join(answer)


def _chord_crossing_queries(lines):
    n, m = map(int, lines[0].split())
    chords = [tuple(map(int, line.split())) for line in lines[1:m + 1]]
    q = int(lines[m + 1])
    queries = [tuple(map(int, line.split())) for line in lines[m + 2:]]
    assert 2 <= n <= 10**6 and 1 <= m <= min(n // 2, 200000) and 1 <= q <= 200000 and len(queries) == q
    assert all(1 <= a < b <= 2 * n and a % 2 == b % 2 == 0 for a, b in chords)
    assert len({endpoint for chord in chords for endpoint in chord}) == m * 2
    assert all(1 <= a < b <= 2 * n and a % 2 == b % 2 == 1 for a, b in queries)
    return " ".join(str(sum(a < left < b < right or left < a < right < b for a, b in chords))
                    for left, right in queries)


def _dominance_counts(lines):
    n = int(lines[0])
    a, b = (list(map(int, line.split())) for line in lines[1:])
    assert 1 <= n <= 200000 and len(a) == len(b) == n
    assert all(0 <= value <= 10**9 for value in a + b)
    return str(sum(a[i] >= a[j] and b[i] <= b[j] for i in range(n) for j in range(n)))


def _sparse_obstacle_grid_paths(lines):
    h, w, k = map(int, lines[0].split())
    walls = {tuple(map(int, line.split())) for line in lines[1:]}
    assert 2 <= h <= 100000 and 2 <= w <= 100000 and 1 <= k <= 3000
    assert len(walls) == k == len(lines) - 1
    assert (1, 1) not in walls and (h, w) not in walls
    assert all(1 <= row <= h and 1 <= col <= w for row, col in walls)
    dp = [[0] * (w + 1) for _ in range(h + 1)]
    dp[0][1] = 1
    for i in range(1, h + 1):
        for j in range(1, w + 1):
            if (i, j) not in walls:
                dp[i][j] = (dp[i-1][j] + dp[i][j-1]) % 1000000007
    return str(dp[h][w])


def _target_sum_subrectangles(lines):
    stream = iter(lines)
    cases = int(next(stream))
    assert 1 <= cases <= 25000
    result = []
    cells = 0
    for _ in range(cases):
        h, w = map(int, next(stream).split())
        assert 1 <= h and 1 <= w
        grid = [next(stream) for _ in range(h)]
        assert all(len(row) == w and set(row) <= {"#", "."} for row in grid)
        cells += h * w
        count = 0
        for top in range(h):
            for bottom in range(top + 1, h + 1):
                for left in range(w):
                    for right in range(left + 1, w + 1):
                        marks = sum(row[left:right].count("#") for row in grid[top:bottom])
                        count += 2 * marks == (bottom - top) * (right - left)
        result.append(str(count))
    assert not list(stream) and cells <= 300000
    return " ".join(result)


def _knuth_interval_dp(lines):
    n = int(lines[0])
    weights = list(map(int, lines[1].split()))
    assert len(lines) == 2 and len(weights) == n and 2 <= n <= 3000
    assert all(1 <= value <= 10**9 for value in weights)
    linear = weights * 2
    table = [[0] * (2 * n + 1) for _ in range(2 * n)]
    for size in range(2, n + 1):
        for left in range(2 * n - size + 1):
            right = left + size
            table[left][right] = sum(linear[left:right]) + min(
                table[left][mid] + table[mid][right] for mid in range(left + 1, right))
    return str(min(table[left][left + n] for left in range(n)))


def _binary_grid_squares(lines):
    h, w, holes = map(int, lines[0].split())
    points = {tuple(map(int, line.split())) for line in lines[1:]}
    assert 1 <= h <= 3000 and 1 <= w <= 3000 and 0 <= holes <= min(h * w, 10**5)
    assert len(points) == holes == len(lines) - 1
    assert all(1 <= row <= h and 1 <= col <= w for row, col in points)
    total = 0
    for i in range(1, h + 1):
        for j in range(1, w + 1):
            for side in range(1, min(h - i + 1, w - j + 1) + 1):
                if any(i <= a < i + side and j <= b < j + side for a, b in points):
                    break
                total += 1
    return str(total)


def _run_length_queue(lines):
    from collections import deque
    q = int(lines[0])
    assert 1 <= q <= 200000 and len(lines) == q + 1
    queue = deque()
    answers = []
    for line in lines[1:]:
        operation, count, *value = map(int, line.split())
        assert 1 <= count <= 10**9
        if operation == 1:
            assert len(value) == 1 and 1 <= value[0] <= 10**9
            queue.extend([value[0]] * count)
        else:
            assert operation == 2 and not value and count <= len(queue)
            answers.append(str(sum(queue.popleft() for _ in range(count))))
    return " ".join(answers)


def _repeated_subsequence(lines):
    n = int(lines[0])
    cycle, target = lines[1:]
    assert 1 <= n <= 10**12 and 1 <= len(cycle) <= 100000 and 1 <= len(target) <= 100000
    assert all("a" <= char <= "z" for char in cycle + target)
    expanded = cycle * n
    answer = 0
    for count in range(1, len(expanded) // len(target) + 1):
        position = 0
        for char in "".join(char * count for char in target):
            matched = expanded.find(char, position)
            if matched == -1:
                break
            position = matched + 1
        else:
            answer = count
            continue
        break
    return str(answer)


def _balanced_pivot_subsequence(lines):
    n, q = map(int, lines[0].split())
    text = lines[1]
    ranges = [tuple(map(int, line.split())) for line in lines[2:]]
    assert 1 <= n <= 100000 and 1 <= q <= 100000 and len(text) == n and len(ranges) == q
    assert set(text) <= {"1", "2", "/"}
    answers = []
    for left, right in ranges:
        assert 1 <= left <= right <= n
        section = text[left - 1:right]
        arms = max((min(section[:i].count("1"), section[i+1:].count("2"))
                    for i, byte in enumerate(section) if byte == "/"), default=-1)
        answers.append(str(2 * arms + 1 if arms >= 0 else 0))
    return " ".join(answers)


def _three_symbol_frequency_counts(lines):
    n = int(lines[0])
    text = lines[1]
    assert len(lines) == 2 and len(text) == n and 1 <= n <= 200000 and set(text) <= {"A", "B", "C"}
    result = 0
    for left in range(n):
        for right in range(left + 1, n + 1):
            part = text[left:right]
            result += len({part.count("A"), part.count("B"), part.count("C")}) == 3
    return str(result)


def _pattern_range_occurrences(lines):
    q = int(lines[0])
    text, pattern = lines[1:3]
    assert 1 <= q <= 200000 and len(lines) == q + 3 and 1 <= len(text) <= 400000 and 1 <= len(pattern) <= 10
    assert all("a" <= byte <= "z" for byte in text + pattern)
    answers = []
    for line in lines[3:]:
        left, right = map(int, line.split())
        assert 1 <= left <= right <= len(text)
        answers.append("Yes" if pattern in text[left - 1:right] else "No")
    return " ".join(answers)


def _exact_subarray_extrema(lines):
    n, maximum, minimum = map(int, lines[0].split())
    values = list(map(int, lines[1].split()))
    assert len(lines) == 2 and len(values) == n and 1 <= n <= 200000
    assert 1 <= minimum <= maximum <= 200000 and all(1 <= value <= 200000 for value in values)
    result = 0
    for left in range(n):
        for right in range(left + 1, n + 1):
            interval = values[left:right]
            result += min(interval) == minimum and max(interval) == maximum
    return str(result)


def _longest_bitonic_subsequence(lines):
    n = int(lines[0])
    values = list(map(int, lines[1].split()))
    assert len(lines) == 2 and len(values) == n and 1 <= n <= 300000
    assert all(1 <= value <= n for value in values)
    left = [1] * n
    right = [1] * n
    for i in range(n):
        for j in range(i):
            if values[j] < values[i]:
                left[i] = max(left[i], left[j] + 1)
    for i in range(n - 1, -1, -1):
        for j in range(i + 1, n):
            if values[j] < values[i]:
                right[i] = max(right[i], right[j] + 1)
    return str(max(a + b - 1 for a, b in zip(left, right)))


def _poisson_binomial(lines):
    from decimal import Decimal, localcontext
    from math import comb

    n = int(lines[0])
    probabilities = [Decimal(value) for value in lines[1].split()]
    assert len(lines) == 2 and len(probabilities) == n and 1 <= n <= 2999 and n % 2
    assert len(set(probabilities)) == 1 and Decimal("0.01") <= probabilities[0] <= Decimal("0.99")
    p = probabilities[0]
    with localcontext() as context:
        context.prec = 80
        answer = sum(comb(n, successes) * p**successes * (1 - p)**(n - successes)
                     for successes in range(n // 2 + 1, n + 1))
    return f"{answer:.12f}"


def _range_add_total_variation(lines):
    n, q = map(int, lines[0].split())
    values = list(map(int, lines[1].split()))
    changes = [tuple(map(int, line.split())) for line in lines[2:]]
    assert len(values) == n and len(changes) == q and 1 <= n <= 100000 and 1 <= q <= 100000
    assert all(abs(value) <= 10**9 for value in values)
    values_after = []
    for left, right, delta in changes:
        assert 1 <= left <= right <= n and abs(delta) <= 10**9
        for i in range(left - 1, right):
            values[i] += delta
        values_after.append(str(sum(abs(a - b) for a, b in zip(values, values[1:]))))
    return " ".join(values_after)


def _inversion_limited_partitions(lines):
    n, limit = map(int, lines[0].split())
    values = list(map(int, lines[1].split()))
    assert len(lines) == 2 and len(values) == n and 1 <= n <= 200000 and 0 <= limit <= n * (n - 1) // 2
    assert all(1 <= value <= 10**9 for value in values)
    ways = [1] + [0] * n
    for right in range(1, n + 1):
        for left in range(right):
            inversions = sum(values[i] > values[j] for i in range(left, right) for j in range(i + 1, right))
            if inversions <= limit:
                ways[right] += ways[left]
        ways[right] %= 10**9 + 7
    return str(ways[n])


def _dictionary_segmenter(lines):
    [text] = lines
    assert 1 <= len(text) <= 100000 and all("a" <= char <= "z" for char in text)
    words = ("dream", "dreamer", "erase", "eraser")
    reachable = {0}
    for start in range(len(text)):
        if start in reachable:
            reachable.update(start + len(word) for word in words if text.startswith(word, start))
    return "YES" if len(text) in reachable else "NO"


def _deadline_job_selection(lines):
    n = int(lines[0])
    jobs = [tuple(map(int, line.split())) for line in lines[1:]]
    assert 1 <= n <= 12 and len(jobs) == n
    assert all(1 <= deadline <= 5000 and 1 <= duration <= 5000 and 1 <= reward <= 10**9
               for deadline, duration, reward in jobs)
    jobs.sort()
    best = 0
    for mask in range(1 << n):
        finish = 0
        total = 0
        for index, (deadline, duration, reward) in enumerate(jobs):
            if mask & (1 << index):
                finish += duration
                total += reward
                if finish > deadline:
                    break
        else:
            best = max(best, total)
    return str(best)


def _periodic_pattern_runs(lines):
    period, pattern = lines
    n, m = len(period), len(pattern)
    assert 1 <= n <= 500000 and 1 <= m <= 500000
    assert all("a" <= byte <= "z" for byte in period + pattern)
    best = 0
    for start in range(n):
        count = 0
        while count < n and all(period[(start + count * m + j) % n] == byte for j, byte in enumerate(pattern)):
            count += 1
        if count == n:
            return "-1"
        best = max(best, count)
    return str(best)


def _pattern_stamping(lines):
    n, m = map(int, lines[0].split())
    _, target, stamp = lines
    assert 1 <= m <= min(n, 5) and n <= 12 and len(target) == n and len(stamp) == m
    assert all("A" <= char <= "Z" for char in target + stamp)
    queue = ["#" * n]
    seen = set(queue)
    for current in queue:
        if target == current:
            return "Yes"
        for left in range(n - m + 1):
            next_state = current[:left] + stamp + current[left + m:]
            if next_state not in seen:
                seen.add(next_state)
                queue.append(next_state)
    return "No"


def _subarray_frequency_windows_distinct(lines):
    n, limit = map(int, lines[0].split())
    values = list(map(int, lines[1].split()))
    assert len(lines) == 2 and len(values) == n and 1 <= limit <= n <= 100000
    assert all(1 <= value <= 10**9 for value in values)
    return str(max(right - left for left in range(n) for right in range(left + 1, n + 1)
                   if len(set(values[left:right])) <= limit))


def _subarray_sum_counts_equal(lines):
    n, target = map(int, lines[0].split())
    values = list(map(int, lines[1].split()))
    assert len(lines) == 2 and len(values) == n and 1 <= n <= 200000
    assert abs(target) <= 10**15 and all(abs(value) <= 10**9 for value in values)
    return str(sum(sum(values[left:right]) == target for left in range(n) for right in range(left + 1, n + 1)))


def _clamped_translation(lines):
    n = int(lines[0])
    steps = [tuple(map(int, line.split())) for line in lines[1:n + 1]]
    q = int(lines[n + 1])
    queries = list(map(int, lines[n + 2].split()))
    assert 1 <= n <= 200000 and 1 <= q <= 200000 and len(queries) == q and len(lines) == n + 3
    assert all(abs(amount) <= 10**9 and 1 <= kind <= 3 for amount, kind in steps)
    assert all(abs(value) <= 10**9 for value in queries)
    results = []
    for value in queries:
        for amount, kind in steps:
            if kind == 1:
                value += amount
            elif kind == 2:
                value = max(value, amount)
            else:
                value = min(value, amount)
        results.append(str(value))
    return " ".join(results)


def _weighted_interval_stabbing(lines):
    n, m = map(int, lines[0].split())
    intervals = [tuple(map(int, line.split())) for line in lines[1:]]
    assert 1 <= n <= 12 and 1 <= m <= 200000 and len(intervals) == m
    assert all(1 <= left <= right <= n and abs(w) <= 10**9 for left, right, w in intervals)
    maximum = 0
    for mask in range(1 << n):
        selected = {point for point in range(1, n + 1) if mask & (1 << (point - 1))}
        score = sum(w for left, right, w in intervals if any(left <= point <= right for point in selected))
        maximum = max(maximum, score)
    return str(maximum)


def _maximum_allowed_pair(lines):
    n, m, p = map(int, lines[0].split())
    left = list(map(int, lines[1].split()))
    right = list(map(int, lines[2].split()))
    forbidden = {tuple(map(int, line.split())) for line in lines[3:]}
    assert 1 <= n <= 100000 and 1 <= m <= 100000 and 0 <= p <= min(100000, n * m - 1)
    assert len(left) == n and len(right) == m and all(1 <= value <= 10**9 for value in left + right)
    assert len(lines) == p + 3 and len(forbidden) == p
    assert all(1 <= i <= n and 1 <= j <= m for i, j in forbidden)
    return str(max(a + b for i, a in enumerate(left, 1) for j, b in enumerate(right, 1) if (i, j) not in forbidden))


def _monotone_value_shift(lines):
    n = int(lines[0])
    updates = [tuple(map(int, line.split())) for line in lines[1:n + 1]]
    q = int(lines[n + 1])
    original = list(map(int, lines[n + 2:]))
    assert 1 <= n <= 200000 and 1 <= q <= 300000 and len(original) == q
    assert all(1 <= low <= high <= 500000 for low, high in updates)
    assert all(1 <= rating <= 500000 for rating in original)
    result = []
    for rating in original:
        for low, high in updates:
            if low <= rating <= high:
                rating += 1
        result.append(str(rating))
    return " ".join(result)


def _nearest_value_index(lines):
    n, q = map(int, lines[0].split())
    values = list(map(int, lines[1].split()))
    assert 1 <= n <= 100000 and 1 <= q <= 100000 and len(lines) == q + 2
    assert len(values) == n and all(abs(value) <= 10**8 for value in values)
    answers = []
    for line in lines[2:]:
        center, rank = map(int, line.split())
        assert abs(center) <= 10**8 and 1 <= rank <= n
        answers.append(sorted(abs(center - value) for value in values)[rank - 1])
    return _answers(answers)


def _grid_patterns(lines):
    height, width = map(int, lines[0].split())
    rows, columns = map(int, lines[height + 1].split())
    text = lines[1:height + 1]
    pattern = lines[height + 2:]
    assert all(1 <= dimension <= 1000 for dimension in (height, width, rows, columns))
    assert len(text) == height and len(pattern) == rows
    assert all(len(row) == width and row.isascii() and row.isalnum() for row in text)
    assert all(len(row) == columns and row.isascii() and row.isalnum() for row in pattern)
    return _answers(value for y in range(height - rows + 1) for x in range(width - columns + 1)
                    if all(text[y + i][x:x + columns] == row for i, row in enumerate(pattern))
                    for value in (y, x))


def _edit_alignment(lines):
    n, m = map(int, lines[0].split())
    a, b = [list(map(int, row.split())) for row in lines[1:]]
    assert 1 <= n <= 1000 and 1 <= m <= 1000 and len(a) == n and len(b) == m
    assert all(1 <= value <= 10**9 for value in a + b)
    costs = [[0] * (m + 1) for _ in range(n + 1)]
    costs[0] = list(range(m + 1))
    for i, first in enumerate(a, 1):
        costs[i][0] = i
        for j, second in enumerate(b, 1):
            costs[i][j] = min(costs[i-1][j] + 1, costs[i][j-1] + 1, costs[i-1][j-1] + (first != second))
    return str(costs[-1][-1])


_CHECKS = {
    "coordinate_compression": _coordinate_compression,
    "point_set_mex": _point_set_mex,
    "sliding_window_extrema": _sliding_window_extrema,
    "pair_sum_order_handshake": _pair_sum_handshake,
    "string_radix_order_alphabet": _string_radix_order,
    "dynamic_prefix_filter": _dynamic_prefix_filter,
    "prefix_bitmap_table": _prefix_bitmap_table,
    "interval_map_paint": _interval_map_paint,
    "subarray_median_order": _subarray_median_order,
    "range_equal_runs": _range_equal_runs,
    "lexicographic_subsequence": _lexicographic_subsequence,
    "range_nondecreasing_subsequence": _range_nondecreasing_subsequence,
    "cyclic_rotation_pairs": _cyclic_rotation_pairs,
    "cyclic_rotation_inversions": _cyclic_rotation_inversions,
    "cyclic_rotation_inversions_adjacent_swaps": _cyclic_rotation_inversions_adjacent_swaps,
    "range_add_threshold_min": _range_add_threshold_min,
    "bounded_zigzag_subsequence": _bounded_zigzag_subsequence,
    "pattern_erasure": _pattern_erasure,
    "pattern_avoidance": _pattern_avoidance,
    "sorted_substring_index": _sorted_substring_index,
    "forbidden_sum_partitions": _forbidden_sum_partitions,
    "lattice_shape_transforms": _lattice_shape_transforms,
    "cyclic_xor_matches": _cyclic_xor_matches,
    "ordered_multiset_extrema": _ordered_multiset_extrema,
    "chord_crossing_queries": _chord_crossing_queries,
    "dominance_counts": _dominance_counts,
    "sparse_obstacle_grid_paths": _sparse_obstacle_grid_paths,
    "target_sum_subrectangles": _target_sum_subrectangles,
    "knuth_interval_dp": _knuth_interval_dp,
    "binary_grid_squares": _binary_grid_squares,
    "run_length_queue": _run_length_queue,
    "repeated_subsequence": _repeated_subsequence,
    "balanced_pivot_subsequence": _balanced_pivot_subsequence,
    "three_symbol_frequency_counts": _three_symbol_frequency_counts,
    "pattern_range_occurrences": _pattern_range_occurrences,
    "exact_subarray_extrema": _exact_subarray_extrema,
    "longest_bitonic_subsequence": _longest_bitonic_subsequence,
    "poisson_binomial": _poisson_binomial,
    "range_add_total_variation": _range_add_total_variation,
    "inversion_limited_partitions": _inversion_limited_partitions,
    "dictionary_segmenter": _dictionary_segmenter,
    "deadline_job_selection": _deadline_job_selection,
    "periodic_pattern_runs": _periodic_pattern_runs,
    "pattern_stamping": _pattern_stamping,
    "subarray_frequency_windows_distinct": _subarray_frequency_windows_distinct,
    "subarray_sum_counts_equal": _subarray_sum_counts_equal,
    "clamped_translation": _clamped_translation,
    "weighted_interval_stabbing": _weighted_interval_stabbing,
    "maximum_allowed_pair": _maximum_allowed_pair,
    "monotone_value_shift": _monotone_value_shift,
    "nearest_value_index": _nearest_value_index,
    "grid_pattern_matching_all": _grid_patterns,
    "edit_alignment_sequence_matching": _edit_alignment,
    "adjacent_swap_distance": _adjacent_swap,
    "interval_lis": _interval_lis,
    "interval_schedule_max_weight": _interval_schedule,
    "lis_count": _lis_count,
    "modular_pair_sum_order": _modular_pair_sum,
    "monge_shortest_paths_baseball": _monge_baseball,
    "pair_product_order": _pair_product,
    "parallel_binary_search_stamp_rally": _stamp_rally,
    "subarray_mex_histogram": _mex_histogram,
    "rollback_lis_tree": _rollback_lis,
    "state_cycle_sequence_sum": _state_cycle,
    "subset_sum_meet_in_middle_max": _subset_sum,
    "weighted_monotone_matching_flowers": _weighted_monotone_matching,
    "bracket_sequence_range": _bracket_sequence,
    "concatenated_pattern_count": _concatenation,
    "cyclic_rotation": _cyclic_rotation,
    "distinct_subsequence_order": _distinct_subsequence_order,
    "dynamic_palindrome": _dynamic_palindrome,
    "dynamic_substring_matching": _dynamic_substring,
    "hamming_distance_matching_binary_min": _hamming_distance,
    "offline_prefix_lcp_sum": _offline_prefix_lcp_sum,
    "run_length_z_sum": _run_length_z,
    "shortest_absent_subsequence": _shortest_absent_subsequence,
    "shortest_common_superstring": _shortest_superstring,
    "string_periods_best_representation": _best_representation,
    "substring_comparison_common_prefix": _substring_comparison,
    "suffix_tree_nonoverlapping_repeat": _suffix_repeat,
}


def independent(plan, data):
    return _CHECKS[plan["verify_id"]](data.splitlines())
