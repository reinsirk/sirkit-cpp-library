# 文字列

各ライブラリは必要な標準ヘッダーを含み、単独で貼り付けられます。

| ファイル | 対応する検証 |
| --- | --- |
| [RollingHash.cpp](RollingHash.cpp) | [Z Algorithm](../Verify/current/submissions/rolling_hash.cpp) |
| [Trie.cpp](Trie.cpp) | [C - gacha](../Verify/current/submissions/trie_dictionary.cpp) |
| [Aho_Corasick.cpp](Aho_Corasick.cpp) | [Aho Corasick](../Verify/current/submissions/aho_corasick.cpp)・[G - Count Substring Query](../Verify/current/submissions/aho_corasick_counts.cpp) |
| [Suffix_Automaton.cpp](Suffix_Automaton.cpp) | [Number of Substrings](../Verify/current/submissions/suffix_automaton.cpp)・[Longest Common Substring](../Verify/current/submissions/suffix_automaton_lcs.cpp) |
| [Palindromic_Tree.cpp](Palindromic_Tree.cpp) | [Eertree](../Verify/current/submissions/palindromic_tree.cpp) |
| [Lyndon_Factorization.cpp](Lyndon_Factorization.cpp) | [Lyndon Factorization](../Verify/current/submissions/lyndon_factorization.cpp) |
| [Prefix_Function.cpp](Prefix_Function.cpp) | [String Matching](../Verify/current/submissions/kmp_string_matching.cpp) |
| [Longest_Common_Subsequence.cpp](Longest_Common_Subsequence.cpp) | [F - LCS](../Verify/current/submissions/lcs_sequence.cpp) |
| [Edit_Distance.cpp](Edit_Distance.cpp) | [F - Operate K](../Verify/current/submissions/bounded_edit_distance.cpp)・[Edit Distance](../Verify/current/submissions/edit_distance.cpp) |
| [Run_Enumerate.cpp](Run_Enumerate.cpp) | [Run Enumerate](../Verify/current/submissions/run_enumerate.cpp) |
| [Wildcard_Pattern_Matching.cpp](Wildcard_Pattern_Matching.cpp) | [Wildcard Pattern Matching](../Verify/current/submissions/wildcard_pattern_matching.cpp) |
| [Palindromes_In_Deque.cpp](Palindromes_In_Deque.cpp) | [Palindromes in Deque](../Verify/current/submissions/palindromes_in_deque.cpp) |
| [Prefix_Substring_LCS.cpp](Prefix_Substring_LCS.cpp) | [Prefix-Substring LCS](../Verify/current/submissions/prefix_substring_lcs.cpp) |
| [Number_Of_Subsequences.cpp](Number_Of_Subsequences.cpp) | [Number of Subsequences](../Verify/current/submissions/number_of_subsequences.cpp) |
| [Palindromic_Factorization.cpp](Palindromic_Factorization.cpp) | [回文の最小分割](../Verify/current/submissions/palindromic_factorization_local.cpp) |

問題URL、追加APIの補足検査、現在版の判定状況は[Verify一覧](../Verify/current/README.md)を参照してください。

区間ハッシュの取得、共通接頭辞の照会、再構築、衝突の扱いは[説明](../Document/rolling_hash.md)と[使用例](../Examples/rolling_hash_example.cpp)を参照してください。

[Trieの説明](../Document/trie.md)に扱う文字列、所有権、各操作の前提をまとめています。

## 静的な文字列処理

