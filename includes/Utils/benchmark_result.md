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
| ❌ FAIL | `map.load_factor() <= map.max_load_factor()` (HashMapTest.cpp:363) |
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

## Benchmarks

### HashMap seul (100 000 ops)

| Operation | Time | Ops/sec |
|-----------|-----:|--------:|
| insert int | 10.83 ms | 9231905 |
| find int (all hits) | 3.22 ms | 31075202 |
| find int (all misses) | 3.58 ms | 27964206 |
| erase int | 8.72 ms | 11465260 |
| insert string | 44.08 ms | 2268757 |
| find string (all hits) | 22.86 ms | 4374645 |
| iterate all | 3.67 ms | 27262814 |
| insert+erase alternating | 5.09 ms | 19634793 |
| insert with max rehash pressure | 8.66 ms | 11544678 |


### Scalabilite insert int

| N | Temps |
|---|------:|
| 10000 | 0.30 ms |
| 100000 | 7.52 ms |
| 1000000 | 98.50 ms |

### HashMap vs std::map (10000 ops)

| Operation | HashMap | std::map | Ratio |
|-----------|--------:|---------:|------:|
| insert int | 0.30 ms | 5.32 ms | 🟢 17.49x |
| find int | 0.20 ms | 2.57 ms | 🟢 12.61x |
| insert string | 2.13 ms | 7.66 ms | 🟢 3.60x |
| iterate all | 0.21 ms | 0.34 ms | 🟢 1.60x |


### HashMap vs std::map (100000 ops)

| Operation | HashMap | std::map | Ratio |
|-----------|--------:|---------:|------:|
| insert int | 7.82 ms | 59.03 ms | 🟢 7.55x |
| find int | 6.49 ms | 31.18 ms | 🟢 4.81x |
| insert string | 34.85 ms | 89.54 ms | 🟢 2.57x |
| iterate all | 3.79 ms | 3.16 ms | 🔴 0.84x |


### HashMap vs unordered_map (10000 ops)

  insert int                     HashMap:    0.33 ms   unordered_map:    1.75 ms   ratio: 5.39x
  find int (hits)                HashMap:    0.20 ms   unordered_map:    0.65 ms   ratio: 3.29x
  find int (misses)              HashMap:    0.36 ms   unordered_map:    0.81 ms   ratio: 2.22x
  erase int                      HashMap:    0.23 ms   unordered_map:    1.01 ms   ratio: 4.49x
  insert string                  HashMap:    2.04 ms   unordered_map:    3.40 ms   ratio: 1.67x
  iterate all                    HashMap:    0.21 ms   unordered_map:    0.21 ms   ratio: 0.98x

### HashMap vs unordered_map (100000 ops)

  insert int                     HashMap:    8.53 ms   unordered_map:   18.66 ms   ratio: 2.19x
  find int (hits)                HashMap:    7.91 ms   unordered_map:    6.76 ms   ratio: 0.85x
  find int (misses)              HashMap:    3.77 ms   unordered_map:    8.55 ms   ratio: 2.27x
  erase int                      HashMap:    6.98 ms   unordered_map:   10.35 ms   ratio: 1.48x
  insert string                  HashMap:   45.49 ms   unordered_map:   35.82 ms   ratio: 0.79x
  iterate all                    HashMap:    3.72 ms   unordered_map:    2.30 ms   ratio: 0.62x

### Impact tombstone cleanup (100000 ops) — insert N, erase 50%

| Operation | HashMap | unordered_map | Ratio |
|-----------|--------:|--------------:|------:|
| find miss after 50% erase | 3.91 ms | 5.80 ms | 🟢 1.48x |
| find hit after 50% erase | 5.38 ms | 3.40 ms | 🔴 0.63x |
| iterate after 50% erase | 3.34 ms | 1.12 ms | 🔴 0.34x |
| insert 1000 after 50% erase (cleanup cost) | 0.04 ms | 0.18 ms | 🟢 4.86x |


### Patterns naturels (100000 ops)

| Operation | HashMap | unordered_map | Ratio |
|-----------|--------:|--------------:|------:|
| sliding window (insert+erase+find, W=1000) | 8.05 ms | 36.14 ms | 🟢 4.49x |
| hot set 80/20 (find+erase+insert mixed) | 15.99 ms | 39.64 ms | 🟢 2.48x |
| churn (insert/erase same keys, pool=500) | 7.62 ms | 19.63 ms | 🟢 2.58x |
| batch update 5 waves 10% (erase+insert+iter) | 36.76 ms | 55.04 ms | 🟢 1.50x |

