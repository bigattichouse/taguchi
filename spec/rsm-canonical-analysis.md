# rsm — canonical analysis

*Completes E4, which specs "stationary-point **+ canonical analysis**"
(EXPANSION.md §E4) but shipped only the stationary point. Written 2026-08-24.*

> **LANDED 2026-08-24.** Built as specified except in three places, corrected
> below: the flat test needed a numerical floor as well as the statistical one
> (§4), `none` is decided by curvature alone rather than by curvature and slope
> (§5), and a stationary ridge reports `is_the_optimum_sought: true` rather
> than false (§6).

---

## 1. Why: two measured failures

`rsm analyze` classifies the stationary point from the definiteness of `B`
using Sylvester's criterion on leading principal minors
(`optimize/rsm/src/cli/main.c:278-324`). Two failures were reproduced against
the built binary on `examples/cookies/rsm.space`.

**Failure 1 — a stationary ridge is reported as a point maximum.**
Response `y = 100 − 4(u+v)²` in coded units — a perfect ridge along `u = −v`,
every point on which scores exactly 100:

```
Stationary point is a maximum
Predicted yield: 100
butter    0    0.75
temp     -0    375
```

The design's own run 2 (butter 0.8914, temp 353.79) also scores 100. The tool
names one arbitrary point on a line of equally-good settings and flags nothing.
`within_design_region` is true, so no guardrail fires. This is a silent wrong
answer.

**Failure 2 — the verdict flips on noise-floor perturbations.**
Adding `−ε·u²` to the same surface:

| ε | verdict |
|---|---|
| +1e-3 | maximum |
| −1e-3 | saddle |
| +1e-7 | maximum |
| −1e-7 | saddle |

A curvature term 4×10⁷ times smaller than the dominant curvature decides
between "here is your optimum" and "there is no interior optimum." No
experiment resolves ε at that scale, so on a real ridge the verdict is
decided by rounding.

**Rising ridges are caught, but misdiagnosed.** With a linear term added, the
stationary point flies off as `1/ε` (coded 2.5 → 25 → 250 for ε = 1e-2 →
1e-4), and the existing `within_design_region` check correctly refuses it as
extrapolation. But the advice it gives — "re-centre the ranges on it and run
again" — sends you chasing a ridge that recedes every time. The correct
advice is to move *along* the ridge direction, which requires knowing that
direction.

## 2. Why Sylvester's criterion cannot be patched

Not a threshold-tuning problem. **Sylvester's criterion tests strict
definiteness only.** Positive/negative *semi*definiteness requires all
`2ᵏ−1` principal minors, not the `k` leading ones — and semidefinite `B` is
exactly what a ridge is. The instrument cannot express the answer, at any
tolerance.

Eigenvalues can: `λᵢ = 0` *is* the ridge, and `|λᵢ|` measured against its own
standard error is a test you can actually run.

## 3. Decision: replace, do not run in parallel

Replace the minor-determinant loop entirely.

- **Strict superset.** Eigenvalues return the same three verdicts plus
  magnitudes and directions. Nothing is lost.
- **Two paths would disagree exactly where it matters.** Near-singular `B` is
  both the interesting case and the case where the two methods diverge, and
  there is no principled tiebreaker between them.
- **Cost is roughly neutral.** Cyclic Jacobi (~60 lines) replaces the
  determinant loop (~40 lines, `main.c:286-321`). Net ≈ +20 lines in `rsm`,
  minus what moves to `libdoe`.

## 4. The mathematics

Fitted model, coded units:  `ŷ = b₀ + xᵀb + xᵀBx`,
with `B_ii = quadᵢ`, `B_ij = cross_ij/2` (already built at `main.c:260-270`).

Stationary point `x_s = −½B⁻¹b` (already solved at `main.c:272-276`).

**Canonical form.** With `Q` the orthonormal eigenvectors of `B` and `λ` its
eigenvalues, substituting `w = Qᵀ(x − x_s)`:

```
ŷ = ŷ_s + Σ λᵢ wᵢ²
```

Each `λᵢ` is the curvature along canonical direction `qᵢ`. Verdicts:

| condition | kind | meaning |
|---|---|---|
| all `λᵢ < 0` | `maximum` | interior peak |
| all `λᵢ > 0` | `minimum` | interior bowl |
| mixed signs, none flat | `saddle` | no interior optimum |
| some `λᵢ ≈ 0`, `gᵢ ≈ 0` | `stationary_ridge` | a line/plane of equally good settings |
| some `λᵢ ≈ 0`, `gᵢ ≠ 0` | `rising_ridge` | surface climbs along `qᵢ`; no interior optimum |

