# HashMap Test Results

### Insert edge cases

| Status | Test |
|--------|------|
| ✅ PASS | insert empty string key |
| ✅ PASS | insert zero as key |
| ✅ PASS | insert negative integer keys |
| ✅ PASS | insert same value for different keys |
| ✅ PASS | multiple overwrites of same key |

### Erase edge cases

| Status | Test |
|--------|------|
| ✅ PASS | erase nonexistent key on empty map |
| ✅ PASS | erase same key twice |
| ✅ PASS | erase and reinsert same key many times |
| ✅ PASS | erase middle element of collision chain |
| ✅ PASS | find still works after erase in chain |

### Collisions extremes

| Status | Test |
|--------|------|
| ✅ PASS | all keys hash to same bucket |
| ✅ PASS | erase all from same bucket |
| ✅ PASS | tombstone slot reused in collision chain |

### Rehash edge cases

| Status | Test |
|--------|------|
| ✅ PASS | rehash starting from capacity 1 |
| ✅ PASS | rehash does not copy tombstones |
| ❌ FAIL | `map.load_factor() <= map.max_load_factor()` (HashMapTest.cpp:296) |
| ❌ FAIL | **test_load_factor_never_exceeded**: assertion failed |

### Iterator edge cases

| Status | Test |
|--------|------|
| ✅ PASS | iterator on empty map |
| ✅ PASS | iterator on single element |
| ✅ PASS | iterator with mostly erased map |
| ✅ PASS | modify value through iterator |

### Copy / Assignation

| Status | Test |
|--------|------|
| ✅ PASS | copy constructor deep copy |
| ✅ PASS | assignment operator deep copy |
| ✅ PASS | self assignment is safe |
| ✅ PASS | assign small map to large map |

### Stress

| Status | Test |
|--------|------|
| ✅ PASS | stress: alternate insert/erase 1000 times |
| ✅ PASS | stress: 500 overwrites then verify |
| ✅ PASS | stress: iterator count matches size after mixed ops |

### Tombstone cleanup

| Status | Test |
|--------|------|
| ✅ PASS | tombstone cleanup preserves all live entries |
| ✅ PASS | find correct after tombstone cleanup |
| ✅ PASS | size stays coherent through multiple cleanup cycles |
| ✅ PASS | tombstone counter stays coherent |

---

**30 passed, 1 failed**

==============================
---

## Benchmarks (12 runs par mesure)

### HashMap seul (100000 ops, 12 runs)

| Operation | Median | p10–p90 | Ops/sec |
|-----------|-------:|--------:|--------:|
| insert int                             |   6.85 ms | [6.74–8.62]    | 14604936 |
| find int (all hits)                    |   4.06 ms | [2.77–4.75]    | 24615385 |
| find int (all misses)                  |   3.67 ms | [3.56–3.99]    | 27270248 |
| erase int                              |   7.03 ms | [6.62–8.48]    | 14227787 |
| insert string                          |  35.65 ms | [34.67–42.02]  | 2804892 |
| find string (all hits)                 |  26.28 ms | [25.46–27.63]  | 3805827 |
| iterate all                            |   4.19 ms | [3.80–5.11]    | 23886301 |
| insert+erase alternating               |   7.43 ms | [7.38–7.55]    | 13455328 |
| insert with max rehash pressure        |   8.06 ms | [7.95–8.39]    | 12412338 |


### Scalabilite insert int (median sur 10 runs)

| N | Median | p10-p90 |
|---|-------:|--------:|
|   10000 |   0.31 ms | [0.31-0.32] |
|  100000 |   6.64 ms | [6.59-6.82] |
| 1000000 | 111.36 ms | [97.08-130.94] |

### HashMap vs std::map (10000 ops, 12 runs)

| Operation | HashMap | std::map | Ratio |
|-----------|--------:|-------:|------:|
| insert int                                 | 0.33 [0.31–0.38]     | 5.14 [5.02–5.29]     | 🟢 ~15.57x  [13.13–17.23] |
| find int                                   | 0.20 [0.20–0.23]     | 2.68 [2.60–2.73]     | 🟢 13.61x |
| insert string                              | 1.81 [1.74–1.87]     | 7.43 [7.37–7.58]     | 🟢 ~4.11x  [3.93–4.35] |
| iterate all                                | 0.22 [0.21–0.23]     | 0.19 [0.19–0.20]     | 🔴 0.90x |


