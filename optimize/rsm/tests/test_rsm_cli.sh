#!/usr/bin/env bash
# test_rsm_cli.sh — the rsm binary (E4).
#
# EXPANSION.md states the validation: "recovers the known optimum of a
# synthetic quadratic bowl to tolerance; degenerate fits (saddle,
# rank-deficient) produce clean errors." So plant a bowl and check the tool
# finds it, then hand it surfaces that have no optimum to find.
set -u
BIN="${BIN:-build/bin}"
RSM="$BIN/rsm"
TMP="build/rsm_cli_$$"
pass=0; fail=0
cleanup() { rm -rf "$TMP"; }
trap cleanup EXIT
[ -x "$RSM" ] || { echo "rsm not found at $RSM" >&2; exit 2; }
mkdir -p "$TMP" || exit 2

ok()  { pass=$((pass+1)); echo "  PASS: $1"; }
bad() { fail=$((fail+1)); echo "  FAIL: $1  ($2)"; }
expect_exit() { local w="$1" l="$2"; shift 2; "$@" >"$TMP/o" 2>"$TMP/e"; local g=$?
    [ "$g" -eq "$w" ] && ok "$l" || bad "$l" "exit $g, wanted $w"; }
expect_match() { local p="$1" l="$2"; shift 2; "$@" >"$TMP/o" 2>"$TMP/e"
    grep -q -- "$p" "$TMP/o" "$TMP/e" && ok "$l" || bad "$l" "no '$p'"; }

