#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""明示的に解放を省く高速版を通常ソースから作る。"""
from pathlib import Path
import argparse
import hashlib
import json
from fast_hash_source import build as build_fast_hash

ROOT = Path(__file__).resolve().parents[1]


try:
    from public_namespace import wrap_source, unwrap_source
except ModuleNotFoundError:
    from Scripts.public_namespace import wrap_source, unwrap_source

def replace_once(text: str, old: str, new: str) -> str:
    if text.count(old) != 1:
        raise ValueError("通常版が変わっています。高速版の差分を見直してください: " + old[:100])
    return text.replace(old, new)


def binary_trie(source: str) -> str:
    source = replace_once(source, "//\n// API一覧", "// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/fast_variants.md\n//\n// API一覧")
    source = replace_once(source, "// [BinaryTrie<T,MAX_LOG,NODES>]", "// [速度優先版 BinaryTrie<T,MAX_LOG,NODES>]\n// ノードとID用ブロックをnewで確保し、破棄時も解放しない。プロセス終了まで累積する。\n// 正常な入力・十分なNODES・確保成功・size_typeに収まる件数が前提。例外時の状態復元は保証しない。")
    source = replace_once(source, "// NODES は根込みの仮想二分ノード数の上限。内部は最大2bitずつ分岐する。以下 B = MAX_LOG + 1。", "// NODESは実ノード/ID枠の確保上限。必要数が上限内であることは利用側が保証。以下B=MAX_LOG+1。")
    source = replace_once(source, "// SKIP_RUNTIME_CHECKS を定義すると入力検査を省く。容量保護は常に有効。", "// 配布のFast版は入力検査・容量検査・仮想二分ノード数の管理を除く。")
    source = replace_once(source, "// add/insert_uniqueは償却O(B)、ID追加は償却O(1)。確保失敗時は値・件数・IDを保持する。", "// add/insert_uniqueは償却O(B)、ID追加は償却O(1)。確保失敗時の保証はない。")
    source = replace_once(source, "// 最初の最大2048ノードもブロックとして所有する。", "// 最初の最大2048ノードもブロックとして確保し、解放しない。")
    source = replace_once(source, "// ID配列本体もブロックで所有し、拡張時の参照を維持する。", "// ID配列も解放しないブロックへ置き、拡張時の参照を維持する。")
    source = replace_once(source, "// ノードをブロック単位で所有し、拡張時もアドレスを維持する。", "// 確保したブロックの参照を保持し、拡張時もアドレスを維持する。")
    source = replace_once(source, "#include <memory>\n", "")
    source = replace_once(source, "std::vector<std::unique_ptr<Node[]>> blocks_;", "std::vector<Node*> blocks_;")
    source = replace_once(source, "std::unique_ptr<InitialBlock> initial_;", "InitialBlock* initial_ = nullptr;")
    source = replace_once(source, "std::array<IdSlot, kInitialIdSlots> initial_ids_;", "IdSlot* initial_ids_ = nullptr;")
    source = replace_once(source, "std::vector<std::unique_ptr<IdSlot[]>> id_blocks_;", "std::vector<IdSlot*> id_blocks_;")
    source = replace_once(source, "std::make_unique<IdSlot[]>(n)", "new IdSlot[n]")
    source = replace_once(source, "std::make_unique<Node[]>(n)", "new Node[n]")
    source = replace_once(source, "std::make_unique_for_overwrite<InitialBlock>()", "new InitialBlock")
    source = replace_once(source, "    size_type virtual_nodes_ = 1;\n", "")
    source = replace_once(source, '            if (id_capacity_ == NODES)\n                throw std::length_error("BinaryTrie: ID slot limit exceeded");\n', "")
    source = replace_once(source, '            if (capacity_ == NODES)\n            {\n                throw std::length_error("BinaryTrie: node limit exceeded");\n            }\n', "")
    source = replace_once(source, "        const bool recycled = free_id_slot_ != kNoSlot;", "        if (!initial_ids_ && kInitialIdSlots != 0) initial_ids_ = new IdSlot[kInitialIdSlots];\n        const bool recycled = free_id_slot_ != kNoSlot;")
    source = replace_once(source,
        "        if (!initial_ids_ && kInitialIdSlots != 0) initial_ids_ = new IdSlot[kInitialIdSlots];",
        "#ifndef SIRKIT_FAST_TRIE_INLINE_IDS\n        if (!initial_ids_ && kInitialIdSlots != 0) initial_ids_ = new IdSlot[kInitialIdSlots];\n#endif")
    source = replace_once(source,
        "    static constexpr size_type kIdBlockSize = 4096;",
        "#ifdef SIRKIT_FAST_TRIE_INLINE_IDS\n    union InitialIdStorage\n    {\n        std::array<IdSlot, kInitialIdSlots> slots;\n        InitialIdStorage() : slots{} {}\n        ~InitialIdStorage() {} // 初期ID配列のvector容量も解放しない。\n    };\n#endif\n    static constexpr size_type kIdBlockSize = 4096;")
    source = replace_once(source, "    size_type free_id_slot_ = kNoSlot;",
        "    size_type free_id_slot_ = kNoSlot;\n#ifdef SIRKIT_FAST_TRIE_INLINE_IDS\n    InitialIdStorage initial_storage_;\n#endif")
    source = replace_once(source, "    BinaryTrie()\n    {",
        "    BinaryTrie()\n    {\n#ifdef SIRKIT_FAST_TRIE_INLINE_IDS\n        initial_ids_ = initial_storage_.slots.data();\n#endif")
    source = replace_once(source,
        "// ノードとID用ブロックをnewで確保し、破棄時も解放しない。プロセス終了まで累積する。",
        "// ノードとID用ブロックをnewで確保し、破棄時も解放しない。プロセス終了まで累積する。\n// SIRKIT_FAST_TRIE_INLINE_IDS: 最初の32個までのID枠を内蔵し、初回ID追加の確保を省く。\n// 既定はID未使用時のオブジェクトを小さく保つ。内蔵時もIDのvector容量は解放しない。")
    source = replace_once(source, "            Container{}.swap(entry.ids);", "            entry.ids.clear(); // 再利用枠の容量を保持する。")
    start = source.index("    bool insert(T x, int id, bool unique)\n")
    end = source.index("\npublic:\n    BinaryTrie()", start)
    source = source[:start] + """    bool insert(T x, int id, bool unique)
    {
        const U key = checked_key(x) ^ lazy_;
        std::array<Node*, kLevels + 1> path;
        path[0] = root_;
        size_type first_new = 0;
        for (size_type depth = 1; depth <= kLevels; ++depth)
        {
            const auto d = digit(key, depth);
            Node* child = path[depth - 1]->data.child[d];
            if (!child)
            {
                child = acquire_node();
                if (!first_new) first_new = depth;
                else path[depth - 1]->data.child[d] = child;
            }
            path[depth] = child;
        }
        if (unique && !first_new) return false;
        Node* leaf = path[kLevels];
        if (first_new) leaf->data.ids.ptr = nullptr;
        if (id != -1)
        {
            if (leaf->data.ids.ptr) leaf->data.ids.ptr->push_back(id);
            else append_id(leaf, id);
        }
        if (first_new) path[first_new - 1]->data.child[digit(key, first_new)] = path[first_new];
        for (Node* node : path) ++node->count;
        return true;
    }
""" + source[end:]
    source = replace_once(source, "            virtual_nodes_ -= width(depth) - static_cast<unsigned>(width(depth) == 2 && parent->data.child[d ^ 1u]);\n", "")
    # 高速版でも、削除したノードは内部のfree listで再利用する。
    return source


