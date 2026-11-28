// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/weighted_l1_statistics.md
//
// API一覧
// WeightedL1Statistics(coordinates={}): 追加できるint64_t座標を登録する。重複可。U座標の登録はO(U log(U+1))。
// insert(x,w=1)/erase(x,w=1): 登録済みのxへuint64_tの重みを加える/引く。不足するeraseはfalseで状態を保つ。
// weight_at(x)/total_weight(): 登録座標の重み/全重み。total_weightはO(1)。全重みがuint64_tを超える追加は例外。
// kth(rank): 重みを個数とみなした0始まりの順位。範囲外はnullopt。
// minimizer_interval(): Σw|x-a|を最小にする閉区間。空集合の関数は全域0で、戻り値はnullopt。
// absolute_deviation(x): 任意のint64_t xに対するΣw|x-a|。uint64_tを超えた場合は例外。
// 異なる座標数Dに対し、上記の個別操作はO(log(D+1))時間。格納領域O(U+1)。コピーは独立、move元は空。