HAVE_PY=0
command -v python3 >/dev/null 2>&1 && HAVE_PY=1
skip() { echo "  SKIP: $1  (no python3)"; }
json_is() { local want="$1" expr="$2" l="$3"; shift 3; "$@" >"$TMP/o" 2>"$TMP/e"
    [ "$HAVE_PY" -eq 1 ] || { skip "$l"; return; }
    local got
    got=$(python3 -c 'import json,sys
d = json.load(open(sys.argv[1]))
print(eval(sys.argv[2]))' "$TMP/o" "$expr" 2>"$TMP/pe")
    [ "$got" = "$want" ] && ok "$l" || bad "$l" "got '${got:-$(head -1 "$TMP/pe")}', wanted '$want'"; }

cat > "$TMP/s.space" <<'EOF'
factors:
  x: -10,10
  y: -10,10
seed: 1
EOF

# ---- the design ---------------------------------------------------------
expect_exit 0 "sample runs" "$RSM" sample "$TMP/s.space"
n=$("$RSM" sample "$TMP/s.space" | tail -n +2 | wc -l)
# 2^2 corners + 2*2 axial + 3 centres
[ "$n" -eq 11 ] && ok "sample emits corners + axial + centres = 11" \
                || bad "sample emits corners + axial + centres = 11" "$n rows"
# The axial points must reach BEYOND the corners -- that is what makes the
# pure quadratic terms estimable at all.
python3 - "$TMP/s.space" <<'PY' >/dev/null 2>&1 && ok "axial points extend past the corners" \
    || bad "axial points extend past the corners" "see above"
import subprocess, sys, csv, io
out = subprocess.run(["build/bin/rsm", "sample", sys.argv[1]], capture_output=True, text=True).stdout
rows = list(csv.DictReader(io.StringIO(out)))
xs = sorted(abs(float(r["x"])) for r in rows)
assert xs[-1] > xs[len(xs)//2] + 1e-9, "no point reaches past the corner radius"
PY

# ---- the stated validation: recover a planted optimum -------------------
# A bowl peaking at x=3, y=-4 with value 100.
{ echo "run_id,response"; "$RSM" sample "$TMP/s.space" | tail -n +2 \
  | awk -F, '{printf "%d,%.10g\n", NR, 100 - 2*($2-3)*($2-3) - 3*($3+4)*($3+4)}'; } > "$TMP/bowl.csv"

expect_exit 0 "analyze runs" "$RSM" analyze "$TMP/s.space" "$TMP/bowl.csv"
expect_match "maximum" "finds a maximum" "$RSM" analyze "$TMP/s.space" "$TMP/bowl.csv"
json_is "maximum" "d['stationary_point_kind']" "json: reports a maximum" \
    "$RSM" analyze "$TMP/s.space" "$TMP/bowl.csv" --json
json_is "True" "abs([s['value'] for s in d['settings'] if s['factor']=='x'][0] - 3) < 1e-6" \
    "recovers the planted x = 3" \
    "$RSM" analyze "$TMP/s.space" "$TMP/bowl.csv" --json
json_is "True" "abs([s['value'] for s in d['settings'] if s['factor']=='y'][0] + 4) < 1e-6" \
    "recovers the planted y = -4" \
    "$RSM" analyze "$TMP/s.space" "$TMP/bowl.csv" --json
json_is "True" "abs(d['predicted'] - 100) < 1e-6" "recovers the peak value 100" \
    "$RSM" analyze "$TMP/s.space" "$TMP/bowl.csv" --json
json_is "True" "d['is_the_optimum_sought']" "a maximum is what --maximize wanted" \
    "$RSM" analyze "$TMP/s.space" "$TMP/bowl.csv" --json
json_is "True" "d['within_design_region']" "and it lies inside the region run" \
    "$RSM" analyze "$TMP/s.space" "$TMP/bowl.csv" --json
# Asking to minimise a bowl must say the surface turns the wrong way.
json_is "False" "d['is_the_optimum_sought']" "a maximum is NOT what --minimize wanted" \
    "$RSM" analyze "$TMP/s.space" "$TMP/bowl.csv" --minimize --json
expect_match "asked to minimize" "and says so" \
    "$RSM" analyze "$TMP/s.space" "$TMP/bowl.csv" --minimize

# An inverted bowl must be found as a minimum, or the verdict is not reading
# the surface at all.
{ echo "run_id,response"; "$RSM" sample "$TMP/s.space" | tail -n +2 \
  | awk -F, '{printf "%d,%.10g\n", NR, 2*($2-3)*($2-3) + 3*($3+4)*($3+4)}'; } > "$TMP/cup.csv"
json_is "minimum" "d['stationary_point_kind']" "an inverted bowl is a minimum" \
    "$RSM" analyze "$TMP/s.space" "$TMP/cup.csv" --json

# ---- degenerate fits ----------------------------------------------------
{ echo "run_id,response"; "$RSM" sample "$TMP/s.space" | tail -n +2 \
  | awk -F, '{printf "%d,%.10g\n", NR, $2*$2 - $3*$3}'; } > "$TMP/saddle.csv"
json_is "saddle" "d['stationary_point_kind']" "x^2 - y^2 is a saddle" \
    "$RSM" analyze "$TMP/s.space" "$TMP/saddle.csv" --json
json_is "False" "d['is_the_optimum_sought']" "and a saddle is not an optimum" \
    "$RSM" analyze "$TMP/s.space" "$TMP/saddle.csv" --json

{ echo "run_id,response"; "$RSM" sample "$TMP/s.space" | tail -n +2 \
  | awk -F, '{printf "%d,%.10g\n", NR, 3*$2 + 2*$3}'; } > "$TMP/plane.csv"
expect_match "No stationary point" "a plane has no turning point" \
    "$RSM" analyze "$TMP/s.space" "$TMP/plane.csv"
json_is "none" "d['stationary_point_kind']" "json: reports none for a plane" \
    "$RSM" analyze "$TMP/s.space" "$TMP/plane.csv" --json

# An optimum outside the region run is extrapolation, and must be labelled.
{ echo "run_id,response"; "$RSM" sample "$TMP/s.space" | tail -n +2 \
  | awk -F, '{printf "%d,%.10g\n", NR, 100 - 2*($2-90)*($2-90) - 3*($3+4)*($3+4)}'; } > "$TMP/far.csv"
json_is "False" "d['within_design_region']" "an optimum outside the region is flagged" \
    "$RSM" analyze "$TMP/s.space" "$TMP/far.csv" --json
expect_match "EXTRAPOLATION" "and the table says so" \
    "$RSM" analyze "$TMP/s.space" "$TMP/far.csv"

# ---- canonical analysis: ridges -----------------------------------------
#
# EXPANSION.md E4 specs "stationary-point + canonical analysis". The canonical
# half is what makes a RIDGE reportable. Sylvester's criterion on the leading
# principal minors -- which this tool used to classify with -- tests strict
# definiteness only; a ridge is the semidefinite case and escapes it, so these
# surfaces used to come back as a confident point optimum or as a saddle
# depending on rounding.

# Every ordinary fit still gets a canonical table, and a real optimum has no
# flat direction in it.
json_is "2" "d['schema']" "schema is 2" \
    "$RSM" analyze "$TMP/s.space" "$TMP/bowl.csv" --json
json_is "2" "len(d['canonical'])" "a 2-factor fit has 2 canonical directions" \
    "$RSM" analyze "$TMP/s.space" "$TMP/bowl.csv" --json
json_is "False" "any(c['flat'] for c in d['canonical'])" \
    "a genuine peak has no flat direction" \
    "$RSM" analyze "$TMP/s.space" "$TMP/bowl.csv" --json
json_is "True" "all(c['eigenvalue'] < 0 for c in d['canonical'])" \
    "and both its curvatures are negative" \
    "$RSM" analyze "$TMP/s.space" "$TMP/bowl.csv" --json

# A STATIONARY RIDGE: y = 100 - 4(u+v)^2 in coded units, so every setting on
# the line u = -v scores exactly 100. Naming one point as "the" maximum is a
# wrong answer, not an imprecise one.
{ echo "run_id,response"; "$RSM" sample "$TMP/s.space" | tail -n +2 \
  | awk -F, '{printf "%d,%.12g\n", NR, 100 - 0.08*($2+$3)*($2+$3)}'; } > "$TMP/ridge.csv"

json_is "stationary_ridge" "d['stationary_point_kind']" \
    "a flat ridge is reported as a ridge, not a point" \
    "$RSM" analyze "$TMP/s.space" "$TMP/ridge.csv" --json
json_is "True" "abs(d['canonical'][0]['eigenvalue'] + 8) < 1e-6" \
    "the curved direction has eigenvalue -8" \
    "$RSM" analyze "$TMP/s.space" "$TMP/ridge.csv" --json
json_is "True" "d['canonical'][1]['flat'] and not d['canonical'][0]['flat']" \
    "exactly one direction is flat, and it sorts last" \
    "$RSM" analyze "$TMP/s.space" "$TMP/ridge.csv" --json
# The flat direction is (1,-1)/sqrt2 -- the two loadings equal in size and
# opposite in sign. Either overall sign names the same line.
json_is "True" "abs(abs(d['canonical'][1]['along'][0]['loading']) - 0.70710678) < 1e-4" \
    "the flat direction is the (1,-1) diagonal" \
    "$RSM" analyze "$TMP/s.space" "$TMP/ridge.csv" --json
json_is "True" "d['canonical'][1]['along'][0]['loading'] * d['canonical'][1]['along'][1]['loading'] < 0" \
    "and its two loadings oppose" \
    "$RSM" analyze "$TMP/s.space" "$TMP/ridge.csv" --json
json_is "True" "abs(d['predicted'] - 100) < 1e-6" "the ridge's value is still recovered" \
    "$RSM" analyze "$TMP/s.space" "$TMP/ridge.csv" --json
# A ridge of maxima IS the maximum you asked for -- it just is not unique.
json_is "True" "d['is_the_optimum_sought']" \
    "a ridge of maxima is the optimum sought" \
    "$RSM" analyze "$TMP/s.space" "$TMP/ridge.csv" --json
expect_match "Stationary ridge" "the text says ridge" \
    "$RSM" analyze "$TMP/s.space" "$TMP/ridge.csv"
expect_match "tolerance" "and says the tolerance is free" \
    "$RSM" analyze "$TMP/s.space" "$TMP/ridge.csv"
# The representative point must lie ON the ridge (u = -v, i.e. x = -y).
json_is "True" "abs(d['settings'][0]['value'] + d['settings'][1]['value']) < 1e-6" \
    "the point it names lies on the ridge" \
    "$RSM" analyze "$TMP/s.space" "$TMP/ridge.csv" --json

# A RISING RIDGE: the same surface with a slope along the flat direction. There
# is no interior optimum; the old advice ("re-centre on the stationary point")
# would chase a point that recedes as fast as you follow it.
{ echo "run_id,response"; "$RSM" sample "$TMP/s.space" | tail -n +2 \
  | awk -F, '{printf "%d,%.12g\n", NR, 100 - 0.08*($2+$3)*($2+$3) + 0.42426407*($2-$3)}'; } > "$TMP/rising.csv"

json_is "rising_ridge" "d['stationary_point_kind']" \
    "a sloped ridge is a rising ridge" \
    "$RSM" analyze "$TMP/s.space" "$TMP/rising.csv" --json
json_is "False" "d['is_the_optimum_sought']" \
    "and a rising ridge is not an optimum" \
    "$RSM" analyze "$TMP/s.space" "$TMP/rising.csv" --json
expect_match "no interior optimum" "and says there is none to find" \
    "$RSM" analyze "$TMP/s.space" "$TMP/rising.csv"
expect_match "ALONG" "and says to move along the ridge, not re-centre on it" \
    "$RSM" analyze "$TMP/s.space" "$TMP/rising.csv"

# STABILITY. Perturbing the ridge's curvature by 1e-7 -- far below anything an
# experiment resolves -- used to flip the verdict between "maximum" and
# "saddle". Both signs must now land on the same answer.
for sgn in 1 -1; do
  { echo "run_id,response"; "$RSM" sample "$TMP/s.space" | tail -n +2 \
    | awk -F, -v g=$sgn '{printf "%d,%.14g\n", NR, 100 - 0.08*($2+$3)*($2+$3) - g*1e-7*0.02*$2*$2}'; } \
    > "$TMP/pert$sgn.csv"
done
p_plus=$("$RSM" analyze "$TMP/s.space" "$TMP/pert1.csv" --json 2>/dev/null \
    | python3 -c 'import json,sys;print(json.load(sys.stdin)["stationary_point_kind"])' 2>/dev/null)
p_minus=$("$RSM" analyze "$TMP/s.space" "$TMP/pert-1.csv" --json 2>/dev/null \
    | python3 -c 'import json,sys;print(json.load(sys.stdin)["stationary_point_kind"])' 2>/dev/null)
if [ "$HAVE_PY" -eq 0 ]; then skip "a 1e-7 curvature flip does not change the verdict"
elif [ "$p_plus" = "$p_minus" ] && [ "$p_plus" = "stationary_ridge" ]; then
    ok "a 1e-7 curvature flip does not change the verdict"
else
    bad "a 1e-7 curvature flip does not change the verdict" "+:$p_plus -:$p_minus"
fi

# The STATISTICAL half of the flat test. At eps = 1e-3 the curvature is real on
# noiseless data and the tool should say so -- but add measurement noise and it
# stops being distinguishable from zero, which is the honest verdict. This is
# what a fixed relative threshold could not do.
{ echo "run_id,response"; "$RSM" sample "$TMP/s.space" | tail -n +2 \
  | awk -F, '{printf "%d,%.14g\n", NR, 100 - 0.08*($2+$3)*($2+$3) - 1e-3*0.02*$2*$2}'; } \
  > "$TMP/eps_clean.csv"
json_is "maximum" "d['stationary_point_kind']" \
    "noiseless, a 1e-3 curvature is real and is reported" \
    "$RSM" analyze "$TMP/s.space" "$TMP/eps_clean.csv" --json
{ echo "run_id,response"; "$RSM" sample "$TMP/s.space" | tail -n +2 \
  | awk -F, '{n=0.2*sin(NR*12.9898)*cos(NR*4.1414);
              printf "%d,%.14g\n", NR, 100 - 0.08*($2+$3)*($2+$3) - 1e-3*0.02*$2*$2 + n}'; } \
  > "$TMP/eps_noisy.csv"
json_is "stationary_ridge" "d['stationary_point_kind']" \
    "with noise, that same curvature is not distinguishable from flat" \
    "$RSM" analyze "$TMP/s.space" "$TMP/eps_noisy.csv" --json
json_is "True" "d['residual_std_error'] > 0 and d['residual_df'] == 5" \
    "and the residual scale it judged against is reported" \
    "$RSM" analyze "$TMP/s.space" "$TMP/eps_noisy.csv" --json

# A plane is still "none" and not a ridge: no curvature ANYWHERE is a different
# outcome from no curvature in one direction, and it keeps its own advice.
json_is "True" "all(c['flat'] for c in d['canonical'])" \
    "a plane is flat in every direction" \
    "$RSM" analyze "$TMP/s.space" "$TMP/plane.csv" --json

# ---- three factors ------------------------------------------------------
cat > "$TMP/t3.space" <<'EOF'
factors:
  x: -10,10
  y: -10,10
  z: -10,10
seed: 1
EOF
{ echo "run_id,response"; "$RSM" sample "$TMP/t3.space" | tail -n +2 \
  | awk -F, '{printf "%d,%.10g\n", NR, 100 - 2*($2-1)*($2-1) - 3*($3+2)*($3+2) - 4*($4-3)*($4-3)}'; } \
  > "$TMP/bowl3.csv"
json_is "maximum" "d['stationary_point_kind']" "three factors: finds the maximum" \
    "$RSM" analyze "$TMP/t3.space" "$TMP/bowl3.csv" --json
json_is "3" "len(d['canonical'])" "three factors: three canonical directions" \
    "$RSM" analyze "$TMP/t3.space" "$TMP/bowl3.csv" --json
json_is "True" "abs([s['value'] for s in d['settings'] if s['factor']=='z'][0] - 3) < 1e-6" \
    "three factors: recovers the planted z = 3" \
    "$RSM" analyze "$TMP/t3.space" "$TMP/bowl3.csv" --json
# Eigenvalues come back sorted by descending magnitude -- part of the contract,
# because the JSON is diffed against committed example output.
json_is "True" "all(abs(d['canonical'][i]['eigenvalue']) >= abs(d['canonical'][i+1]['eigenvalue']) for i in range(2))" \
    "three factors: directions sort by descending curvature" \
    "$RSM" analyze "$TMP/t3.space" "$TMP/bowl3.csv" --json

# ---- errors -------------------------------------------------------------
head -4 "$TMP/bowl.csv" > "$TMP/short.csv"
expect_exit 1 "an incomplete result set exits 1" \
    "$RSM" analyze "$TMP/s.space" "$TMP/short.csv"
expect_match "needs all of them" "and says why" \
    "$RSM" analyze "$TMP/s.space" "$TMP/short.csv"

cat > "$TMP/one.space" <<'EOF'
factors:
  x: -10,10
seed: 1
EOF
expect_exit 1 "one factor is refused" "$RSM" sample "$TMP/one.space"
expect_match "2 or 3 factors" "and says the range" "$RSM" sample "$TMP/one.space"

cat > "$TMP/four.space" <<'EOF'
factors:
  a: -1,1
  b: -1,1
  c: -1,1
  d: -1,1
seed: 1
EOF
expect_exit 1 "four factors are refused" "$RSM" sample "$TMP/four.space"

expect_exit 2 "no arguments exits 2" "$RSM"
expect_exit 2 "unknown command exits 2" "$RSM" wat "$TMP/s.space"
expect_exit 2 "unknown option exits 2" \
    "$RSM" analyze "$TMP/s.space" "$TMP/bowl.csv" --bogus
expect_exit 1 "a missing space file exits 1" "$RSM" sample "$TMP/nope.space"

echo
echo "rsm CLI tests: $pass passed, $fail failed"
[ "$fail" -eq 0 ]