def fast_introduction(source: str) -> str:
    source = replace_once(source, "//\n// API一覧", "// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/fast_variants.md\n//\n// API一覧")
    source = replace_once(source, "// API一覧\n", "// API一覧\n// 速度優先版: 内部の主要領域は破棄・解放しない。格納型のデストラクタの副作用を必要としないこと。\n// 入力・容量・件数の前提と確保成功を利用側が保証する。例外時の状態復元は保証しない。\n")
    return source


def fast_buffer_storage(source: str, replacements: list[tuple[str, str]]) -> str:
    source = fast_introduction(source)
    for old, new in replacements:
        source = replace_once(source, old, new)
    helper = unwrap_source((ROOT / "Fast/Source/Internal/Buffer.cpp").read_text())
    helper = helper[helper.index("#include "):]
    index = source.index("#include ")
    return source[:index] + helper + "\n" + source[index:]


def fenwick(source: str) -> str:
    return fast_buffer_storage(source, [
        ("std::vector<Storage> tree_;", "reinsirk::detail::FastBuffer<Storage> tree_;"),
        ("std::vector<Storage> replacement(storage_size(checked_size(n)), Storage{});", "reinsirk::detail::FastBuffer<Storage> replacement(storage_size(checked_size(n)), Storage{});")])


def segment(source: str) -> str:
    return fast_buffer_storage(source, [("std::vector<S> data_;", "reinsirk::detail::FastBuffer<S> data_;")])


