// SPDX-License-Identifier: CC0-1.0
// 参考（辞書語への分割）: https://atcoder.jp/contests/abc049/tasks/arc065_a
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/dictionary_segmenter.md
//
// API一覧
// DictionarySegmenter<Alphabet=26,First='a'>(words): 辞書を構築する。空語は無視し、添字は元の語順のまま。
// ByteDictionarySegmenter: 全バイト用。
// minimum_segmentation(text) const: 最少の語数で分割し、0始まりの辞書語IDを先頭から返す。不可能ならnullopt。
// 空textは空の分割。辞書は内部で保持する。コピーは同じ読み取り専用辞書を共有、move元は空辞書相当。
// 辞書の語数k・総文字数L・状態数Sとして構築O(1+k+L+S*Alphabet)時間、O(1+k+S*Alphabet)領域。
// 長さn・一致数RのtextにO(1+n+R)時間、O(n+1)領域。
