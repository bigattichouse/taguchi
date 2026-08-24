/*
 * rsm — response surface methodology (EXPANSION.md E4).
 *
 * The stage after screening and attribution. Those answer "which factors
 * matter"; this answers "what setting is best", by fitting a QUADRATIC surface
 * over the two or three survivors and solving for its stationary point.
 *
 *   rsm sample  model.space                  -> a central composite design
 *   rsm analyze model.space results.csv      -> fit, optimum, and a verdict
 *
 * A quadratic is the smallest model that can have an interior optimum, which
 * is the whole reason for the stage: a linear fit always points at a corner,
 * so it can rank factors but never locate a peak.
 *
 * The design is a CENTRAL COMPOSITE: the 2^k factorial corners (which estimate
 * the interactions), 2k axial points at +/-alpha (which estimate the pure
 * quadratic terms -- corners alone cannot, since every coordinate is +/-1 and
 * x^2 is 1 everywhere), and centre replicates. alpha = (2^k)^(1/4) makes it
 * rotatable: prediction variance depends on distance from the centre and not
 * on direction, so the fit is equally trustworthy whichever way the optimum
 * turns out to lie.
 */

#include "doe.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define RSM_MAX_FACTORS 3        /* k=3 is 6 corners of terms; beyond that a
                                  * CCD costs more than the stage is worth */
#define RSM_CENTRE_RUNS 3
#define RSM_MAX_TERMS   10       /* 1 + k + k + k(k-1)/2 at k=3 */
#define RSM_JSON_SCHEMA 2

/* ---- linear algebra and small statistics --------------------------------
 *
 * Everything here is at most 10x10 (k=3 gives 10 quadratic terms), so plain
 * Gauss-Jordan with partial pivoting is the right tool: no iteration, no
 * library, and a singular system is detected rather than approximated. The
 * symmetric eigendecomposition the canonical analysis needs is suite-level
 * plumbing and lives in libdoe (doe_eigen_sym), not here.
 */
/*
 * Gauss-Jordan inverse. rsm needs the INVERSE rather than just a solution:
 * (X^T X)^-1 is what turns a fitted coefficient into a standard error, and
 * the standard error is what lets the canonical analysis say "this direction
 * is flat" instead of "this direction has a small number in it".
 */
static int invert(const double *A, double *inv, size_t n) {
    if (n > RSM_MAX_TERMS) return -1;
    double m[RSM_MAX_TERMS * RSM_MAX_TERMS];
    for (size_t i = 0; i < n * n; i++) m[i] = A[i];
    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++) inv[i * n + j] = (i == j) ? 1.0 : 0.0;

    for (size_t col = 0; col < n; col++) {
        size_t piv = col;
        for (size_t r = col + 1; r < n; r++)
            if (fabs(m[r * n + col]) > fabs(m[piv * n + col])) piv = r;
        if (fabs(m[piv * n + col]) < 1e-12) return -1;          /* singular */
        if (piv != col) {
            for (size_t c = 0; c < n; c++) {
                double t = m[col * n + c]; m[col * n + c] = m[piv * n + c]; m[piv * n + c] = t;
                t = inv[col * n + c]; inv[col * n + c] = inv[piv * n + c]; inv[piv * n + c] = t;
            }
        }
        double d = m[col * n + col];
        for (size_t c = 0; c < n; c++) { m[col * n + c] /= d; inv[col * n + c] /= d; }
        for (size_t r = 0; r < n; r++) {
            if (r == col) continue;
            double f = m[r * n + col];
            if (f == 0.0) continue;
            for (size_t c = 0; c < n; c++) {
                m[r * n + c]   -= f * m[col * n + c];
                inv[r * n + c] -= f * inv[col * n + c];
            }
        }
    }
    return 0;
}

/* x^T M x for a small symmetric M -- the variance of a linear combination of
 * fitted coefficients, once M is (X^T X)^-1 scaled by sigma^2. */
static double quad_form(const double *c, const double *M, size_t n) {
    double acc = 0.0;
    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++) acc += c[i] * M[i * n + j] * c[j];
    return acc;
}

/*
 * Two-sided 95% critical values of Student's t. A CCD leaves few residual
 * degrees of freedom -- 5 for k=2, 7 for k=3 -- which is exactly the regime
 * where using 1.96 instead would call a direction flat that is not.
 */