where `gᵢ = qᵢᵀb` is the linear coefficient resolved along `qᵢ`. This split is
what separates the two measured failures: Failure 1 is a stationary ridge,
Failure 2's linear-term variant is a rising ridge.

**Testing `λᵢ ≈ 0` honestly.** `λᵢ` is *linear* in the fitted coefficients:

```
λᵢ = Σ_a q_ia² β_aa  +  Σ_{a<b} q_ia q_ib β_ab
```

(the `½` in `B_ij` cancels against the term appearing twice). So with
`c_i` that coefficient vector, `Var(λᵢ) = c_iᵀ Σ_β c_i` exactly, to first
order in `Q`, where `Σ_β = σ̂²(XᵀX)⁻¹` and `σ̂² = SSE/(n−p)`. For `k=2`:
`n=11`, `p=6`, `df=5`. `gᵢ` is linear in `β` the same way and gets the same
treatment.

**Flat test:** direction `i` is flat when `|λᵢ| < t_{df,0.975} · se(λᵢ)`.
**Rising test:** given flat `i`, the ridge rises when `|gᵢ| ≥ t_{df,0.975} · se(gᵢ)`.

This is a statistical test against the fit's own residual scale, not a magic
constant — which is the difference between reporting a ridge and guessing at
one. The CCD's centre replicates (runs 9–11 for `k=2`) additionally supply a
pure-error estimate; using it in place of `σ̂²` is a possible refinement, not
required for the first cut.

## 5. Implementation

**`core/libdoe` — new shared plumbing** (per 1d49738, "no tool keeps its own
copy of the suite's plumbing"; a symmetric eigensolver is suite-level, and
`taguchi robust` is a plausible second consumer):

```c
/* Symmetric eigendecomposition by cyclic Jacobi. A is n*n row-major and is
 * not modified. vals receives n eigenvalues sorted by DESCENDING |value|, so
 * the stiffest direction is first and flat directions land last; vecs
 * receives the matching orthonormal eigenvectors as rows. Deterministic:
 * sweeps to a fixed off-diagonal tolerance, sign-normalised so each vector's
 * largest-magnitude component is positive. Returns 0, or -1 with err filled
 * (non-finite entry, no convergence in the sweep cap). */
int doe_eigen_sym(const double *A, size_t n, double *vals, double *vecs, char *err);
```

Sign normalisation and a fixed sort order are load-bearing: without them the
JSON output is not reproducible run to run, and `examples/` commits its
outputs and tests that they stay true (ffc70ce).

**`optimize/rsm/src/cli/main.c`:**

1. Retain `(XᵀX)⁻¹`. The local `solve` (`main.c:42`) destroys `A`; extend it
   to `p` right-hand sides (identity) or copy `A` first. `p ≤ 10`, so cost is
   irrelevant.
2. Compute `SSE` over the `n` runs from the fitted coefficients, and `σ̂²`.
3. Call `doe_eigen_sym(B, k, ...)`.
4. Replace `main.c:278-324` with the verdict table in §4.
5. `flat` (the rank-deficient `2Bx = −b` solve at `main.c:276`) stops being a
   separate "none" outcome and becomes a *consequence* of a zero eigenvalue —
   the ridge branch owns it. `none` is reserved for the genuine plane: **every**
   `λᵢ` tests flat. The spec first said this also required every `gᵢ` to test
   zero, which is wrong — a plane's whole point is that it has a nonzero
   gradient, so that rule would have classified every plane as a rising ridge
   and lost an outcome the tool already reported correctly. Curvature alone
   decides it: no curvature anywhere is a plane, no curvature in *some*
   direction is a ridge.

6. The stationary point comes out of the decomposition rather than a second
   solve: `x_s = Σ over the curved directions of −gᵢ/(2λᵢ) · qᵢ`. Skipping the
   flat directions is the minimum-norm solution, so it is well defined when `B`
   is singular and returns the point on the ridge nearest the design centre —
   the representative worth quoting. The local `solve` helper is replaced
   outright by a Gauss-Jordan `invert`, since `(XᵀX)⁻¹` is needed anyway and
   nothing else called it.

## 6. Output contract

**Text**, stationary-ridge case:

```
Response surface for 'yield' (maximizing), 11 runs

Stationary ridge -- no single best setting
Predicted yield: 100, anywhere along the ridge

direction     curvature      eigenvalue
d1            strong (max)      -8.0000
d2            flat  -- ridge     0.0000  +/- 0.0021

The surface does not change along d2:
  butter   +0.707
  temp     -0.707
Any setting on that line scores the same. Fix the combination
wherever it is cheapest to hold -- that tolerance is free.
```

