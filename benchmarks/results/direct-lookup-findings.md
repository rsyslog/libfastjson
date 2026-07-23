# Direct child-page lookup results

The candidate replaces iterator API calls inside the private
`_fjson_find_child()` function with direct traversal of the existing child
pages. It preserves the page representation, insertion order, deleted slots,
case-sensitive and case-insensitive comparisons, and key/value ownership.

Baseline and candidate were built from revision
`f1983a831a1abd6ab8d8cdbc5137f7bc997b4018` with
`-O2 -g -fno-omit-frame-pointer`. Each complete paired session used one
alternating calibration pair followed by 11 alternating measured pairs for
all 50 workloads. The host was non-exclusive and cache state was uncontrolled.
Ratios are baseline ns/op divided by candidate ns/op; above 1 is faster.

| Profile | Session A speedup (relative MAD) | Session B speedup (relative MAD) | Result |
|---|---:|---:|---|
| Lookup width 1, first | 3.1195x (0.0137) | 3.1259x (0.0077) | pass |
| Lookup width 8, last | 2.3126x (0.0066) | 2.3627x (0.0091) | pass |
| Lookup width 16, last | 1.9043x (0.0081) | 1.9395x (0.0160), rerun | pass |
| Lookup width 32, last | 2.3398x (0.0071) | 2.3273x (0.1050), rerun | inconclusive dispersion |
| Lookup width 32, miss | 1.9420x (0.0062) | 2.0523x (0.1567), rerun | inconclusive dispersion |
| Replace width 1, first | 1.6046x (0.0065) | 1.1726x (0.0313) | pass |
| Replace width 8, last | 1.8451x (0.0110) | 1.8613x (0.0352) | pass |
| Replace width 32, last | 1.8809x (0.0010) | 1.8726x (0.0030) | pass |

Session B initially had seven workloads above the relative-MAD threshold of
0.05. A focused rerun made four conclusive. Lookup width 32 last/miss and
replace width 8 first remained noisy and are classified as inconclusive. Their
median ratios still favored the candidate, but they are not used as acceptance
evidence.

No workload in either complete session regressed by more than 5%. The lowest
core guardrail ratios were 0.9667 and 0.9708, both string workloads independent
of object lookup. Normal object construction improved as duplicate checks grew,
while known-new construction remained effectively neutral as expected.

The candidate is not retained yet: three workloads remained inconclusive after
one rerun, exceeding the documented relative-MAD limit. Stable targeted read
and replacement profiles beat the required 10% improvement in both independent
sessions, and every core guardrail stayed within the 5% regression limit. A
clean-tree rerun is needed before treating this as retained standalone
libfastjson evidence; an rsyslog end-to-end run remains a separate integration
check before attributing the full library speedup to `$!varname` throughput.
