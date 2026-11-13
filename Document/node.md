# 単方向ノードと連結列

`Node`は値を持たず、`std::shared_ptr<Node> n`で次のノードを所有します。[API一覧](../DataStructure/Node.cpp)と[使用例](../Examples/node_example.cpp)を参照してください。

コピーは後続ノードを共有します。共有されていない長い後続列は反復処理で解放し、連結に沿った深い再帰を避けます。循環参照の自動回収は行いません。

| 操作 | 戻り値・前提 |
| --- | --- |
| `make_node(next={})` | `next`を後続に持つ新しいノード。O(1)時間・1回確保。 |
| `make_chain(count, tail={})` | `tail`の前に`count`個追加した先頭。O(count)時間・count回確保。 |
| `chain_size(head)` / `chain_tail(head)` | ノード数／末尾。空の末尾はnullptr。循環なしでO(k)時間・O(1)追加領域。 |
| `chain_nth(head, index)` | 0始まりのindex番目、不足ならnullptr。循環も可。O(index+1)時間上界。 |
| `push_front_node(head)` / `pop_front_node(head)` | 先頭の追加／削除。削除は空ならfalse。O(1)時間。 |
| `reverse_chain(head)` | 循環のない列を反転して新しい先頭を返す。O(k)時間。 |
| `clone_chain(head)` | 循環のない列の独立した複製。O(k)時間・k回確保。 |
| `cycle_entry(head)` / `has_cycle(head)` | 循環入口の非所有ポインタ／循環の有無。O(k)時間・O(1)追加領域。 |
| `break_cycle(head)` | 循環を切断したらtrue。O(k)時間・O(1)追加領域。 |
| `clear_chain(head)` | 先頭を空にし、非共有の後続を反復解放。循環回収は先にbreak_cycle。 |

`chain_size`、`chain_tail`、`clone_chain`は既定で循環を検出し、`invalid_argument`を送出します。`SKIP_RUNTIME_CHECKS`時は循環のない入力が前提です。

`reverse_chain`と`break_cycle`による接続変更は、同じノードを共有する側にも見えます。`pop_front_node`は共有された元先頭の`n`を切断しません。

`push_front_node`は新ノードの確保に成功してから接続を移すため、確保に失敗しても元の先頭と接続を保持します。

`cycle_entry`はFloyd法で入口を求め、`has_cycle`は合流の検出で終了します。`break_cycle`は速い側のポインタの直前も保持し、合流後に入口と周期末尾を同期して探します。先頭から循環する場合は合流時点で末尾も分かるため、周期をもう一周しません。切断直前に先頭の所有権を保持するため、切断する`n`自身を引数にしても切断中のノードは生存します。

`SKIP_RUNTIME_CHECKS`時の`chain_size`は2ノードずつ数えます。個数の表現上限に近づいた場合は1ノードずつ進め、検査を省略してもオーバーフローを防ぎます。

GCCでは`chain_nth`が空列を先に返し、2ノードずつ進めます。残り1個は通常の走査で処理し、不足した位置の所有権も保持して返します。循環上でも指定した回数だけ進みます。

オンラインVerifyは[ABC228 B - Takahashi's Secret](https://atcoder.jp/contests/abc228/tasks/abc228_b)です。各人をノード、秘密を伝える相手を次ノードとして接続し、始点からの循環を`break_cycle`で切断して`chain_size`で到達人数を数えます。始点から到達しない循環も、全ノードの接続を解除して回収します。共有された末尾、複製、逆転、確保失敗、長い列の反復破棄は独立したローカル検査で補います。