def lazy_segment(source: str) -> str:
    return fast_buffer_storage(source, [
        ("std::vector<S> data_;", "reinsirk::detail::FastBuffer<S> data_;"),
        ("std::vector<F> lazy_;", "reinsirk::detail::FastBuffer<F> lazy_;"),
        ("std::vector<std::uint8_t> pending_;", "reinsirk::detail::FastBuffer<std::uint8_t> pending_;")])


def helper_body(path):
    text=unwrap_source(path.read_text());return text[text.index('#include '):]

def introduce(source,note):
    source=replace_once(source,'//\n// API一覧','// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/fast_variants.md\n//\n// API一覧')
    return replace_once(source,'// API一覧\n','// API一覧\n'+note+'\n// 入力・容量・件数の前提と確保成功を利用側が保証する。例外時の状態復元は保証しない。\n')

def treap_unreleased(source,unreleased_helper):
    source=introduce(source,'// 速度優先版: 最後のノード配列は破棄・解放しない。プロセス終了まで累積する。\n// ノード配列はvectorの通常の成長を使い、再確保時の旧領域を回収する。格納型の破棄の副作用を必要としないこと。')
    source=replace_once(source,'std::vector<StoredNode> nodes_;','reinsirk::detail::UnreleasedVector<StoredNode> nodes_;')
    at=source.index('#include ');return source[:at]+helper_body(unreleased_helper)+'\n'+source[at:]

def aho_compiler_storage(source,buffer_helper,unreleased_helper):
    source=introduce(source,'// 速度優先版: GCCでは最後の主要配列の破棄・解放を省き、成長時はvectorとして旧領域を回収する。\n// Clangでは従来のFastBufferを使い、成長中の旧領域も保持する。最後の領域はプロセス終了まで累積する。\n// 格納型の破棄の副作用を必要としないこと。')
    for old,new in [('std::vector<reinsirk::detail::AhoNode<Alphabet>> nodes_;','reinsirk::detail::AhoStorage<reinsirk::detail::AhoNode<Alphabet>> nodes_;'),('std::vector<reinsirk::detail::AhoPattern> patterns_;','reinsirk::detail::AhoStorage<reinsirk::detail::AhoPattern> patterns_;'),('std::vector<int> order_;','reinsirk::detail::AhoStorage<int> order_;'),('std::vector<int> order; order.reserve(nodes_.size()); order.push_back(0);','reinsirk::detail::AhoStorage<int> order; order.reserve(nodes_.size()); order.push_back(0);')]:source=replace_once(source,old,new)
    alias='''#ifndef SIRKIT_AHO_FAST_STORAGE_DEFINED
#define SIRKIT_AHO_FAST_STORAGE_DEFINED
namespace reinsirk::detail {
#if defined(__GNUC__) && !defined(__clang__)
template<class T>using AhoStorage=UnreleasedVector<T>;
#else
template<class T>using AhoStorage=FastBuffer<T>;
#endif
}
#endif
'''
    helpers='#if defined(__GNUC__) && !defined(__clang__)\n'+helper_body(unreleased_helper)+'\n#else\n'+helper_body(buffer_helper)+'\n#endif\n'
    at=source.index('#include ');return source[:at]+helpers+alias+source[at:]

def implicit_treap(source: str) -> str:
    return treap_unreleased(source,ROOT/"Fast/Source/Internal/Unreleased_Vector.cpp")

def aho_corasick(source: str) -> str:
    return aho_compiler_storage(source,ROOT/"Fast/Source/Internal/Buffer.cpp",ROOT/"Fast/Source/Internal/Unreleased_Vector.cpp")


def wavelet_matrix(source: str) -> str:
    return fast_buffer_storage(source, [
        ("std::vector<std::uint64_t> words_;", "reinsirk::detail::FastBuffer<std::uint64_t> words_;"),
        ("std::vector<std::uint32_t> prefix_;", "reinsirk::detail::FastBuffer<std::uint32_t> prefix_;"),
        ("std::vector<reinsirk::detail::WaveletRankBits> levels_;", "reinsirk::detail::FastBuffer<reinsirk::detail::WaveletRankBits> levels_;"),
        ("std::vector<std::size_t> zero_;", "reinsirk::detail::FastBuffer<std::size_t> zero_;")])