### HashMap vs std::map (100000 ops, 12 runs)

| Operation | HashMap | std::map | Ratio |
|-----------|--------:|-------:|------:|
| insert int                                 | 7.12 [6.75–7.59]     | 59.27 [59.04–59.61]  | 🟢 ~8.33x  [7.78–8.84] |
| find int                                   | 3.59 [2.83–6.33]     | 30.73 [30.58–30.96]  | 🟢 ~8.55x  [4.83–10.93] |
| insert string                              | 33.13 [31.02–34.51]  | 88.44 [87.60–88.72]  | 🟢 ~2.67x  [2.54–2.86] |
| iterate all                                | 3.69 [3.66–3.75]     | 2.62 [2.56–2.79]     | 🔴 0.71x |


### HashMap vs unordered_map (10000 ops, 12 runs)

| Operation | HashMap | unordered_map | Ratio |
|-----------|--------:|-------:|------:|
| insert int                                 | 0.31 [0.31–0.33]     | 1.81 [1.79–1.83]     | 🟢 5.84x |
| find int (hits)                            | 0.20 [0.20–0.21]     | 0.66 [0.65–0.67]     | 🟢 3.35x |
| find int (misses)                          | 0.36 [0.35–0.37]     | 0.80 [0.79–0.82]     | 🟢 2.25x |
| erase int                                  | 0.22 [0.22–0.23]     | 1.03 [1.01–1.15]     | 🟢 4.63x |
| insert string                              | 1.77 [1.74–1.89]     | 2.81 [2.79–2.94]     | 🟢 1.59x |
| iterate all                                | 0.25 [0.25–0.26]     | 0.19 [0.19–0.21]     | 🔴 ~0.79x  [0.75–0.84] |


### HashMap vs unordered_map (100000 ops, 12 runs)

| Operation | HashMap | unordered_map | Ratio |
|-----------|--------:|-------:|------:|
| insert int                                 | 6.59 [6.49–6.96]     | 18.48 [18.28–18.69]  | 🟢 2.81x |
| find int (hits)                            | 2.95 [2.37–5.63]     | 6.66 [6.61–6.82]     | 🟢 ~2.26x  [1.18–2.88] |
| find int (misses)                          | 3.62 [3.59–3.97]     | 7.89 [7.88–7.94]     | 🟢 2.18x |
| erase int                                  | 5.02 [4.04–5.85]     | 10.34 [10.25–10.46]  | 🟢 ~2.06x  [1.75–2.59] |
| insert string                              | 32.62 [32.13–33.53]  | 33.72 [31.53–34.11]  | 🟢 1.03x |
| iterate all                                | 3.72 [3.68–3.94]     | 2.03 [2.01–2.11]     | 🔴 0.55x |


### Impact tombstone cleanup (100000 ops, 12 runs) — insert N, erase 50%

| Operation | HashMap | unordered_map | Ratio |
|-----------|--------:|-------:|------:|
| find miss after 50% erase                  | 3.60 [3.54–3.85]     | 5.86 [5.82–6.00]     | 🟢 1.63x |
| find hit after 50% erase                   | 1.40 [1.28–2.77]     | 3.35 [3.32–3.42]     | 🟢 ~2.39x  [1.20–2.68] |
| iterate after 50% erase                    | 2.78 [2.73–2.86]     | 1.07 [1.04–1.18]     | 🔴 0.38x |
| insert 1000 after 50% erase (cleanup cost) | 0.04 [0.04–0.04]     | 0.18 [0.18–0.18]     | 🟢 4.42x |


### Patterns naturels (100000 ops, 12 runs)

| Operation | HashMap | unordered_map | Ratio |
|-----------|--------:|-------:|------:|
| sliding window (insert+erase+find, W=1000) | 8.10 [8.03–8.39]     | 34.46 [33.91–35.51]  | 🟢 4.25x |
| hot set 80/20 (find+erase+insert mixed)    | 4.75 [4.16–5.46]     | 10.13 [10.05–10.34]  | 🟢 ~2.13x  [1.84–2.48] |
| churn (insert/erase same keys, pool=500)   | 7.44 [7.40–7.48]     | 18.95 [18.88–19.12]  | 🟢 2.55x |
| batch update 5 waves 10% (erase+insert+iter) | 23.61 [23.44–23.97]  | 25.87 [25.54–26.18]  | 🟢 1.10x |