Rising-ridge case replaces the closing paragraph with the direction of
improvement (`+q_i` or `−q_i` by the sign of `gᵢ`) and says the surface is
still climbing at the edge of the region, so the next design should be
displaced along it — the advice that Failure 2 currently gets wrong.

**JSON** — bump `RSM_JSON_SCHEMA` to `2`:

- `stationary_point_kind` gains `"stationary_ridge"` and `"rising_ridge"`.
- New `canonical` array, one entry per direction, in eigensolver order:
  `{"direction": "d1", "eigenvalue": -8.0, "se": 0.0021, "flat": false,
    "along": [{"factor": "butter", "loading": 0.707}, ...]}`
- `is_the_optimum_sought` is `true` for a **stationary ridge** whose curved
  directions all match the objective, and `false` for a rising ridge. The spec
  first said false for both. That was wrong: a ridge of maxima, when you are
  maximising, *is* the maximum — the predicted value is genuinely the best
  achievable and the settings list names a point that attains it. What it is
  not is *unique*, and `stationary_point_kind` plus the `canonical` array is
  where that gets said. Reporting false would have implied the tool had failed
  to find the optimum when it had found an entire line of them.
- Also new: `residual_std_error` and `residual_df`, so a consumer can see the
  scale the flat test judged against rather than having to trust it.
- Each `canonical` entry also carries `slope` (`gᵢ`), which is what separates
  the two ridge kinds.
- Existing keys keep their meanings.

Consumers are only `optimize/rsm/tests/test_rsm_cli.sh` and
`examples/tests/test_examples.sh:112` — no other binary parses this — so the
vocabulary extension is contained.

## 7. Validation

Add to `make validate` (EXPANSION.md §E4 already carries the row: "degenerate
fits (saddle, rank-deficient) produce clean errors"):

1. **Known bowl** — existing case, must still report `minimum` with the
   optimum recovered to tolerance. Non-regression.
2. **Stationary ridge** — `y = 100 − 4(u+v)²`. Must report
   `stationary_ridge`, `λ = [−8, 0]`, and the flat direction
   `(+0.707, −0.707)` to tolerance. *This is Failure 1; it currently reports
   `maximum`.*
3. **Rising ridge** — ridge plus a linear term along the flat direction. Must
   report `rising_ridge` and the improving direction, not an extrapolated
   point. *This is Failure 2.*
4. **Perturbation stability** — case 2 with `ε = ±1e-7`, noiseless. Both signs
   must give the same verdict (`stationary_ridge`). *Currently flips
   maximum/saddle.* Only the numerical floor can settle this one.
4b. **The statistical flat test** — case 2 with `ε = ±1e-3`. Noiseless, that
   curvature is real and must be reported as a `maximum`; add measurement
   noise and the same curvature must become `stationary_ridge`, because
   `λ₂ = 0.018 ± 0.054` is not distinguishable from zero. Only the t-test can
   settle this one, and no fixed threshold could settle both.
5. **True saddle** — `x² − y²`, must still report `saddle` with `λ = [+2, −2]`,
   neither flat.
6. **True plane** — must still report `none`.

Cases 2–4 fail against the current build; that is the point of them.

## 8. Scope

Two or three factors, unchanged. No new design type, no new command, no new
binary — `rsm analyze` gains output it should already have had. Per
EXPANSION.md's additive rule nothing is retired: every verdict the tool
reports today it still reports, on strictly better evidence.

Roadmap bookkeeping: STATUS.md:45 records E4 as "central composite design,
quadratic fit, stationary point," which is accurate to the build but drops the
canonical-analysis clause EXPANSION.md:151 promises. Land this and the two
agree; until then E4 is partial, not complete.

## 9. As built

| | |
|---|---|
| `core/src/linalg.c` | new — `doe_eigen_sym`, cyclic Jacobi, 137 lines |
| `core/include/doe.h` | +1 declaration, new "Linear algebra" section |
| `core/tests/test_doe.c` | +4 tests (29 total, valgrind clean) |
| `optimize/rsm/src/cli/main.c` | `solve` → `invert`; classifier replaced; canonical output |
| `optimize/rsm/tests/test_rsm_cli.sh` | 29 → 56 assertions |

`rsm` is the only caller of `doe_eigen_sym`. Of the 26 committed example
outputs, only `5-rsm-analysis.txt` changed — every other tool is byte-identical,
which is the check that says this touched nothing it should not have.

The tutorial's own temp × time surface now reports `stationary_ridge`. Its
README described that case as a ridge in prose while the tool called it a
saddle; the two now agree.
