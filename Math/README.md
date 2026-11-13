# 数論・組合せ・変換

各ライブラリは必要な標準ヘッダーを含み、単独で貼り付けられます。

| ファイル | 対応する検証 |
| --- | --- |
| [Combination/Binomial_Mod.cpp](Combination/Binomial_Mod.cpp) | [Binomial Coefficient](../Verify/current/submissions/binomial_integer_split.cpp) |
| [Combination/ModInt_Binomial.cpp](Combination/ModInt_Binomial.cpp) | [Binomial Coefficient](../Verify/current/submissions/binomial_modint_split.cpp) |
| [Combination/Prime_Binomial.cpp](Combination/Prime_Binomial.cpp) | [Binomial Coefficient (Prime Mod)](../Verify/current/submissions/binomial_prime_split.cpp) |
| [Combination_AtCoderMod.cpp](Combination_AtCoderMod.cpp) | [Binomial Coefficient (Prime Mod)](../Verify/current/submissions/binomial_prime.cpp) |
| [Combination_Mod.cpp](Combination_Mod.cpp) | [Binomial Coefficient](../Verify/current/submissions/binomial_integer.cpp) |
| [Eratosthenes.cpp](Eratosthenes.cpp) | [Enumerate Primes](../Verify/current/submissions/eratosthenes.cpp) |
| [Fraction.cpp](Fraction.cpp) | [分数の比較](../Verify/current/submissions/fraction_sort.cpp)・[四則演算](../Verify/current/submissions/fraction_arithmetic.cpp) |
| [Zeta/Bitwise_Convolution.cpp](Zeta/Bitwise_Convolution.cpp) | [Bitwise And Convolution](../Verify/current/submissions/zeta_and_split.cpp) |
| [Zeta/Divisor_Multiple_Transform.cpp](Zeta/Divisor_Multiple_Transform.cpp) | [GCD Convolution](../Verify/current/submissions/zeta_gcd_transform_split.cpp) |
| [Zeta/Gcd_Lcm_Convolution.cpp](Zeta/Gcd_Lcm_Convolution.cpp) | [GCD Convolution](../Verify/current/submissions/zeta_gcd_conv_split.cpp) |
| [Zeta/Set_Transform.cpp](Zeta/Set_Transform.cpp) | [Bitwise And Convolution](../Verify/current/submissions/zeta_set_as_and_split.cpp) |
| [Zeta/Sieve.cpp](Zeta/Sieve.cpp) | [Prime Factorization](../Verify/current/submissions/zeta_sieve_factorize_split.cpp) |
| [Zeta/Sparse_Transform.cpp](Zeta/Sparse_Transform.cpp) | [GCD Convolution](../Verify/current/submissions/zeta_sparse_gcd_split.cpp) |
| [Zeta_Mebius.cpp](Zeta_Mebius.cpp) | [Bitwise And Convolution](../Verify/current/submissions/zeta_and_full.cpp) |
| [prime_judge(Miller-Rabin).cpp](prime_judge(Miller-Rabin).cpp) | [Primality Test](../Verify/current/submissions/primality.cpp) |
| [Pollard_Rho.cpp](Pollard_Rho.cpp) | [Factorize](../Verify/current/submissions/pollard_rho_factorize.cpp) |
| [Formal_Power_Series.cpp](Formal_Power_Series.cpp) | [inverse of Formal Power Series](../Verify/current/submissions/fps_inverse.cpp)・[log of Formal Power Series](../Verify/current/submissions/fps_log.cpp)・[exp of Formal Power Series](../Verify/current/submissions/fps_exp.cpp)・[pow of Formal Power Series](../Verify/current/submissions/fps_pow.cpp)・[sqrt of Formal Power Series](../Verify/current/submissions/fps_sqrt.cpp)・[Division of Polynomials](../Verify/current/submissions/fps_divmod.cpp) |
| [Xor_Basis.cpp](Xor_Basis.cpp) | [Xor Sum 3](../Verify/current/submissions/xor_basis.cpp) |
| [Berlekamp_Massey.cpp](Berlekamp_Massey.cpp) | [Find Linear Recurrence](../Verify/current/submissions/berlekamp_massey.cpp) |
| [Linear_Recurrence.cpp](Linear_Recurrence.cpp) | [Kth term of Linearly Recurrent Sequence](../Verify/current/submissions/linear_recurrence.cpp) |
| [Matrix.cpp](Matrix.cpp) | [Matrix Product](../Verify/current/submissions/matrix_product.cpp)・[Pow of Matrix](../Verify/current/submissions/matrix_power.cpp) |
| [Gaussian_Elimination.cpp](Gaussian_Elimination.cpp) | [Determinant](../Verify/current/submissions/matrix_determinant.cpp)・[Inverse Matrix](../Verify/current/submissions/matrix_inverse.cpp)・[Rank of Matrix](../Verify/current/submissions/matrix_rank.cpp)・[System of Linear Equations](../Verify/current/submissions/matrix_linear_system.cpp) |
| [Binary_Matrix.cpp](Binary_Matrix.cpp) | [Matrix Product (Mod 2)](../Verify/current/submissions/binary_matrix_product.cpp)・[Rank of Matrix (Mod 2)](../Verify/current/submissions/binary_matrix_rank.cpp)・[Determinant of Matrix (Mod 2)](../Verify/current/submissions/binary_matrix_det.cpp)・[Inverse Matrix (Mod 2)](../Verify/current/submissions/binary_matrix_inverse.cpp)・[System of Linear Equations (Mod 2)](../Verify/current/submissions/binary_matrix_solve.cpp) |
| [Convolution.cpp](Convolution.cpp) | [Convolution (Mod 1,000,000,007)](../Verify/current/submissions/convolution_any_mod.cpp)・[G - Fine Triplets](../Verify/current/submissions/convolution_integer_triplets.cpp) |
| [Polynomial.cpp](Polynomial.cpp) | [Multipoint Evaluation](../Verify/current/submissions/multipoint_evaluation.cpp)・[Polynomial Interpolation](../Verify/current/submissions/polynomial_interpolation.cpp)・[Polynomial Taylor Shift](../Verify/current/submissions/polynomial_taylor_shift.cpp) |
| [Modular_Arithmetic.cpp](Modular_Arithmetic.cpp) | [Sqrt Mod](../Verify/current/submissions/modular_sqrt.cpp)・[050 - Power](../Verify/current/submissions/modular_power.cpp)・[E - Throne](../Verify/current/submissions/linear_congruence.cpp)・[E - Oversleeping](../Verify/current/submissions/chinese_remainder.cpp) |

用途別版の使い分けは[二項係数](Combination/README.md)・[ゼータ／メビウス変換](Zeta/README.md)を参照してください。Mintは利用側で用意します。

問題URL、追加APIの補足検査、現在版の判定状況は[Verify一覧](../Verify/current/README.md)を参照してください。

64ビット素数判定・指定底の検査・前後の素数探索は[説明](../Document/primality.md)と[使用例](../Examples/primality_example.cpp)を参照してください。

64ビット整数による分数の四則演算・厳密比較・丸めは[説明](../Document/fraction.md)と[使用例](../Examples/fraction_example.cpp)を参照してください。

## 多項式

| ファイル | 対応する検証 |
| --- | --- |
| [NTT.cpp](NTT.cpp) | [Convolution (Mod 998244353)](../Verify/current/submissions/ntt_convolution.cpp) |
