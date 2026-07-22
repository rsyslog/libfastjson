# Current-main libfastjson performance findings

Revision `f1983a831a1abd6ab8d8cdbc5137f7bc997b4018` was characterized in two
independent sessions. Each complete session contains 50 workloads and 11
measured trials per workload. All correctness checks passed. Workloads whose
relative MAD exceeded 0.05 would have been rerun once; none exceeded 0.05.

The benchmark uses libfastjson's case-insensitive comparison mode because that
is rsyslog's default for `$!` variables. Lookup objects use equal-length keys
named `key-NNN`, intentionally exercising keys with a shared prefix.

## Lookup

| Workload | Session A ns/op | Session B ns/op | Approximate cost relative to first key |
|---|---:|---:|---:|
| Width 1, first | 14.813 | 14.740 | 1.0x |
| Width 8, last | 36.571 | 37.194 | 2.5x |
| Width 16, last | 59.685 | 59.141 | 4.0x |
| Width 32, last | 137.794 | 139.116 | 9.3-9.4x |
| Width 32, miss | 117.778 | 117.625 | 8.0x |

First-key lookup remained approximately 14.6-14.9 ns as width grew. Late and
missing keys scaled with the number and content of comparisons. This confirms
that direct internal child-page traversal is the strongest low-risk library
candidate. It should retain the current array representation, case behavior,
insertion order, deletion holes, and ownership rules.

## Inline strings

| Value length | Session A ns/op | Session B ns/op |
|---:|---:|---:|
| 16 bytes | 21.756 | 22.904 |
| 31 bytes | 21.739 | 22.274 |
| 32 bytes | 29.416 | 29.346 |
| 33 bytes | 29.513 | 29.540 |
| 128 bytes | 29.616 | 30.278 |
| 208 bytes | 29.960 | 29.867 |
| 4096 bytes | 88.574 | 88.935 |

Crossing the current inline boundary from 31 to 32 bytes increased lifecycle
cost by 35.3% and 31.8% in the two sessions. Values from 32 through 208 bytes
then cost approximately 30-31 ns because they all take the external allocation
path. Enlarging the inline buffer is therefore a credible candidate, but its
threshold must be selected with 32-bit and 64-bit layout checks and measured as
a paired implementation rather than inferred from this baseline alone.

## Writes and object construction

Replacing the first key cost about 31 ns regardless of object width. Replacing
the last key cost about 58 ns at width 8 and 137-138 ns at width 32, again
showing lookup as the scaling term.

Fresh object construction was also measured with and without libfastjson's
existing `FJSON_OBJECT_ADD_KEY_IS_NEW` flag:

| Width | Normal A ns | Known-new A ns | Speedup A | Normal B ns | Known-new B ns | Speedup B |
|---:|---:|---:|---:|---:|---:|---:|
| 1 | 60.401 | 54.948 | 1.10x | 60.419 | 54.626 | 1.11x |
| 4 | 176.324 | 130.113 | 1.36x | 176.269 | 129.593 | 1.36x |
| 8 | 427.785 | 248.354 | 1.72x | 427.119 | 248.302 | 1.72x |
| 16 | 1047.242 | 488.741 | 2.14x | 1041.454 | 490.719 | 2.12x |
| 32 | 3646.321 | 1327.269 | 2.75x | 3633.927 | 1337.653 | 2.72x |

This is an existing caller-controlled optimization, not a proposed semantic
change. Rsyslog may use it only where absence was already proven while holding
the applicable message/global lock or while constructing a private fresh
object. It is unsafe for ordinary replacement-capable writes.

The 8-to-9-member object lifecycle step was approximately 29% in both complete
sessions, reflecting allocation of the second child page plus another value
and key. The embedded first page remains a deliberate allocation-avoidance
tradeoff and should not be removed based on this characterization.

## Candidate order

1. Implement direct internal page traversal in `_fjson_find_child()` and run
   two paired sessions.
2. Independently test larger inline-string thresholds, including compile-time
   object-size assertions on supported word sizes.
3. Audit rsyslog call sites for already-proven-new insertions that can safely
   use `FJSON_OBJECT_ADD_KEY_IS_NEW`.
4. Consider hybrid indexing only after measuring wider, representative
   objects; do not restore the former hash design by default.

No runtime candidate has been implemented or accepted by these results.