static double t_crit_95(size_t df) {
    static const double t[30] = {
        12.706, 4.303, 3.182, 2.776, 2.571, 2.447, 2.365, 2.306, 2.262, 2.228,
        2.201,  2.179, 2.160, 2.145, 2.131, 2.120, 2.110, 2.101, 2.093, 2.086,
        2.080,  2.074, 2.069, 2.064, 2.060, 2.056, 2.052, 2.048, 2.045, 2.042
    };
    if (df == 0) return 0.0;
    if (df <= 30) return t[df - 1];
    return 1.96;
}

/* ---- the design --------------------------------------------------------- */

/* Coded points in [-1,1] plus axial reach; caller maps to real values. */
static size_t ccd_points(size_t k, double *out, size_t cap) {
    size_t corners = (size_t)1 << k;
    double alpha = pow((double)corners, 0.25);
    size_t n = 0;

    for (size_t i = 0; i < corners; i++) {
        if (n >= cap) return 0;
        for (size_t f = 0; f < k; f++)
            out[n * k + f] = (i & ((size_t)1 << f)) ? 1.0 : -1.0;
        n++;
    }
    for (size_t f = 0; f < k; f++) {
        for (int sign = -1; sign <= 1; sign += 2) {
            if (n >= cap) return 0;
            for (size_t g = 0; g < k; g++) out[n * k + g] = 0.0;
            out[n * k + f] = sign * alpha;
            n++;
        }
    }
    for (size_t c = 0; c < RSM_CENTRE_RUNS; c++) {
        if (n >= cap) return 0;
        for (size_t f = 0; f < k; f++) out[n * k + f] = 0.0;
        n++;
    }
    return n;
}

/* Coded [-alpha, alpha] -> the factor's real range. The corners sit at the
 * range's edges, so the axial points reach beyond it -- that is what alpha
 * means, and the .space bounds have to be the region you can actually run. */
static double decode(const doe_space_t *sp, size_t f, double coded, double alpha) {
    double u = 0.5 + 0.5 * (coded / alpha);
    if (u < 0.0) u = 0.0;
    if (u > 1.0) u = 1.0;
    char buf[DOE_MAX_VALUE];
    return strtod(doe_factor_value(sp, f, u, buf, sizeof buf), NULL);
}

/* ---- the quadratic model ------------------------------------------------
 *
 * Terms, in this order: 1, x_i, x_i^2, x_i x_j (i<j).
 */
static size_t term_count(size_t k) { return 1 + k + k + k * (k - 1) / 2; }

static void terms_of(const double *x, size_t k, double *row) {
    size_t t = 0;
    row[t++] = 1.0;
    for (size_t i = 0; i < k; i++) row[t++] = x[i];
    for (size_t i = 0; i < k; i++) row[t++] = x[i] * x[i];
    for (size_t i = 0; i < k; i++)
        for (size_t j = i + 1; j < k; j++) row[t++] = x[i] * x[j];
}

static char *read_all(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return NULL; }
    rewind(f);
    char *b = malloc((size_t)sz + 1);
    if (!b) { fclose(f); return NULL; }
    size_t got = fread(b, 1, (size_t)sz, f);
    fclose(f);
    b[got] = '\0';
    return b;
}

static int load_space(const char *path, doe_space_t *sp) {
    char *c = read_all(path);
    if (!c) { fprintf(stderr, "Error: cannot open '%s'\n", path); return -1; }
    char err[DOE_ERR_SIZE];
    int rc = doe_space_parse(c, sp, err);
    free(c);
    if (rc != 0) { fprintf(stderr, "Error parsing %s: %s\n", path, err); return -1; }
    if (sp->factor_count < 2 || sp->factor_count > RSM_MAX_FACTORS) {
        fprintf(stderr, "Error: rsm needs 2 or %d factors, got %zu. Screen first;\n"
                        "a response surface over everything is the full factorial\n"
                        "this toolkit exists to avoid.\n",
                RSM_MAX_FACTORS, sp->factor_count);
        return -1;
    }
    return 0;
}

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s sample  <file.space>\n"
        "       %s analyze <file.space> <results.csv> [--metric NAME] [--minimize] [--json]\n"
        "\n"
        "  Response surface methodology: fits a quadratic over 2-3 factors and\n"
        "  solves for the stationary point -- the setting the surface says is\n"
        "  best, which a linear fit can never locate.\n"
        "\n"
        "  sample   central composite design (corners, axial points, centres)\n"
        "  analyze  fit, stationary point, and whether it is a max, min or saddle\n",
        prog, prog);
}

