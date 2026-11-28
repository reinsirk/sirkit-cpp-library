// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/run_length_z.md
//
// API一覧
// RunLengthZ::Run{byte,count}: byteがcount回連続する区間。countはuint64_t。
// RunLengthZ(runs): 連長圧縮された入力からO(n+1)時間・領域で構築。0回のrunは飛ばし、隣接同値は結合。
// size(): 展開後の長さ。O(1)。合計長はUINT64_MAX以下、結合後の区間数はINT_MAX以下。
// prefix_at(position): 展開後の0始まりの位置から始まる接尾辞との共通接頭辞長。position==size()なら0。O(log(n+2))。
// sum_prefix_lengths(modulus=1000000007): 全開始位置のZ値の総和を正のuint32_tの法で返す。O(n+1)、照会追加O(1)。
// nは入力区間数。同梱のZ AlgorithmはDocument/z_algorithm.mdを参照。