| ファイル | 対応する検証 |
| --- | --- |
| [Z_Algorithm.cpp](Z_Algorithm.cpp) | [Z Algorithm](../Verify/current/submissions/z_algorithm.cpp) |
| [Manacher.cpp](Manacher.cpp) | [Enumerate Palindromes](../Verify/current/submissions/manacher.cpp) |
| [Suffix_Array.cpp](Suffix_Array.cpp) | [Suffix Array](../Verify/current/submissions/suffix_array.cpp)・[Number of Substrings](../Verify/current/submissions/suffix_array_substrings.cpp) |
| [Concatenated_Pattern_Count.cpp](Concatenated_Pattern_Count.cpp) | [SEARCH in the Text Editor](../Verify/current/submissions/concatenated_pattern_count.cpp) |
| [Cyclic_Rotation.cpp](Cyclic_Rotation.cpp) | [String Shifting](../Verify/current/submissions/cyclic_rotation.cpp) |
| [Dynamic_Palindrome.cpp](Dynamic_Palindrome.cpp) | [Palindrome Query](../Verify/current/submissions/dynamic_palindrome.cpp) |
| [Multi_String_Longest_Common_Substring.cpp](Multi_String_Longest_Common_Substring.cpp) | [Longest Common Substring](../Verify/current/submissions/multi_string_longest_common_substring.cpp) |
| [String_Periods.cpp](String_Periods.cpp) | [Best Representation](../Verify/current/submissions/string_periods_best_representation.cpp) |
| [Substring_Index.cpp](Substring_Index.cpp) | [Number of Substrings](../Verify/current/submissions/substring_index_distinct_count.cpp)<br>[K-th Substring](../Verify/current/submissions/substring_index_kth.cpp)<br>[Count Substring Query](../Verify/current/submissions/substring_index_occurrences.cpp) |
| [Suffix_Tree.cpp](Suffix_Tree.cpp) | [Who Says a Pun?](../Verify/current/submissions/suffix_tree_nonoverlapping_repeat.cpp) |
| [Hamming_Distance_Matching.cpp](Hamming_Distance_Matching.cpp) | [Substring 2](../Verify/current/submissions/hamming_distance_matching_binary_min.cpp)<br>[Substring](../Verify/current/submissions/hamming_distance_matching_lowercase_min.cpp) |
| [Substring_Comparison.cpp](Substring_Comparison.cpp) | [典型LCP](../Verify/current/submissions/substring_comparison_common_prefix.cpp)<br>[String Hash](../Verify/current/submissions/substring_comparison_equal.cpp) |
| [Offline_Prefix_Lcp_Sum.cpp](Offline_Prefix_Lcp_Sum.cpp) | [Sum of Common Prefix Length](../Verify/current/submissions/offline_prefix_lcp_sum.cpp) |
| [Run_Length_Z.cpp](Run_Length_Z.cpp) | [Long Prefixes](../Verify/current/submissions/run_length_z_sum.cpp) |
| [Shortest_Common_Superstring.cpp](Shortest_Common_Superstring.cpp) | [Compress Strings](../Verify/current/submissions/shortest_common_superstring.cpp) |
| [Dynamic_Substring_Matching.cpp](Dynamic_Substring_Matching.cpp) | [Itsuki](../Verify/current/submissions/dynamic_substring_matching.cpp) |
| [Bracket_Sequence.cpp](Bracket_Sequence.cpp) | [Range Bracket Sequence](../Verify/current/submissions/bracket_sequence_range.cpp)<br>[Parenthesis Checking](../Verify/current/submissions/bracket_sequence_balanced.cpp) |
| [Distinct_Subsequence_Order.cpp](Distinct_Subsequence_Order.cpp) | [辞書順](../Verify/current/submissions/distinct_subsequence_order.cpp) |
| [Shortest_Absent_Subsequence.cpp](Shortest_Absent_Subsequence.cpp) | [Don't Be a Subsequence](../Verify/current/submissions/shortest_absent_subsequence.cpp) |
| [Edit_Alignment.cpp](Edit_Alignment.cpp) | [ABC185 E - Sequence Matching](../Verify/current/submissions/edit_alignment_sequence_matching.cpp) |
| [Grid_Pattern_Matching.cpp](Grid_Pattern_Matching.cpp) | [Seek Grid](../Verify/current/submissions/grid_pattern_matching_seek_grid.cpp)<br>[Pattern Search](../Verify/current/submissions/grid_pattern_matching_all.cpp) |
| [String_Radix_Order.cpp](String_Radix_Order.cpp) | [本棚](../Verify/current/submissions/string_radix_order_bookshelf.cpp)<br>[Neo-lexicographic Ordering](../Verify/current/submissions/string_radix_order_alphabet.cpp) |
| [Dynamic_Prefix_Filter.cpp](Dynamic_Prefix_Filter.cpp) | [Forbidden Prefix](../Verify/current/submissions/dynamic_prefix_filter.cpp) |
| [Prefix_Bitmap_Table.cpp](Prefix_Bitmap_Table.cpp) | [Substitute and Search](../Verify/current/submissions/prefix_bitmap_table.cpp) |
| [Cyclic_Rotation_Order.cpp](Cyclic_Rotation_Order.cpp) | [Two Strings](../Verify/current/submissions/cyclic_rotation_pairs.cpp) |
| [Pattern_Erasure.cpp](Pattern_Erasure.cpp) | [Take ABC](../Verify/current/submissions/pattern_erasure.cpp) |
| [Pattern_Avoidance.cpp](Pattern_Avoidance.cpp) | [Banned Substrings](../Verify/current/submissions/pattern_avoidance.cpp) |
| [Sorted_Substring_Index.cpp](Sorted_Substring_Index.cpp) | [Substring of Sorted String](../Verify/current/submissions/sorted_substring_index.cpp) |
| [Balanced_Pivot_Subsequence.cpp](Balanced_Pivot_Subsequence.cpp) | [11/22 Subsequence](../Verify/current/submissions/balanced_pivot_subsequence.cpp) |
| [Repeated_Subsequence.cpp](Repeated_Subsequence.cpp) | [SSttrriinngg in StringString](../Verify/current/submissions/repeated_subsequence.cpp) |
| [Bitmask_NFA.cpp](Bitmask_NFA.cpp) | [G - Wildcard Construction](../Verify/current/submissions/bitmask_nfa_wildcard.cpp) |
| [Cyclic_Xor_Matches.cpp](Cyclic_Xor_Matches.cpp) | [Xor Shift](../Verify/current/submissions/cyclic_xor_matches.cpp) |
| [Lexicographic_Concatenation.cpp](Lexicographic_Concatenation.cpp) | [F - Concat (2nd)](../Verify/current/submissions/lexicographic_concatenation_second.cpp) |
| [Lexicographic_String_Shuffle.cpp](Lexicographic_String_Shuffle.cpp) | [No.205 マージして辞書順最小](../Verify/current/submissions/lexicographic_string_shuffle.cpp) |
| [Regex_Language_Edit_Distance.cpp](Regex_Language_Edit_Distance.cpp) | [No.874 正規表現間距離](../Verify/current/submissions/regex_language_edit_distance.cpp) |
| [DFA_Minimization.cpp](DFA_Minimization.cpp) | [G - Wildcard Construction](../Verify/current/submissions/dfa_minimization_wildcard.cpp)<br>[決定性有限オートマトンの状態の最小化](../Verify/current/submissions/dfa_minimization_reference.cpp) |
| [Pattern_Stamping.cpp](Pattern_Stamping.cpp) | [Stamp](../Verify/current/submissions/pattern_stamping.cpp) |
| [Periodic_Pattern_Runs.cpp](Periodic_Pattern_Runs.cpp) | [Strings of Eternity](../Verify/current/submissions/periodic_pattern_runs.cpp) |
| [Dictionary_Segmenter.cpp](Dictionary_Segmenter.cpp) | [Daydream](../Verify/current/submissions/dictionary_segmenter.cpp) |
| [Pattern_Range_Occurrences.cpp](Pattern_Range_Occurrences.cpp) | [Range Search Query](../Verify/current/submissions/pattern_range_occurrences.cpp) |
| [Three_Symbol_Frequency_Counts.cpp](Three_Symbol_Frequency_Counts.cpp) | [Unbalanced ABC Substrings](../Verify/current/submissions/three_symbol_frequency_counts.cpp) |
