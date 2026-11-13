# 新規オンラインVerify driverの読解レビュー

対象は今回新設した47本のnormal sourceと79本のonline driver。main全体、公開APIの前提、対応する公式入出力を読み取り点検し、型・符号・方向・root・境界の変換について**確認できた実不具合は0件**。コンパイル、追加性能測定、オンライン提出はしていない。

- 点加算和は最大1e15、Beatsは公式の処理中絶対値上限により和が2e17以内。
- フローの負費用、非可換の合成順、LCTの半開区間→閉パス変換、永続木の更新後rootを確認。
- 0長/空区間/0行・0列を許す公式問題では対応するAPIと入力抽出も整合。
- 79本とも外部ACL/Mint不要。mainへの入力分布別切替は見つからない。

centroid_decomposition / fps_sqrtのsampleは一つの既知出力とのtokens回帰照合。公式は別解を許すため、任意の別解を受け付けるcheckerとは区別する。オンライン問題の採用自体は適切であり、既存の独立試験は重心分割の性質と平方根の数式を別途検査している。今回の凍結では変更不要。

matrix_product / binary_matrix_detの公式本文は今回の取得が失敗し、現行数値上限の外部再確認は未完。一部の他問題のMAXパラメータも未取得としてreportに明記した。79本すべての公式最大ケースや時間制限内実行を証明したという結果ではなく、新規online ACを意味しない。

全件の短い理由と対象の実SHAは[照合結果](verify-new-driver-domains-2026-10-31.json)に記録し、79行のTSVと元のscope・報告は[補足ZIP](data/verify-new-driver-domains-2026-10-31.zip)に保存した。レビュー終了時にdriver・submission・47 sourceの変更は検出されなかった。
