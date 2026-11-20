# 作業ガイド

作業を始める前に、対象に対応する共有ルールを読んでください。

- 公開API、命名、所有権、行う入力検査、処理系の違いは[C++の実装規約](Document/contributing/cpp.md)
- API説明、使用例、README、生成ページの書式は[API文書と使用例](Document/contributing/api-docs.md)
- Verify先の選定、ローカル検証、オンライン提出と提出資料の整理は[正しさの検証と提出](Document/contributing/verification.md)
- 高速化の進め方と手元での性能比較は[性能測定と最適化](Document/contributing/performance.md)。既存の提出を調べるときは[キラーケースの探し方](Document/contributing/performance.md#既存の提出からキラーケースを探す)
- SIMDの活用、データ配置・走査順の見直し、AVX-512実機がない場合の検証は[SIMDとキャッシュの最適化](Document/contributing/performance.md#simdとキャッシュの最適化)
- フォルダの配置、コミット、開発ブランチ、CI、配布は[開発・CI・配布](Document/contributing/workflow.md)
- 外部コードの出典とSPDXは[出典とライセンス](Document/contributing/licensing.md)
- ClaudeとCodexの利用は[ClaudeとCodexの利用](Document/contributing/tools.md)

作業環境固有の設定は、Git管理外の`.sirkit-local/AGENTS.md`がある場合に参照してください。
