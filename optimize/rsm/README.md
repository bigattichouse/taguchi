# rsm — response surface methodology

Screening says *which* factors matter. Attribution says *how much*. This says
**what setting is best**, by fitting a quadratic over the two or three
survivors, solving for its stationary point, and reporting which way the
surface curves around it.

```sh
rsm sample  model.space > design.csv     # run these
rsm analyze model.space results.csv
```

```
Stationary point is a maximum
Predicted response: 100

factor                      coded          value
x                          0.4243              3
y                         -0.5657             -4

direction  curvature      eigenvalue  std error            x            y
d1         curves down          -150  6.388e-09        0.000        1.000
d2         curves down          -100  6.388e-09        1.000        0.000
```

(The standard errors are ~0 because this example's response is a formula with
nothing to be noisy about. On measured data they are not.)

A quadratic is the smallest model that can have an **interior** optimum, which
is the entire reason for the stage: a linear fit always points at a corner, so
it can rank factors but never locate a peak.

## The design

A **central composite**: the 2ᵏ factorial corners (which estimate the
interactions), 2k axial points at ±α, and centre replicates.

The axial points are not decoration. Corners alone cannot estimate a pure
quadratic term — every coordinate is ±1 there, so x² is 1 everywhere and the
curvature is invisible. α = (2ᵏ)^¼ makes the design *rotatable*: prediction
variance depends on distance from the centre and not on direction, so the fit
is equally trustworthy whichever way the optimum turns out to lie.

## Canonical analysis

The second table rotates the fit onto the directions the surface actually
curves along. In those coordinates the model is just

```
y = y_s + lambda_1 w_1^2 + lambda_2 w_2^2 + ...
```

so each **eigenvalue** is the curvature along one direction and the loadings
say what mix of factors that direction is. Reading it:

- **all negative** → a maximum; **all positive** → a minimum; **mixed** → a
  saddle. Same verdict line as before, now with the magnitudes behind it.
- **how big** tells you which factor to hold tightest. A curvature of −150
  punishes drift ten times harder than one of −15.
- **near zero** is the interesting one, and it gets its own verdict below.

The **std error** column is what makes "near zero" a claim rather than a
guess. Each eigenvalue is a linear combination of the fitted coefficients, so
it carries a standard error from the same fit, and a direction is called flat
when its curvature is within `t·se` of zero at the residual scale. The centre
replicates in the design are what pay for that estimate.

## Ridges

A **ridge** is a direction the surface does not curve along — and it is the
single most useful thing this stage can find, because it is free tolerance.

- **stationary ridge** — a whole line of settings scores the same. The tool
  names one representative point and says the line exists, instead of passing
  off one arbitrary point on it as *the* optimum. Fix that combination wherever
  it is cheapest to hold.
- **rising ridge** — flat in one direction but still climbing along it. There
  is no interior optimum: displace the next design **along** the ridge. Do not
  re-centre on the reported point, which recedes as fast as you follow it.

Definiteness tests on the leading principal minors — the cheaper classical
route — cannot report either one. They test *strict* definiteness, and a ridge
is the semidefinite case, so it comes back as a confident point optimum or as
a saddle depending on rounding. Eigenvalues make the ridge the answer rather
than a gap.

## What it tells you when there is no answer

- **saddle** — the surface curves up one way and down another. There is no
  interior optimum to report, and the tool says so rather than naming a point.
- **stationary ridge / rising ridge** — no *single* best setting, for the two
  different reasons above.
- **no stationary point** — the fit is a plane: no curvature in any direction,
  or none the quadratic terms can be told from zero. The optimum is on a
  boundary.
- **outside the design region** — a stationary point beyond the runs is
  **extrapolation**. Re-centre the ranges on it and run again before believing
  it.
- **wrong kind** — you asked to minimise and found a maximum. The surface turns
  the wrong way; move the ranges toward the direction that improves.

Each of those is a real outcome of a real experiment, and each is more useful
than a number that looks like an answer.

## Limits

Two or three factors. A response surface over everything is the full factorial
this toolkit exists to avoid — screen first, then bring the survivors here.

## Worked example

[**5. Where exactly is the peak?**](../../examples/cookies/#5-where-exactly-is-the-peak-rsm) — part of [one experiment carried through every tool](../../examples/cookies/),
where this one finds 369F -- a temperature that appeared in no design.
