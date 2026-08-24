/*
 * linalg.c -- small dense linear algebra shared across the suite.
 *
 * Everything here is sized for a DOE, not for a solver library: the matrices
 * are k*k with k a factor count, so at most a handful of rows. That budget
 * buys explicit, allocation-light code with no iteration counts to tune.
 */

#include "doe.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define EIGEN_MAX_SWEEPS 60
#define EIGEN_MAX_N      16

/*
 * Symmetric eigendecomposition by cyclic Jacobi.
 *
 * Jacobi is the right choice at this size: it is unconditionally convergent
 * for symmetric input, it needs no balancing or shift strategy, and it gets
 * the SMALL eigenvalues right. That last property is the whole point here --
 * a near-zero eigenvalue is not noise to be tolerated, it is the answer (a
 * flat direction in a response surface), so a method that computes small
 * eigenvalues to low relative accuracy would defeat the purpose.
 */
int doe_eigen_sym(const double *A, size_t n, double *vals, double *vecs,
                  char *err) {
    if (!A || !vals || !vecs || n == 0 || n > EIGEN_MAX_N) {
        if (err) snprintf(err, DOE_ERR_SIZE,
                          "eigen: need 1..%d dimensions, got %zu",
                          EIGEN_MAX_N, n);
        return -1;
    }
    double a[EIGEN_MAX_N * EIGEN_MAX_N];
    for (size_t i = 0; i < n * n; i++) {
        if (!isfinite(A[i])) {
            if (err) snprintf(err, DOE_ERR_SIZE,
                              "eigen: matrix entry %zu is not finite", i);
            return -1;
        }
        a[i] = A[i];
    }
    /* Symmetrise defensively: callers build B from a fit, and a rounding
     * asymmetry would otherwise leak into the rotations. */
    for (size_t i = 0; i < n; i++)
        for (size_t j = i + 1; j < n; j++) {
            double m = 0.5 * (a[i * n + j] + a[j * n + i]);
            a[i * n + j] = a[j * n + i] = m;
        }

    /* vecs accumulates the rotations; rows end up as the eigenvectors. */
    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++) vecs[i * n + j] = (i == j) ? 1.0 : 0.0;

    double scale = 0.0;
    for (size_t i = 0; i < n * n; i++)
        if (fabs(a[i]) > scale) scale = fabs(a[i]);
    if (scale == 0.0) {                       /* the zero matrix: all of it */
        for (size_t i = 0; i < n; i++) vals[i] = 0.0;
        return 0;
    }

    /*
     * Sweep until the off-diagonal is gone, where "gone" means each entry is
     * negligible against the two diagonal entries it sits between -- an entry
     * so small that rotating it away would not change either of them in
     * double precision. Those entries are zeroed outright, so the sum below
     * reaches exactly 0.0 and the loop terminates on an equality rather than
     * on a tolerance anyone has to justify.
     *
     * Comparing the SUM OF SQUARES against a scale-derived tolerance instead
     * is the tempting shortcut and it is wrong: squaring means a threshold
     * that looks like 1e-18 accepts off-diagonal entries around 1e-9, which
     * is real error in the eigenvectors, not rounding. That mistake survived
     * three hand-picked test matrices here and only fell over on the fourth
     * random one.
     */
    int swept = 0;
    for (; swept < EIGEN_MAX_SWEEPS; swept++) {
        double off = 0.0;
        for (size_t i = 0; i < n; i++)
            for (size_t j = i + 1; j < n; j++) off += a[i * n + j] * a[i * n + j];
        if (off == 0.0) break;

        for (size_t p = 0; p < n; p++) {
            for (size_t q = p + 1; q < n; q++) {
                double apq = a[p * n + q];
                if (apq == 0.0) continue;
                double big = 100.0 * fabs(apq);
                if (fabs(a[p * n + p]) + big == fabs(a[p * n + p]) &&
                    fabs(a[q * n + q]) + big == fabs(a[q * n + q])) {
                    a[p * n + q] = a[q * n + p] = 0.0;
                    continue;
                }
                /* theta picks the rotation that zeroes (p,q); the sign choice
                 * on t keeps |t| <= 1, which is what makes Jacobi stable. */
                double theta = (a[q * n + q] - a[p * n + p]) / (2.0 * apq);
                double t = (theta >= 0.0 ? 1.0 : -1.0)
                         / (fabs(theta) + sqrt(theta * theta + 1.0));
                double c = 1.0 / sqrt(t * t + 1.0);
                double s = t * c;

                for (size_t i = 0; i < n; i++) {
                    double aip = a[i * n + p], aiq = a[i * n + q];
                    a[i * n + p] = c * aip - s * aiq;
                    a[i * n + q] = s * aip + c * aiq;
                }
                for (size_t j = 0; j < n; j++) {
                    double apj = a[p * n + j], aqj = a[q * n + j];
                    a[p * n + j] = c * apj - s * aqj;
                    a[q * n + j] = s * apj + c * aqj;
                }
                for (size_t i = 0; i < n; i++) {
                    double vpi = vecs[p * n + i], vqi = vecs[q * n + i];
                    vecs[p * n + i] = c * vpi - s * vqi;
                    vecs[q * n + i] = s * vpi + c * vqi;
                }
            }
        }
    }
    if (swept == EIGEN_MAX_SWEEPS) {
        if (err) snprintf(err, DOE_ERR_SIZE,
                          "eigen: no convergence in %d sweeps", EIGEN_MAX_SWEEPS);
        return -1;
    }
    for (size_t i = 0; i < n; i++) vals[i] = a[i * n + i];

    /*
     * Deterministic ordering and sign. Both are load-bearing, not tidiness:
     * the output is committed to JSON and to the examples suite, so an
     * eigenpair that permuted or flipped between runs would fail a diff.
     * Descending |value| puts the stiffest direction first and leaves the
     * flat ones -- the interesting ones -- last, together.
     */
    for (size_t i = 0; i + 1 < n; i++) {
        size_t best = i;
        for (size_t j = i + 1; j < n; j++)
            if (fabs(vals[j]) > fabs(vals[best])) best = j;
        if (best == i) continue;
        double tv = vals[i]; vals[i] = vals[best]; vals[best] = tv;
        for (size_t c = 0; c < n; c++) {
            double t2 = vecs[i * n + c];
            vecs[i * n + c] = vecs[best * n + c];
            vecs[best * n + c] = t2;
        }
    }
    for (size_t i = 0; i < n; i++) {
        size_t lead = 0;
        for (size_t c = 1; c < n; c++)
            if (fabs(vecs[i * n + c]) > fabs(vecs[i * n + lead])) lead = c;
        if (vecs[i * n + lead] < 0.0)
            for (size_t c = 0; c < n; c++) vecs[i * n + c] = -vecs[i * n + c];
    }
    return 0;
}