def persistent_segment(source: str) -> str:
    source = fast_introduction(source)
    source = replace_once(source,
        "// 速度優先版: 内部の主要領域は破棄・解放しない。格納型のデストラクタの副作用を必要としないこと。",
        "// 速度優先版: 状態本体をnewで確保し、破棄・reset・clear時も解放しない。\n// ノード列はvectorの通常の成長・再確保を使う。格納型の破棄の副作用を必要としないこと。")
    for old, new in [
        ("std::unique_ptr<State> state_;", "State* state_;"),
        ("std::make_unique<State>(n, std::move(op), std::move(identity))", "new State(n, std::move(op), std::move(identity))"),
        ("PersistentSegmentTree(PersistentSegmentTree&&) noexcept = default;", "PersistentSegmentTree(PersistentSegmentTree&& other) noexcept : state_(std::exchange(other.state_, nullptr)) {}"),
        ("PersistentSegmentTree& operator=(PersistentSegmentTree&&) noexcept = default;", "PersistentSegmentTree& operator=(PersistentSegmentTree&& other) noexcept {\n        if (this != &other) state_ = std::exchange(other.state_, nullptr);\n        return *this;\n    }"),
        ("void swap(PersistentSegmentTree& other) noexcept { state_.swap(other.state_); }", "void swap(PersistentSegmentTree& other) noexcept { std::swap(state_, other.state_); }"),
        ("auto next = std::make_unique<State>(n, state_->op, state_->identity);", "auto next = new State(n, state_->op, state_->identity);"),
        ("// reset(n)/clear(): 全根を無効化して単位元列/長さ0へ戻す。旧ノード数に比例する破棄費用。", "// reset(n)/clear(): 全根を無効化して単位元列/長さ0へ戻す。O(1)、旧領域を解放しない。")]:
        source = replace_once(source, old, new)
    for old, new in (("std::make_unique<State>(*other.state_)", "new State(*other.state_)"),
                     ("state_.swap(next)", "std::swap(state_, next)")):
        if source.count(old) != 2:
            raise ValueError("PersistentSegmentTreeのコピー/再初期化を再確認してください: " + old)
        source = source.replace(old, new)
    return source


def generate(check: bool = False) -> None:
    builders = {"DataStructure/Binary_Trie.cpp": binary_trie,
                "DataStructure/Fenwick_Tree.cpp": fenwick,
                "DataStructure/Segment_Tree.cpp": segment,
                "DataStructure/Lazy_Segment_Tree.cpp": lazy_segment,
                "DataStructure/implicit_treap.cpp": implicit_treap,
                "DataStructure/Persistent_Segment_Tree.cpp": persistent_segment,
                "DataStructure/Hash_map.cpp": lambda source: build_fast_hash(source, "HashMap", ROOT),
                "DataStructure/Hash_set.cpp": lambda source: build_fast_hash(source, "HashSet", ROOT),
                "DataStructure/Wavelet_Matrix.cpp": wavelet_matrix,
                "String/Aho_Corasick.cpp": aho_corasick}
    mapping = {}
    records = []
    for name, build in builders.items():
        original = (ROOT / name).read_text()
        content = wrap_source(build(unwrap_source(original))).encode()
        target = "Fast/Source/" + name
        output = ROOT / target
        if check:
            if not output.is_file() or output.read_bytes() != content:
                raise ValueError("専用高速版が同期していません: " + target)
        else:
            output.parent.mkdir(parents=True, exist_ok=True)
            output.write_bytes(content)
        mapping[name] = target
        records.append({"source": name, "source_sha256": hashlib.sha256(original.encode()).hexdigest(),
                        "path": target, "sha256": hashlib.sha256(content).hexdigest()})
    for name in ("DataStructure/Node.cpp",):
        target = "Fast/Source/" + name
        content = wrap_source((ROOT / target).read_text()).encode()
        if check and (ROOT / target).read_bytes() != content:
            raise ValueError("Fast手書き版のnamespaceが未同期です: " + target)
        if not check:
            (ROOT / target).write_bytes(content)
        mapping[name] = target
        records.append({"source": name, "source_sha256": hashlib.sha256((ROOT / name).read_bytes()).hexdigest(),
                        "path": target, "sha256": hashlib.sha256(content).hexdigest(), "handwritten": True})
    outputs = {ROOT / "Scripts/fast_overrides.json": mapping, ROOT / "Fast/source_manifest.json": records}
    for path, value in outputs.items():
        data = (json.dumps(value, ensure_ascii=False, indent=2) + "\n").encode()
        if check:
            if path.read_bytes() != data:
                raise ValueError("専用高速版manifestが同期していません")
        else:
            path.write_bytes(data)
    print(str(len(records)) + "件の専用高速実装: " + ("同期確認" if check else "生成"))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    generate(parser.parse_args().check)