static int cmd_sample(const char *path) {
    doe_space_t sp;
    if (load_space(path, &sp) != 0) return 1;
    size_t k = sp.factor_count;
    double alpha = pow((double)((size_t)1 << k), 0.25);

    double pts[64 * RSM_MAX_FACTORS];
    size_t n = ccd_points(k, pts, 64);
    if (n == 0) { fprintf(stderr, "Error: design too large\n"); return 1; }

    printf("run_id");
    for (size_t f = 0; f < k; f++) printf(",%s", sp.factors[f].name);
    printf("\n");
    for (size_t i = 0; i < n; i++) {
        printf("%zu", i + 1);
        for (size_t f = 0; f < k; f++)
            printf(",%.10g", decode(&sp, f, pts[i * k + f], alpha));
        printf("\n");
    }
    return 0;
}

static int cmd_analyze(const char *path, const char *csv, const char *metric,
                       int minimize, int as_json) {
    doe_space_t sp;
    if (load_space(path, &sp) != 0) return 1;
    size_t k = sp.factor_count;
    double alpha = pow((double)((size_t)1 << k), 0.25);

    double pts[64 * RSM_MAX_FACTORS];
    size_t n = ccd_points(k, pts, 64);
    if (n == 0) { fprintf(stderr, "Error: design too large\n"); return 1; }

    double *y = malloc(n * sizeof *y);
    if (!y) { fprintf(stderr, "Error: out of memory\n"); return 1; }
    char err[DOE_ERR_SIZE];
    /* NaN-fills, and refuses a run the file repeats -- a duplicated row used to
     * overwrite the earlier value silently. */
    if (doe_csv_read_design(csv, metric, y, n, err) != 0) {
        fprintf(stderr, "Error reading results: %s\n", err);
        free(y);
        return 1;
    }
    for (size_t i = 0; i < n; i++) {
        if (!isfinite(y[i])) {
            fprintf(stderr, "Error: missing or non-finite response for run %zu.\n"
                            "The design has %zu runs; a quadratic fit needs all of them.\n",
                    i + 1, n);
            free(y);
            return 1;
        }
    }

    /* Normal equations for the quadratic model. */
    size_t p = term_count(k);
    double *A = calloc(p * p, sizeof *A);
    double *b = calloc(p, sizeof *b);
    double row[16];
    if (!A || !b) { fprintf(stderr, "Error: out of memory\n"); return 1; }

    for (size_t i = 0; i < n; i++) {
        terms_of(&pts[i * k], k, row);
        for (size_t r = 0; r < p; r++) {
            b[r] += row[r] * y[i];
            for (size_t c = 0; c < p; c++) A[r * p + c] += row[r] * row[c];
        }
    }
    /*
     * Fit, keeping (X^T X)^-1: the canonical analysis below needs standard
     * errors, not just coefficients.
     */
    double Ainv[RSM_MAX_TERMS * RSM_MAX_TERMS];
    if (invert(A, Ainv, p) != 0) {
        fprintf(stderr, "Error: the fit is rank-deficient -- this design cannot\n"
                        "identify a quadratic in %zu factors. That usually means the\n"
                        "results were not produced by `rsm sample`.\n", k);
        free(A); free(b); free(y);
        return 1;
    }
    double coef[RSM_MAX_TERMS];
    for (size_t r = 0; r < p; r++) {
        double acc = 0.0;
        for (size_t c = 0; c < p; c++) acc += Ainv[r * p + c] * b[c];
        coef[r] = acc;
    }
    for (size_t t = 0; t < p; t++) b[t] = coef[t];

    double b0 = b[0];
    const double *lin = &b[1];
    const double *quad = &b[1 + k];
    const double *cross = &b[1 + 2 * k];

    /* Residual variance, and the covariance of the coefficients that follows
     * from it. A CCD's centre replicates are what make this more than a
     * formality: without repeated runs there is nothing to estimate it from. */
    double sse = 0.0;
    for (size_t i = 0; i < n; i++) {
        terms_of(&pts[i * k], k, row);
        double pred = 0.0;
        for (size_t t = 0; t < p; t++) pred += b[t] * row[t];
        double resid = y[i] - pred;
        sse += resid * resid;
    }
    size_t df = n > p ? n - p : 0;
    double sigma2 = df > 0 ? sse / (double)df : 0.0;
    double tcrit = t_crit_95(df);
    double cov[RSM_MAX_TERMS * RSM_MAX_TERMS];
    for (size_t i = 0; i < p * p; i++) cov[i] = sigma2 * Ainv[i];

    /*
     * The quadratic part, as a matrix.
     *
     *   dy/dx_i = lin_i + 2*quad_i*x_i + sum_{j!=i} cross_ij * x_j
     *
     * so the stationary point solves 2B x = -lin, with B_ii = quad_i and
     * B_ij = cross_ij / 2. B is the Hessian of the fitted surface, halved.
     */
    double B[RSM_MAX_FACTORS * RSM_MAX_FACTORS] = {0};
    for (size_t i = 0; i < k; i++) B[i * k + i] = quad[i];
    {
        size_t t = 0;
        for (size_t i = 0; i < k; i++)
            for (size_t j = i + 1; j < k; j++) {
                B[i * k + j] = cross[t] / 2.0;
                B[j * k + i] = cross[t] / 2.0;
                t++;
            }
    }

    /*
     * CANONICAL ANALYSIS.
     *
     * Rotating to the eigenvectors of B turns the fit into
     *
     *     y = y_s + sum_i lambda_i * w_i^2,      w = Q^T (x - x_s)
     *
     * so each eigenvalue is the curvature along one canonical direction and
     * the classification is just their signs. Definiteness tests on the
     * leading principal minors would settle max/min/saddle more cheaply, but
     * only for STRICT definiteness -- a semidefinite B, which is exactly what
     * a ridge is, needs all 2^k-1 principal minors and so escapes them
     * entirely. Eigenvalues make the ridge the answer rather than a gap:
     * lambda_i = 0 IS the flat direction, and it can be tested against its own
     * standard error.
     */
    double lam[RSM_MAX_FACTORS], Q[RSM_MAX_FACTORS * RSM_MAX_FACTORS];
    char eerr[DOE_ERR_SIZE];
    if (doe_eigen_sym(B, k, lam, Q, eerr) != 0) {
        fprintf(stderr, "Error: cannot decompose the fitted surface: %s\n", eerr);
        free(A); free(b); free(y);
        return 1;
    }

    /* The gradient at the design centre, resolved along each direction. */
    double g[RSM_MAX_FACTORS];
    for (size_t i = 0; i < k; i++) {
        double acc = 0.0;
        for (size_t a = 0; a < k; a++) acc += Q[i * k + a] * lin[a];
        g[i] = acc;
    }

    /*
     * Standard errors. lambda_i is LINEAR in the fitted coefficients --
     *
     *     lambda_i = sum_a q_ia^2 * beta_aa + sum_{a<b} q_ia q_ib * beta_ab
     *
     * (the 1/2 in B_ij cancels against the term appearing twice) -- so its
     * variance is exact to first order in Q, no delta-method approximation
     * beyond treating the rotation as fixed. g_i is linear in the same way.
     */
    double se_lam[RSM_MAX_FACTORS], se_g[RSM_MAX_FACTORS];
    for (size_t i = 0; i < k; i++) {
        double c[RSM_MAX_TERMS];
        for (size_t t = 0; t < p; t++) c[t] = 0.0;
        for (size_t a = 0; a < k; a++) c[1 + k + a] = Q[i * k + a] * Q[i * k + a];
        {
            size_t t = 0;
            for (size_t a = 0; a < k; a++)
                for (size_t bb = a + 1; bb < k; bb++) {
                    c[1 + 2 * k + t] = Q[i * k + a] * Q[i * k + bb];
                    t++;
                }
        }
        double v = quad_form(c, cov, p);
        se_lam[i] = v > 0.0 ? sqrt(v) : 0.0;

        for (size_t t = 0; t < p; t++) c[t] = 0.0;
        for (size_t a = 0; a < k; a++) c[1 + a] = Q[i * k + a];
        v = quad_form(c, cov, p);
        se_g[i] = v > 0.0 ? sqrt(v) : 0.0;
    }

    /*
     * Is a direction flat? Two tests, and a direction has to clear both.
     *
     * The statistical one -- |lambda| within t*se of zero -- is the one that
     * matters on real data: it asks whether the curvature is distinguishable
     * from zero at the fit's own residual scale, which is the honest question.
     *
     * The numerical one -- |lambda| below 1e-6 of the coefficient scale --
     * exists because a noiseless response (a simulation, a validation case)
     * has sse = 0, and then every standard error is zero and the statistical
     * test can never fire. Without it an exactly flat ridge would be read as
     * curvature of size 1e-16.
     */
    double scale = 0.0;
    for (size_t i = 0; i < k; i++) {
        if (fabs(lam[i]) > scale) scale = fabs(lam[i]);
        if (fabs(lin[i]) > scale) scale = fabs(lin[i]);
    }
    int is_flat[RSM_MAX_FACTORS];
    size_t nflat = 0;
    for (size_t i = 0; i < k; i++) {
        double thr = tcrit * se_lam[i];
        double numeric = 1e-6 * scale;
        if (numeric > thr) thr = numeric;
        is_flat[i] = (fabs(lam[i]) <= thr);
        if (is_flat[i]) nflat++;
    }

    int anypos = 0, anyneg = 0;
    for (size_t i = 0; i < k; i++) {
        if (is_flat[i]) continue;
        if (lam[i] > 0) anypos = 1; else anyneg = 1;
    }

    /* A flat direction the surface still climbs along is a RISING ridge: no
     * interior optimum, and the fix is to move along it rather than to
     * re-centre on a stationary point that recedes as fast as you chase it. */
    int rising = 0;
    for (size_t i = 0; i < k; i++) {
        if (!is_flat[i]) continue;
        double thr = tcrit * se_g[i];
        double numeric = 1e-6 * scale;
        if (numeric > thr) thr = numeric;
        if (fabs(g[i]) > thr) rising = 1;
    }

    const char *kind;
    int no_point = 0;
    if (nflat == k) {                 /* no curvature anywhere: a plane */
        kind = "none";
        no_point = 1;
    } else if (anypos && anyneg) {
        kind = "saddle";
    } else if (nflat > 0) {
        kind = rising ? "rising_ridge" : "stationary_ridge";
    } else {
        kind = anypos ? "minimum" : "maximum";
    }

    /*
     * The stationary point, from the decomposition rather than from a second
     * solve: x_s = sum over the CURVED directions of -g_i/(2 lambda_i) q_i.
     * Skipping the flat directions is what makes this well defined when B is
     * singular -- it is the minimum-norm solution, i.e. the point on the ridge
     * closest to the design centre, which is the representative worth quoting.
     */
    double xs[RSM_MAX_FACTORS];
    for (size_t a = 0; a < k; a++) xs[a] = 0.0;
    for (size_t i = 0; i < k; i++) {
        if (is_flat[i]) continue;
        double s = -g[i] / (2.0 * lam[i]);
        for (size_t a = 0; a < k; a++) xs[a] += s * Q[i * k + a];
    }

    double y_s = b0;
    if (!no_point) {
        terms_of(xs, k, row);
        y_s = 0.0;
        for (size_t t = 0; t < p; t++) y_s += b[t] * row[t];
    }

    /* "Is this the optimum you asked for?" A stationary ridge of maxima is a
     * real maximum when you are maximising -- it just is not a unique one, and
     * the canonical table is what says so. A rising ridge never is. */
    int all_neg = anyneg && !anypos, all_pos = anypos && !anyneg;
    int wanted = 0;
    if (strcmp(kind, "maximum") == 0 || strcmp(kind, "minimum") == 0 ||
        strcmp(kind, "stationary_ridge") == 0)
        wanted = minimize ? all_pos : all_neg;

    int inside = 1;
    for (size_t i = 0; i < k; i++) if (fabs(xs[i]) > alpha) inside = 0;

    int is_ridge = (strcmp(kind, "stationary_ridge") == 0 ||
                    strcmp(kind, "rising_ridge") == 0);

    if (as_json) {
        char nb[DOE_JSON_NUM], sb[DOE_JSON_STR(DOE_MAX_NAME)];
        printf("{\n  \"tool\": \"rsm\",\n  \"command\": \"analyze\",\n");
        printf("  \"schema\": %d,\n", RSM_JSON_SCHEMA);
        printf("  \"metric\": %s,\n", doe_json_string(metric, sb, sizeof sb));
        printf("  \"objective\": \"%s\",\n", minimize ? "minimize" : "maximize");
        printf("  \"runs\": %zu,\n", n);
        printf("  \"stationary_point_kind\": \"%s\",\n", kind);
        printf("  \"is_the_optimum_sought\": %s,\n", wanted ? "true" : "false");
        printf("  \"within_design_region\": %s,\n", (!no_point && inside) ? "true" : "false");
        printf("  \"residual_std_error\": %s,\n",
               doe_json_number(sigma2 > 0 ? sqrt(sigma2) : 0.0, nb, sizeof nb));
        printf("  \"residual_df\": %zu,\n", df);
        printf("  \"predicted\": %s,\n",
               no_point ? "null" : doe_json_number(y_s, nb, sizeof nb));
        printf("  \"settings\": [\n");
        for (size_t i = 0; i < k; i++) {
            printf("    {\"factor\": %s, \"coded\": %s",
                   doe_json_string(sp.factors[i].name, sb, sizeof sb),
                   no_point ? "null" : doe_json_number(xs[i], nb, sizeof nb));
            if (!no_point) {
                char b2[DOE_JSON_NUM];
                printf(", \"value\": %s",
                       doe_json_number(decode(&sp, i, xs[i], alpha), b2, sizeof b2));
            } else {
                printf(", \"value\": null");
            }
            printf("}%s\n", i + 1 < k ? "," : "");
        }
        printf("  ],\n");
        printf("  \"canonical\": [\n");
        for (size_t i = 0; i < k; i++) {
            char b2[DOE_JSON_NUM], b3[DOE_JSON_NUM];
            printf("    {\"direction\": \"d%zu\", \"eigenvalue\": %s, "
                   "\"std_error\": %s, \"flat\": %s, \"slope\": %s,\n",
                   i + 1,
                   doe_json_number(lam[i], nb, sizeof nb),
                   doe_json_number(se_lam[i], b2, sizeof b2),
                   is_flat[i] ? "true" : "false",
                   doe_json_number(g[i], b3, sizeof b3));
            printf("     \"along\": [");
            for (size_t a = 0; a < k; a++) {
                char b4[DOE_JSON_NUM];
                printf("{\"factor\": %s, \"loading\": %s}%s",
                       doe_json_string(sp.factors[a].name, sb, sizeof sb),
                       doe_json_number(Q[i * k + a], b4, sizeof b4),
                       a + 1 < k ? ", " : "");
            }
            printf("]}%s\n", i + 1 < k ? "," : "");
        }
        printf("  ]\n}\n");
    } else {
        printf("Response surface for '%s' (%s), %zu runs\n\n",
               metric, minimize ? "minimizing" : "maximizing", n);
        if (no_point) {
            printf("No stationary point: the fitted surface has no turning point in\n"
                   "these factors -- it is a plane, or close enough that the quadratic\n"
                   "terms cannot be told from zero. The optimum is on a boundary, so\n"
                   "widen the ranges or use `grid`.\n");
        } else {
            const char *edge = inside ? "" : "  (OUTSIDE the design region)";
            if (strcmp(kind, "stationary_ridge") == 0)
                printf("Stationary ridge -- no single best setting%s\n", edge);
            else if (strcmp(kind, "rising_ridge") == 0)
                printf("Rising ridge -- the surface is still climbing%s\n", edge);
            else
                printf("Stationary point is a %s%s\n", kind, edge);

            if (strcmp(kind, "stationary_ridge") == 0)
                printf("Predicted %s: %.6g, anywhere along the ridge\n\n", metric, y_s);
            else
                printf("Predicted %s: %.6g\n\n", metric, y_s);

            printf("%-20s %12s %14s\n",
                   is_ridge ? "factor (one point)" : "factor", "coded", "value");
            printf("%-20s %12s %14s\n", "------", "-----", "-----");
            for (size_t i = 0; i < k; i++)
                printf("%-20s %12.4g %14.6g\n", sp.factors[i].name, xs[i],
                       decode(&sp, i, xs[i], alpha));

            /* The canonical table: the curvature along each direction, and
             * what mix of factors that direction is. */
            printf("\n%-10s %-12s %12s %10s", "direction", "curvature",
                   "eigenvalue", "std error");
            for (size_t a = 0; a < k; a++) printf(" %12s", sp.factors[a].name);
            printf("\n%-10s %-12s %12s %10s", "---------", "---------",
                   "----------", "---------");
            for (size_t a = 0; a < k; a++) printf(" %12s", "-------");
            printf("\n");
            for (size_t i = 0; i < k; i++) {
                const char *what = is_flat[i] ? "flat"
                                 : (lam[i] < 0 ? "curves down" : "curves up");
                printf("d%-9zu %-12s %12.4g %10.4g", i + 1, what, lam[i], se_lam[i]);
                for (size_t a = 0; a < k; a++) printf(" %12.3f", Q[i * k + a]);
                printf("\n");
            }

            if (strcmp(kind, "stationary_ridge") == 0) {
                printf("\nThe surface does not change along the flat direction");
                if (nflat > 1) printf("s");
                printf(":\n");
                for (size_t i = 0; i < k; i++) {
                    if (!is_flat[i]) continue;
                    printf("  d%zu = ", i + 1);
                    for (size_t a = 0; a < k; a++)
                        printf("%s%+.3f %s", a ? ", " : "", Q[i * k + a],
                               sp.factors[a].name);
                    printf("\n");
                }
                printf("Every setting along that line scores the same, so the point\n"
                       "above is one representative and not the only answer. Fix the\n"
                       "combination wherever it is cheapest to hold -- that tolerance\n"
                       "is free.\n");
            } else if (strcmp(kind, "rising_ridge") == 0) {
                printf("\nThe surface has no curvature along:\n");
                for (size_t i = 0; i < k; i++) {
                    if (!is_flat[i]) continue;
                    double sgn = (g[i] > 0) == (minimize == 0) ? 1.0 : -1.0;
                    printf("  d%zu, and it improves toward ", i + 1);
                    for (size_t a = 0; a < k; a++)
                        printf("%s%+.3f %s", a ? ", " : "", sgn * Q[i * k + a],
                               sp.factors[a].name);
                    printf("\n");
                }
                printf("There is no interior optimum: the ridge runs out of the region\n"
                       "rather than turning over. Displace the next design ALONG that\n"
                       "direction -- re-centring on the point above would chase a\n"
                       "stationary point that recedes as fast as you follow it.\n");
            }

            if (!wanted && !is_ridge)
                printf("\nThis is a %s and you asked to %s. The surface turns the wrong\n"
                       "way: the best setting is on the edge of the region, not inside\n"
                       "it. Move the ranges toward the direction that improves and\n"
                       "re-run.\n", kind, minimize ? "minimize" : "maximize");
            else if (!wanted && strcmp(kind, "stationary_ridge") == 0)
                printf("\nThe ridge curves the wrong way for a %s, so the line above is\n"
                       "the worst set of settings, not the best.\n",
                       minimize ? "minimize" : "maximize");
            if (!inside)
                printf("\nThe stationary point lies outside the region actually run, so\n"
                       "the value above is EXTRAPOLATION. Re-centre the ranges on it and\n"
                       "run again before believing it.\n");
        }
    }

    free(A); free(b); free(y);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) { usage(argv[0]); return 2; }
    const char *cmd = argv[1];

    if (strcmp(cmd, "sample") == 0) return cmd_sample(argv[2]);

    if (strcmp(cmd, "analyze") == 0) {
        if (argc < 4) { fprintf(stderr, "analyze needs a results.csv\n"); return 2; }
        const char *metric = "response";
        int minimize = 0, as_json = 0;
        for (int i = 4; i < argc; i++) {
            if (strcmp(argv[i], "--metric") == 0 && i + 1 < argc) metric = argv[++i];
            else if (strcmp(argv[i], "--minimize") == 0) minimize = 1;
            else if (strcmp(argv[i], "--json") == 0) as_json = 1;
            else { fprintf(stderr, "Error: unknown option '%s'\n", argv[i]); usage(argv[0]); return 2; }
        }
        return cmd_analyze(argv[2], argv[3], metric, minimize, as_json);
    }

    fprintf(stderr, "Unknown command: %s\n", cmd);
    usage(argv[0]);
    return 2;
}
