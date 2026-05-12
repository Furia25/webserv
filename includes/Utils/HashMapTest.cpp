/* ************************************************************************** */
/*                                                                            */
/*   HashMapTest.cpp                                                          */
/*   Edge case tests pour HashMap<Key, Value, HashFunctor>                    */
/*                                                                            */
/* ************************************************************************** */

#include <cassert>
#include <iostream>
#include <string>
#include <stdexcept>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>
#include <map>
#include <unordered_map>
#include "HashMap.hpp"

// ================================================================
//  Framework — sortie Markdown
// ================================================================

static int g_passed = 0;
static int g_failed = 0;
static std::string g_current_section = "";

static void md_section(const char* title)
{
    if (!g_current_section.empty())
        std::cout << std::endl;
    g_current_section = title;
    std::cout << "### " << title << std::endl;
    std::cout << std::endl;
    std::cout << "| Status | Test |" << std::endl;
    std::cout << "|--------|------|" << std::endl;
}

#define TEST(name)  // no-op, nom porté par PASS/FAIL
#define PASS(name)  std::cout << "| ✅ PASS | " << (name) << " |" << std::endl; g_passed++;

#define ASSERT(expr) \
    do { if (!(expr)) { \
        std::cout << "| ❌ FAIL | `" #expr "` (" __FILE__ ":" << __LINE__ << ") |" << std::endl; \
        throw std::runtime_error("assertion failed"); \
    } } while (0)

#define RUN_TEST(fn) \
    do { \
        try { fn(); } \
        catch (const std::exception& e) { \
            std::cout << "| ❌ FAIL | **" #fn "**: " << e.what() << " |" << std::endl; \
            g_failed++; \
        } \
    } while (0)

// ================================================================
//  Types de test
// ================================================================

struct NoDef
{
    int value;
    explicit NoDef(int v) : value(v) {}
    bool operator==(const NoDef& o) const { return value == o.value; }
};

struct CopyCounter
{
    int value;
    static int copies;

    explicit CopyCounter(int v) : value(v) {}
    CopyCounter(const CopyCounter& o) : value(o.value) { copies++; }
    CopyCounter& operator=(const CopyCounter& o) { value = o.value; copies++; return *this; }
    bool operator==(const CopyCounter& o) const { return value == o.value; }
};
int CopyCounter::copies = 0;

template <typename T>
struct BadHash {
    size_t operator()(const T&) const { return 0; }
};

int benchmark_main();

// ================================================================
//  1. Cas limites insert
// ================================================================

void test_insert_empty_string_key()
{
    HashMap<std::string, int> map;
    map.insert("", 42);
    ASSERT(map.contain(""));
    ASSERT(map.at("") == 42);
    ASSERT(map.size() == 1);
    PASS("insert empty string key");
}

void test_insert_zero_key()
{
    HashMap<int, std::string> map;
    map.insert(0, "zero");
    ASSERT(map.contain(0));
    ASSERT(map.at(0) == "zero");
    PASS("insert zero as key");
}

void test_insert_negative_keys()
{
    HashMap<int, int> map;
    for (int i = -50; i < 0; i++)
        map.insert(i, i * -1);
    ASSERT(map.size() == 50);
    for (int i = -50; i < 0; i++)
        ASSERT(map.at(i) == i * -1);
    PASS("insert negative integer keys");
}

void test_insert_same_value_different_keys()
{
    HashMap<std::string, int> map;
    map.insert("a", 1);
    map.insert("b", 1);
    map.insert("c", 1);
    ASSERT(map.size() == 3);
    ASSERT(map.at("a") == 1);
    ASSERT(map.at("b") == 1);
    ASSERT(map.at("c") == 1);
    PASS("insert same value for different keys");
}

void test_insert_overwrite_chain()
{
    HashMap<std::string, int> map;
    for (int i = 0; i < 100; i++)
        map.insert("key", i);
    ASSERT(map.size() == 1);
    ASSERT(map.at("key") == 99);
    PASS("multiple overwrites of same key");
}

// ================================================================
//  2. Cas limites erase
// ================================================================

void test_erase_nonexistent()
{
    HashMap<std::string, int> map;
    ASSERT(map.erase("anything") == false);
    ASSERT(map.size() == 0);
    PASS("erase nonexistent key on empty map");
}

void test_erase_twice()
{
    HashMap<std::string, int> map;
    map.insert("foo", 1);
    ASSERT(map.erase("foo") == true);
    ASSERT(map.erase("foo") == false);
    ASSERT(map.size() == 0);
    PASS("erase same key twice");
}

void test_erase_reinsert_many_times()
{
    HashMap<std::string, int> map;
    for (int i = 0; i < 50; i++)
    {
        map.insert("key", i);
        ASSERT(map.at("key") == i);
        ASSERT(map.size() == 1);
        map.erase("key");
        ASSERT(map.size() == 0);
    }
    PASS("erase and reinsert same key many times");
}

void test_erase_middle_of_chain()
{
    HashMap<int, int, BadHash<int> > map(8, 1.0f, 2.0f);
    map.insert(1, 10);
    map.insert(2, 20);
    map.insert(3, 30);
    map.insert(4, 40);
    map.erase(2);
    ASSERT(map.contain(1) == true);
    ASSERT(map.contain(2) == false);
    ASSERT(map.contain(3) == true);
    ASSERT(map.contain(4) == true);
    ASSERT(map.at(1) == 10);
    ASSERT(map.at(3) == 30);
    ASSERT(map.at(4) == 40);
    PASS("erase middle element of collision chain");
}

void test_erase_then_find_chain()
{
    HashMap<int, int, BadHash<int> > map(8, 1.0f, 2.0f);
    map.insert(1, 100);
    map.insert(2, 200);
    map.insert(3, 300);
    map.erase(1);
    HashMap<int, int, BadHash<int> >::iterator it = map.find(3);
    ASSERT(it != map.end());
    ASSERT(it->second == 300);
    PASS("find still works after erase in chain");
}

// ================================================================
//  3. Collisions extremes
// ================================================================

void test_all_same_bucket()
{
    HashMap<int, int, BadHash<int> > map(16, 1.0f, 2.0f);
    for (int i = 0; i < 15; i++)
        map.insert(i, i * 10);
    ASSERT(map.size() == 15);
    for (int i = 0; i < 15; i++)
        ASSERT(map.at(i) == i * 10);
    PASS("all keys hash to same bucket");
}

void test_all_same_bucket_erase_all()
{
    HashMap<int, int, BadHash<int> > map(16, 1.0f, 2.0f);
    for (int i = 0; i < 10; i++)
        map.insert(i, i);
    for (int i = 0; i < 10; i++)
        ASSERT(map.erase(i) == true);
    ASSERT(map.size() == 0);
    ASSERT(map.empty() == true);
    map.insert(42, 99);
    ASSERT(map.at(42) == 99);
    PASS("erase all from same bucket");
}

void test_tombstone_reuse_in_chain()
{
    HashMap<int, int, BadHash<int> > map(8, 1.0f, 2.0f);
    map.insert(1, 10);
    map.insert(2, 20);
    map.insert(3, 30);
    map.erase(1);
    map.insert(1, 99);
    ASSERT(map.size() == 3);
    ASSERT(map.at(1) == 99);
    ASSERT(map.at(2) == 20);
    ASSERT(map.at(3) == 30);
    PASS("tombstone slot reused in collision chain");
}

// ================================================================
//  4. Rehash edge cases
// ================================================================

void test_rehash_from_capacity_1()
{
    HashMap<int, int> map(1, 0.5f, 2.0f);
    for (int i = 0; i < 20; i++)
        map.insert(i, i);
    ASSERT(map.size() == 20);
    for (int i = 0; i < 20; i++)
        ASSERT(map.at(i) == i);
    PASS("rehash starting from capacity 1");
}

void test_rehash_keeps_tombstones_out()
{
    HashMap<int, int> map(4, 0.9f, 2.0f);
    map.insert(0, 1);
    map.insert(1, 2);
    map.insert(2, 3);
    map.erase(1);
    map.insert(3, 4);
    map.insert(4, 5);
    ASSERT(map.contain(1) == false);
    ASSERT(map.contain(0) == true);
    ASSERT(map.contain(2) == true);
    ASSERT(map.contain(3) == true);
    ASSERT(map.contain(4) == true);
    ASSERT(map.size() == 4);
    PASS("rehash does not copy tombstones");
}

void test_load_factor_never_exceeded()
{
    HashMap<int, int> map(4, 0.7f, 2.0f);
    for (int i = 0; i < 200; i++)
    {
        map.insert(i, i);
        ASSERT(map.load_factor() <= map.max_load_factor());
    }
    PASS("load factor never exceeds max during mass insert");
}

// ================================================================
//  5. Iterateurs edge cases
// ================================================================

void test_iterator_empty_map()
{
    HashMap<std::string, int> map;
    ASSERT(map.begin() == map.end());
    int count = 0;
    for (HashMap<std::string, int>::iterator it = map.begin(); it != map.end(); ++it)
        count++;
    ASSERT(count == 0);
    PASS("iterator on empty map");
}

void test_iterator_single_element()
{
    HashMap<std::string, int> map;
    map.insert("only", 1);
    HashMap<std::string, int>::iterator it = map.begin();
    ASSERT(it != map.end());
    ASSERT(it->second == 1);
    ++it;
    ASSERT(it == map.end());
    PASS("iterator on single element");
}

void test_iterator_all_erased_but_one()
{
    HashMap<int, int> map;
    for (int i = 0; i < 10; i++)
        map.insert(i, i);
    for (int i = 0; i < 9; i++)
        map.erase(i);
    int count = 0;
    for (HashMap<int, int>::iterator it = map.begin(); it != map.end(); ++it)
        count++;
    ASSERT(count == 1);
    ASSERT(map.at(9) == 9);
    PASS("iterator with mostly erased map");
}

void test_iterator_modify_value()
{
    HashMap<std::string, int> map;
    map.insert("a", 1);
    map.insert("b", 2);
    for (HashMap<std::string, int>::iterator it = map.begin(); it != map.end(); ++it)
        it->second *= 10;
    ASSERT(map.at("a") == 10);
    ASSERT(map.at("b") == 20);
    PASS("modify value through iterator");
}

// ================================================================
//  6. Copy / assignation
// ================================================================

void test_copy_constructor()
{
    HashMap<std::string, int> original;
    original.insert("a", 1);
    original.insert("b", 2);
    HashMap<std::string, int> copy(original);
    ASSERT(copy.size() == 2);
    ASSERT(copy.at("a") == 1);
    ASSERT(copy.at("b") == 2);
    original.insert("a", 99);
    ASSERT(copy.at("a") == 1);
    copy.insert("b", 88);
    ASSERT(original.at("b") == 2);
    PASS("copy constructor deep copy");
}

void test_assignment_operator()
{
    HashMap<std::string, int> a;
    a.insert("x", 10);
    HashMap<std::string, int> b;
    b.insert("y", 20);
    b = a;
    ASSERT(b.size() == 1);
    ASSERT(b.contain("x") == true);
    ASSERT(b.contain("y") == false);
    ASSERT(b.at("x") == 10);
    a.insert("x", 99);
    ASSERT(b.at("x") == 10);
    PASS("assignment operator deep copy");
}

void test_self_assignment()
{
    HashMap<std::string, int> map;
    map.insert("foo", 42);
    map = map;
    ASSERT(map.size() == 1);
    ASSERT(map.at("foo") == 42);
    PASS("self assignment is safe");
}

void test_assign_to_larger_map()
{
    HashMap<int, int> large;
    for (int i = 0; i < 50; i++)
        large.insert(i, i);
    HashMap<int, int> small;
    small.insert(1, 99);
    large = small;
    ASSERT(large.size() == 1);
    ASSERT(large.at(1) == 99);
    ASSERT(large.contain(0) == false);
    PASS("assign small map to large map");
}

// ================================================================
//  7. Stress
// ================================================================

void test_stress_insert_erase_alternate()
{
    HashMap<int, int> map;
    for (int i = 0; i < 1000; i++)
    {
        map.insert(i, i);
        if (i % 2 == 0)
            map.erase(i);
    }
    for (int i = 0; i < 1000; i++)
    {
        if (i % 2 == 0)
            ASSERT(map.contain(i) == false);
        else
            ASSERT(map.at(i) == i);
    }
    ASSERT(map.size() == 500);
    PASS("stress: alternate insert/erase 1000 times");
}

void test_stress_same_keys()
{
    HashMap<std::string, int> map;
    for (int i = 0; i < 500; i++)
        map.insert("persistent", i);
    ASSERT(map.size() == 1);
    ASSERT(map.at("persistent") == 499);
    PASS("stress: 500 overwrites then verify");
}

void test_stress_iterator_count()
{
    HashMap<int, int> map;
    for (int i = 0; i < 100; i++)
        map.insert(i, i);
    for (int i = 0; i < 100; i += 3)
        map.erase(i);
    size_t size_count = map.size();
    size_t iter_count = 0;
    for (HashMap<int, int>::iterator it = map.begin(); it != map.end(); ++it)
        iter_count++;
    ASSERT(size_count == iter_count);
    PASS("stress: iterator count matches size after mixed ops");
}

// ================================================================
//  8. Tombstone cleanup
// ================================================================

void test_tombstone_cleanup_correctness()
{
    HashMap<int, int> map(16, 0.9f, 2.0f);
    for (int i = 0; i < 12; i++)
        map.insert(i, i * 10);
    map.erase(0); map.erase(1); map.erase(2); map.erase(3);
    map.insert(99, 999);
    ASSERT(map.contain(0)  == false);
    ASSERT(map.contain(1)  == false);
    ASSERT(map.contain(2)  == false);
    ASSERT(map.contain(3)  == false);
    ASSERT(map.contain(99) == true);
    ASSERT(map.at(99) == 999);
    for (int i = 4; i < 12; i++)
        ASSERT(map.at(i) == i * 10);
    ASSERT(map.size() == 9);
    PASS("tombstone cleanup preserves all live entries");
}

void test_tombstone_cleanup_find_after_heavy_erase()
{
    HashMap<int, int> map(32, 0.9f, 2.0f);
    for (int i = 0; i < 20; i++)
        map.insert(i, i);
    for (int i = 0; i < 15; i++)
        map.erase(i);
    map.insert(100, 100);
    for (int i = 15; i < 20; i++)
    {
        ASSERT(map.contain(i) == true);
        ASSERT(map.at(i) == i);
    }
    for (int i = 0; i < 15; i++)
        ASSERT(map.contain(i) == false);
    PASS("find correct after tombstone cleanup");
}

void test_tombstone_cleanup_size_stable()
{
    HashMap<int, int> map(16, 0.9f, 2.0f);
    for (int cycle = 0; cycle < 5; cycle++)
    {
        for (int i = 0; i < 10; i++)
            map.insert(i, i);
        ASSERT(map.size() == 10);
        for (int i = 0; i < 6; i++)
            map.erase(i);
        ASSERT(map.size() == 4);
        map.insert(50 + cycle, cycle);
        ASSERT(map.size() == 5);
        for (int i = 6; i < 10; i++)
            map.erase(i);
        map.erase(50 + cycle);
        ASSERT(map.size() == 0);
    }
    PASS("size stays coherent through multiple cleanup cycles");
}

void test_tombstone_counter_coherence()
{
    HashMap<int, int> map(64, 0.7f, 2.0f);
    for (int i = 0; i < 30; i++)
        map.insert(i, i);
    for (int i = 0; i < 10; i++)
        map.erase(i);
    ASSERT(map.size() == 20);
    for (int i = 0; i < 10; i++)
        map.insert(i, i * 100);
    ASSERT(map.size() == 30);
    for (int i = 0; i < 10; i++)
        ASSERT(map.at(i) == i * 100);
    for (int i = 10; i < 30; i++)
        ASSERT(map.at(i) == i);
    PASS("tombstone counter stays coherent");
}

// ================================================================
//  Main tests
// ================================================================

int main()
{
    std::cout << "# HashMap Test Results" << std::endl << std::endl;

    md_section("Insert edge cases");
    RUN_TEST(test_insert_empty_string_key);
    RUN_TEST(test_insert_zero_key);
    RUN_TEST(test_insert_negative_keys);
    RUN_TEST(test_insert_same_value_different_keys);
    RUN_TEST(test_insert_overwrite_chain);

    md_section("Erase edge cases");
    RUN_TEST(test_erase_nonexistent);
    RUN_TEST(test_erase_twice);
    RUN_TEST(test_erase_reinsert_many_times);
    RUN_TEST(test_erase_middle_of_chain);
    RUN_TEST(test_erase_then_find_chain);

    md_section("Collisions extremes");
    RUN_TEST(test_all_same_bucket);
    RUN_TEST(test_all_same_bucket_erase_all);
    RUN_TEST(test_tombstone_reuse_in_chain);

    md_section("Rehash edge cases");
    RUN_TEST(test_rehash_from_capacity_1);
    RUN_TEST(test_rehash_keeps_tombstones_out);
    RUN_TEST(test_load_factor_never_exceeded);

    md_section("Iterator edge cases");
    RUN_TEST(test_iterator_empty_map);
    RUN_TEST(test_iterator_single_element);
    RUN_TEST(test_iterator_all_erased_but_one);
    RUN_TEST(test_iterator_modify_value);

    md_section("Copy / Assignation");
    RUN_TEST(test_copy_constructor);
    RUN_TEST(test_assignment_operator);
    RUN_TEST(test_self_assignment);
    RUN_TEST(test_assign_to_larger_map);

    md_section("Stress");
    RUN_TEST(test_stress_insert_erase_alternate);
    RUN_TEST(test_stress_same_keys);
    RUN_TEST(test_stress_iterator_count);

    md_section("Tombstone cleanup");
    RUN_TEST(test_tombstone_cleanup_correctness);
    RUN_TEST(test_tombstone_cleanup_find_after_heavy_erase);
    RUN_TEST(test_tombstone_cleanup_size_stable);
    RUN_TEST(test_tombstone_counter_coherence);

    std::cout << std::endl;
    std::cout << "---" << std::endl << std::endl;
    std::cout << "**" << g_passed << " passed, " << g_failed << " failed**"
              << std::endl << std::endl;
    std::cout << "==============================" << std::endl;

    benchmark_main();
    return g_failed > 0 ? 1 : 0;
}

// ================================================================
//  Benchmark infrastructure — multi-runs + statistiques
// ================================================================

static std::string make_key(int i)
{
    char buf[32];
    std::sprintf(buf, "key_%08d", i);
    return std::string(buf);
}

static double clock_ms()
{
    return static_cast<double>(std::clock()) / CLOCKS_PER_SEC * 1000.0;
}

struct Stats
{
    double median;
    double mean;
    double stdev;
    double cv;      // coefficient de variation = stdev/mean (0..1)
    double p10;
    double p90;
    size_t runs;
};

static Stats compute_stats(std::vector<double>& samples)
{
    std::sort(samples.begin(), samples.end());
    size_t n = samples.size();

    Stats s;
    s.runs   = n;
    s.median = (n % 2 == 0)
        ? (samples[n/2 - 1] + samples[n/2]) / 2.0
        : samples[n/2];
    s.p10 = samples[static_cast<size_t>(n * 0.10)];
    s.p90 = samples[static_cast<size_t>(n * 0.90)];

    double sum = 0.0;
    for (size_t i = 0; i < n; i++) sum += samples[i];
    s.mean = sum / n;

    double var = 0.0;
    for (size_t i = 0; i < n; i++)
        var += (samples[i] - s.mean) * (samples[i] - s.mean);
    s.stdev = std::sqrt(var / n);
    s.cv    = (s.mean > 0.0) ? s.stdev / s.mean : 0.0;

    return s;
}

// ================================================================
//  run_bench : 2 warmups non comptés + BENCH_RUNS mesures
// ================================================================
#define BENCH_RUNS 12

template <typename Fn>
static Stats run_bench(Fn body_fn)
{
    // Warmup : chauffe le cache, pas compté
    body_fn();
    body_fn();

    std::vector<double> samples;
    samples.reserve(BENCH_RUNS);
    for (int r = 0; r < BENCH_RUNS; r++)
        samples.push_back(body_fn());
    return compute_stats(samples);
}

// ================================================================
//  Calcul de l'intervalle de confiance sur un ratio
//
//  ratio_point = median_other / median_self
//
//  Propagation d'erreur simplifiée (premier ordre) :
//    σ_ratio/ratio ≈ sqrt(cv_self² + cv_other²)
//
//  On utilise p10/p90 plutôt que ±σ pour l'affichage :
//    ratio_lo = p10_other / p90_self   (pire cas HashMap lent, other rapide)
//    ratio_hi = p90_other / p10_self   (meilleur cas HashMap rapide, other lent)
// ================================================================
struct RatioCI
{
    double point;   // ratio médian
    double lo;      // borne basse p10/p90
    double hi;      // borne haute p90/p10
    double combined_cv; // incertitude combinée en %
};

static RatioCI compute_ratio(const Stats& self, const Stats& other)
{
    RatioCI r;
    r.point       = (self.median > 0.0) ? other.median / self.median : 0.0;
    r.lo          = (self.p90    > 0.0) ? other.p10    / self.p90    : 0.0;
    r.hi          = (self.p10    > 0.0) ? other.p90    / self.p10    : 0.0;
    double cv_comb = std::sqrt(self.cv * self.cv + other.cv * other.cv);
    r.combined_cv = cv_comb * 100.0;
    return r;
}

// Formate le ratio avec son intervalle.
// Si l'incertitude combinée est faible (< 8%) : affiche juste "2.51x"
// Sinon : affiche "~1.88x [1.3–2.7]" pour signaler l'imprécision
static void format_ratio(char* buf, size_t bufsize, const RatioCI& r)
{
    const char* verdict = (r.point >= 1.0) ? "🟢" : "🔴";
    if (r.combined_cv < 8.0)
        std::snprintf(buf, bufsize, "%s %4.2fx", verdict, r.point);
    else
        std::snprintf(buf, bufsize, "%s ~%4.2fx  [%4.2f–%4.2f]",
            verdict, r.point, r.lo, r.hi);
}

// ================================================================
//  Affichage
// ================================================================

static bool g_bench_table_open = false;

static void bench_md_section(const char* title)
{
    if (g_bench_table_open) std::cout << std::endl;
    g_bench_table_open = false;
    std::cout << "### " << title << std::endl << std::endl;
}

// --- Tableau solo : médiane, p10–p90, ops/sec ---
static void open_simple_table()
{
    if (g_bench_table_open) return;
    std::cout << "| Operation | Median | p10–p90 | Ops/sec |" << std::endl;
    std::cout << "|-----------|-------:|--------:|--------:|" << std::endl;
    g_bench_table_open = true;
}

static void print_stat_row(const char* label, const Stats& s, size_t ops)
{
    open_simple_table();
    double ops_sec = ops / (s.median / 1000.0);
    char range[32];
    std::snprintf(range, sizeof(range), "[%.2f–%.2f]", s.p10, s.p90);
    std::printf("| %-38s | %6.2f ms | %-16s | %.0f |\n",
        label, s.median, range, ops_sec);
}

// --- Tableau comparatif ---
// Colonnes : HashMap med | other med | Ratio (avec IC si bruité)
static void open_compare_table(const char* other_name)
{
    if (g_bench_table_open) std::cout << std::endl;
    char header[256];
    std::snprintf(header, sizeof(header),
        "| Operation | HashMap | %s | Ratio |", other_name);
    std::cout << header << std::endl;
    std::cout << "|-----------|--------:|-------:|------:|" << std::endl;
    g_bench_table_open = true;
}

static void print_compare_row(const char* label,
    const Stats& sh, const Stats& so, size_t /*ops*/)
{
    RatioCI r = compute_ratio(sh, so);
    char ratio_str[64];
    format_ratio(ratio_str, sizeof(ratio_str), r);

    // Affiche p10–p90 entre parenthèses pour chaque mesure
    char hrange[32], orange[32];
    std::snprintf(hrange, sizeof(hrange), "%.2f [%.2f–%.2f]",
        sh.median, sh.p10, sh.p90);
    std::snprintf(orange, sizeof(orange), "%.2f [%.2f–%.2f]",
        so.median, so.p10, so.p90);

    std::printf("| %-42s | %-22s | %-22s | %s |\n",
        label, hrange, orange, ratio_str);
}

// ================================================================
//  Benchmarks HashMap seul
// ================================================================

static void bench_hashmap_solo(size_t n)
{
    char title[64];
    std::sprintf(title, "HashMap seul (%zu ops, %d runs)", n, BENCH_RUNS);
    bench_md_section(title);

    // insert int
    {
        Stats s = run_bench([&]() -> double {
            HashMap<int, int> map(n);
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                map.insert(static_cast<int>(i), static_cast<int>(i));
            return clock_ms() - t0;
        });
        print_stat_row("insert int", s, n);
    }

    // find int — all hits
    {
        HashMap<int, int> map(n);
        for (size_t i = 0; i < n; i++)
            map.insert(static_cast<int>(i), static_cast<int>(i));

        Stats s = run_bench([&]() -> double {
            volatile int sink = 0;
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                sink += map.at(static_cast<int>(i));
            (void)sink;
            return clock_ms() - t0;
        });
        print_stat_row("find int (all hits)", s, n);
    }

    // find int — all misses
    {
        HashMap<int, int> map(n);
        for (size_t i = 0; i < n; i++)
            map.insert(static_cast<int>(i), static_cast<int>(i));

        Stats s = run_bench([&]() -> double {
            double t0 = clock_ms();
            for (size_t i = n; i < n * 2; i++)
                map.contain(static_cast<int>(i));
            return clock_ms() - t0;
        });
        print_stat_row("find int (all misses)", s, n);
    }

    // erase int (reconstruit à chaque run)
    {
        Stats s = run_bench([&]() -> double {
            HashMap<int, int> map(n);
            for (size_t i = 0; i < n; i++)
                map.insert(static_cast<int>(i), static_cast<int>(i));
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                map.erase(static_cast<int>(i));
            return clock_ms() - t0;
        });
        print_stat_row("erase int", s, n);
    }

    // insert string
    {
        Stats s = run_bench([&]() -> double {
            HashMap<std::string, int> map(n);
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                map.insert(make_key(static_cast<int>(i)), static_cast<int>(i));
            return clock_ms() - t0;
        });
        print_stat_row("insert string", s, n);
    }

    // find string — hits
    {
        HashMap<std::string, int> map(n);
        for (size_t i = 0; i < n; i++)
            map.insert(make_key(static_cast<int>(i)), static_cast<int>(i));

        Stats s = run_bench([&]() -> double {
            volatile int sink = 0;
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                sink += map.at(make_key(static_cast<int>(i)));
            (void)sink;
            return clock_ms() - t0;
        });
        print_stat_row("find string (all hits)", s, n);
    }

    // iterate
    {
        HashMap<int, int> map(n);
        for (size_t i = 0; i < n; i++)
            map.insert(static_cast<int>(i), static_cast<int>(i));

        Stats s = run_bench([&]() -> double {
            volatile int sink = 0;
            double t0 = clock_ms();
            for (HashMap<int,int>::iterator it = map.begin(); it != map.end(); ++it)
                sink += it->second;
            (void)sink;
            return clock_ms() - t0;
        });
        print_stat_row("iterate all", s, n);
    }

    // insert+erase alternating
    {
        Stats s = run_bench([&]() -> double {
            HashMap<int, int> map;
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
            {
                map.insert(static_cast<int>(i), static_cast<int>(i));
                if (i % 2 == 0)
                    map.erase(static_cast<int>(i));
            }
            return clock_ms() - t0;
        });
        print_stat_row("insert+erase alternating", s, n);
    }

    // rehash pressure
    {
        Stats s = run_bench([&]() -> double {
            HashMap<int, int> map(1, 0.7f, 2.0f);
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                map.insert(static_cast<int>(i), static_cast<int>(i));
            return clock_ms() - t0;
        });
        print_stat_row("insert with max rehash pressure", s, n);
    }
    std::cout << std::endl;
}

// ================================================================
//  Scalabilité
// ================================================================

static void bench_scalability()
{
    bench_md_section("Scalabilite insert int (median sur 10 runs)");
    std::cout << "| N | Median | p10-p90 |" << std::endl;
    std::cout << "|---|-------:|--------:|" << std::endl;

    size_t sizes[] = { 10000, 100000, 1000000 };
    for (size_t si = 0; si < 3; si++)
    {
        size_t n = sizes[si];
        Stats s = run_bench([&]() -> double {
            HashMap<int, int> map(n);
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                map.insert(static_cast<int>(i), static_cast<int>(i));
            return clock_ms() - t0;
        });
        std::printf("| %7zu | %6.2f ms | [%.2f-%.2f] |\n",
            n, s.median, s.p10, s.p90);
    }
    std::cout << std::endl;
}

// ================================================================
//  Comparaison vs std::map
// ================================================================

static void bench_vs_stdmap(size_t n)
{
    char title[80];
    std::sprintf(title, "HashMap vs std::map (%zu ops, %d runs)", n, BENCH_RUNS);
    bench_md_section(title);
    open_compare_table("std::map");

    // insert int
    {
        Stats sh = run_bench([&]() -> double {
            HashMap<int, int> hmap(n);
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                hmap.insert(static_cast<int>(i), static_cast<int>(i));
            return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            std::map<int, int> smap;
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                smap[static_cast<int>(i)] = static_cast<int>(i);
            return clock_ms() - t0;
        });
        print_compare_row("insert int", sh, so, n);
    }

    // find int
    {
        HashMap<int, int> hmap(n);
        std::map<int, int> smap;
        for (size_t i = 0; i < n; i++)
        {
            hmap.insert(static_cast<int>(i), static_cast<int>(i));
            smap[static_cast<int>(i)] = static_cast<int>(i);
        }
        Stats sh = run_bench([&]() -> double {
            volatile int sink = 0;
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                sink += hmap.at(static_cast<int>(i));
            (void)sink; return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            volatile int sink = 0;
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                sink += smap.at(static_cast<int>(i));
            (void)sink; return clock_ms() - t0;
        });
        print_compare_row("find int", sh, so, n);
    }

    // insert string
    {
        Stats sh = run_bench([&]() -> double {
            HashMap<std::string, int> hmap(n);
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                hmap.insert(make_key(static_cast<int>(i)), static_cast<int>(i));
            return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            std::map<std::string, int> smap;
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                smap[make_key(static_cast<int>(i))] = static_cast<int>(i);
            return clock_ms() - t0;
        });
        print_compare_row("insert string", sh, so, n);
    }

    // iterate
    {
        HashMap<int, int> hmap(n);
        std::map<int, int> smap;
        for (size_t i = 0; i < n; i++)
        {
            hmap.insert(static_cast<int>(i), static_cast<int>(i));
            smap[static_cast<int>(i)] = static_cast<int>(i);
        }
        Stats sh = run_bench([&]() -> double {
            volatile int sink = 0;
            double t0 = clock_ms();
            for (HashMap<int,int>::iterator it = hmap.begin(); it != hmap.end(); ++it)
                sink += it->second;
            (void)sink; return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            volatile int sink = 0;
            double t0 = clock_ms();
            for (std::map<int,int>::iterator it = smap.begin(); it != smap.end(); ++it)
                sink += it->second;
            (void)sink; return clock_ms() - t0;
        });
        print_compare_row("iterate all", sh, so, n);
    }
    std::cout << std::endl;
}

// ================================================================
//  Comparaison vs unordered_map
// ================================================================

static void bench_vs_unordered(size_t n)
{
    char title[80];
    std::sprintf(title, "HashMap vs unordered_map (%zu ops, %d runs)", n, BENCH_RUNS);
    bench_md_section(title);
    open_compare_table("unordered_map");

    // insert int
    {
        Stats sh = run_bench([&]() -> double {
            HashMap<int, int> hmap(n);
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                hmap.insert(static_cast<int>(i), static_cast<int>(i));
            return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            std::unordered_map<int, int> umap; umap.reserve(n);
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                umap[static_cast<int>(i)] = static_cast<int>(i);
            return clock_ms() - t0;
        });
        print_compare_row("insert int", sh, so, n);
    }

    // find int — hits
    {
        HashMap<int, int> hmap(n);
        std::unordered_map<int, int> umap; umap.reserve(n);
        for (size_t i = 0; i < n; i++)
        {
            hmap.insert(static_cast<int>(i), static_cast<int>(i));
            umap[static_cast<int>(i)] = static_cast<int>(i);
        }
        Stats sh = run_bench([&]() -> double {
            volatile int sink = 0;
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                sink += hmap.at(static_cast<int>(i));
            (void)sink; return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            volatile int sink = 0;
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                sink += umap.at(static_cast<int>(i));
            (void)sink; return clock_ms() - t0;
        });
        print_compare_row("find int (hits)", sh, so, n);
    }

    // find int — misses
    {
        HashMap<int, int> hmap(n);
        std::unordered_map<int, int> umap; umap.reserve(n);
        for (size_t i = 0; i < n; i++)
        {
            hmap.insert(static_cast<int>(i), static_cast<int>(i));
            umap[static_cast<int>(i)] = static_cast<int>(i);
        }
        Stats sh = run_bench([&]() -> double {
            double t0 = clock_ms();
            for (size_t i = n; i < n*2; i++)
                hmap.contain(static_cast<int>(i));
            return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            double t0 = clock_ms();
            for (size_t i = n; i < n*2; i++)
                umap.find(static_cast<int>(i));
            return clock_ms() - t0;
        });
        print_compare_row("find int (misses)", sh, so, n);
    }

    // erase int
    {
        Stats sh = run_bench([&]() -> double {
            HashMap<int, int> hmap(n);
            for (size_t i = 0; i < n; i++)
                hmap.insert(static_cast<int>(i), static_cast<int>(i));
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                hmap.erase(static_cast<int>(i));
            return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            std::unordered_map<int, int> umap; umap.reserve(n);
            for (size_t i = 0; i < n; i++)
                umap[static_cast<int>(i)] = static_cast<int>(i);
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                umap.erase(static_cast<int>(i));
            return clock_ms() - t0;
        });
        print_compare_row("erase int", sh, so, n);
    }

    // insert string
    {
        Stats sh = run_bench([&]() -> double {
            HashMap<std::string, int> hmap(n);
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                hmap.insert(make_key(static_cast<int>(i)), static_cast<int>(i));
            return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            std::unordered_map<std::string, int> umap; umap.reserve(n);
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
                umap[make_key(static_cast<int>(i))] = static_cast<int>(i);
            return clock_ms() - t0;
        });
        print_compare_row("insert string", sh, so, n);
    }

    // iterate
    {
        HashMap<int, int> hmap(n);
        std::unordered_map<int, int> umap; umap.reserve(n);
        for (size_t i = 0; i < n; i++)
        {
            hmap.insert(static_cast<int>(i), static_cast<int>(i));
            umap[static_cast<int>(i)] = static_cast<int>(i);
        }
        Stats sh = run_bench([&]() -> double {
            volatile int sink = 0;
            double t0 = clock_ms();
            for (HashMap<int,int>::iterator it = hmap.begin(); it != hmap.end(); ++it)
                sink += it->second;
            (void)sink; return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            volatile int sink = 0;
            double t0 = clock_ms();
            for (std::unordered_map<int,int>::iterator it = umap.begin(); it != umap.end(); ++it)
                sink += it->second;
            (void)sink; return clock_ms() - t0;
        });
        print_compare_row("iterate all", sh, so, n);
    }
    std::cout << std::endl;
}

// ================================================================
//  Impact tombstone cleanup
// ================================================================

static void bench_tombstone(size_t n)
{
    char title[128];
    std::sprintf(title, "Impact tombstone cleanup (%zu ops, %d runs) — insert N, erase 50%%", n, BENCH_RUNS);
    bench_md_section(title);
    open_compare_table("unordered_map");

    // find miss après 50% erase
    {
        Stats sh = run_bench([&]() -> double {
            HashMap<int, int> hmap(n);
            for (size_t i = 0; i < n; i++)
                hmap.insert(static_cast<int>(i), static_cast<int>(i));
            for (size_t i = 0; i < n; i += 2)
                hmap.erase(static_cast<int>(i));
            double t0 = clock_ms();
            for (size_t i = n; i < n*2; i++)
                hmap.contain(static_cast<int>(i));
            return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            std::unordered_map<int, int> umap; umap.reserve(n);
            for (size_t i = 0; i < n; i++)
                umap[static_cast<int>(i)] = static_cast<int>(i);
            for (size_t i = 0; i < n; i += 2)
                umap.erase(static_cast<int>(i));
            double t0 = clock_ms();
            for (size_t i = n; i < n*2; i++)
                umap.find(static_cast<int>(i));
            return clock_ms() - t0;
        });
        print_compare_row("find miss after 50% erase", sh, so, n);
    }

    // find hit après 50% erase
    {
        Stats sh = run_bench([&]() -> double {
            HashMap<int, int> hmap(n);
            for (size_t i = 0; i < n; i++)
                hmap.insert(static_cast<int>(i), static_cast<int>(i));
            for (size_t i = 0; i < n; i += 2)
                hmap.erase(static_cast<int>(i));
            volatile int sink = 0;
            double t0 = clock_ms();
            for (size_t i = 1; i < n; i += 2)
                sink += hmap.at(static_cast<int>(i));
            (void)sink; return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            std::unordered_map<int, int> umap; umap.reserve(n);
            for (size_t i = 0; i < n; i++)
                umap[static_cast<int>(i)] = static_cast<int>(i);
            for (size_t i = 0; i < n; i += 2)
                umap.erase(static_cast<int>(i));
            volatile int sink = 0;
            double t0 = clock_ms();
            for (size_t i = 1; i < n; i += 2)
                sink += umap.at(static_cast<int>(i));
            (void)sink; return clock_ms() - t0;
        });
        print_compare_row("find hit after 50% erase", sh, so, n);
    }

    // iterate après 50% erase
    {
        Stats sh = run_bench([&]() -> double {
            HashMap<int, int> hmap(n);
            for (size_t i = 0; i < n; i++)
                hmap.insert(static_cast<int>(i), static_cast<int>(i));
            for (size_t i = 0; i < n; i += 2)
                hmap.erase(static_cast<int>(i));
            volatile int sink = 0;
            double t0 = clock_ms();
            for (HashMap<int,int>::iterator it = hmap.begin(); it != hmap.end(); ++it)
                sink += it->second;
            (void)sink; return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            std::unordered_map<int, int> umap; umap.reserve(n);
            for (size_t i = 0; i < n; i++)
                umap[static_cast<int>(i)] = static_cast<int>(i);
            for (size_t i = 0; i < n; i += 2)
                umap.erase(static_cast<int>(i));
            volatile int sink = 0;
            double t0 = clock_ms();
            for (std::unordered_map<int,int>::iterator it = umap.begin(); it != umap.end(); ++it)
                sink += it->second;
            (void)sink; return clock_ms() - t0;
        });
        print_compare_row("iterate after 50% erase", sh, so, n);
    }

    // insert 1000 après 50% erase
    {
        Stats sh = run_bench([&]() -> double {
            HashMap<int, int> hmap(n);
            for (size_t i = 0; i < n; i++)
                hmap.insert(static_cast<int>(i), static_cast<int>(i));
            for (size_t i = 0; i < n; i += 2)
                hmap.erase(static_cast<int>(i));
            double t0 = clock_ms();
            for (size_t i = n; i < n + 1000; i++)
                hmap.insert(static_cast<int>(i), static_cast<int>(i));
            return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            std::unordered_map<int, int> umap; umap.reserve(n);
            for (size_t i = 0; i < n; i++)
                umap[static_cast<int>(i)] = static_cast<int>(i);
            for (size_t i = 0; i < n; i += 2)
                umap.erase(static_cast<int>(i));
            double t0 = clock_ms();
            for (size_t i = n; i < n + 1000; i++)
                umap[static_cast<int>(i)] = static_cast<int>(i);
            return clock_ms() - t0;
        });
        print_compare_row("insert 1000 after 50% erase (cleanup cost)", sh, so, n);
    }
    std::cout << std::endl;
}

// ================================================================
//  Patterns naturels
// ================================================================

static void bench_patterns(size_t n)
{
    char title[80];
    std::sprintf(title, "Patterns naturels (%zu ops, %d runs)", n, BENCH_RUNS);
    bench_md_section(title);
    open_compare_table("unordered_map");

    // sliding window
    {
        const size_t WINDOW = 1000;
        Stats sh = run_bench([&]() -> double {
            HashMap<int, int> hmap;
            volatile int sink = 0;
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
            {
                hmap.insert(static_cast<int>(i), static_cast<int>(i));
                if (i >= WINDOW)
                    hmap.erase(static_cast<int>(i - WINDOW));
                sink += hmap.at(static_cast<int>(i));
            }
            (void)sink; return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            std::unordered_map<int, int> umap;
            volatile int sink = 0;
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
            {
                umap[static_cast<int>(i)] = static_cast<int>(i);
                if (i >= WINDOW)
                    umap.erase(static_cast<int>(i - WINDOW));
                sink += umap.at(static_cast<int>(i));
            }
            (void)sink; return clock_ms() - t0;
        });
        print_compare_row("sliding window (insert+erase+find, W=1000)", sh, so, n);
    }

    // hot set 80/20
    {
        const size_t HOT  = n / 5;
        const size_t COLD = n - HOT;
        Stats sh = run_bench([&]() -> double {
            HashMap<int, int> hmap;
            for (size_t i = 0; i < n; i++)
                hmap.insert(static_cast<int>(i), static_cast<int>(i));
            volatile int sink = 0;
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
            {
                if (i % 10 < 8)
                    sink += hmap.at(static_cast<int>(i % HOT));
                else if (i % 10 == 8)
                {
                    int k = static_cast<int>(HOT + (i % COLD));
                    hmap.erase(k); hmap.insert(k, k * 2);
                }
                else
                    sink += hmap.contain(static_cast<int>(HOT + (i % COLD)));
            }
            (void)sink; return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            std::unordered_map<int, int> umap;
            for (size_t i = 0; i < n; i++)
                umap[static_cast<int>(i)] = static_cast<int>(i);
            volatile int sink = 0;
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
            {
                if (i % 10 < 8)
                    sink += umap.at(static_cast<int>(i % HOT));
                else if (i % 10 == 8)
                {
                    int k = static_cast<int>(HOT + (i % COLD));
                    umap.erase(k); umap[k] = k * 2;
                }
                else
                    sink += umap.count(static_cast<int>(HOT + (i % COLD)));
            }
            (void)sink; return clock_ms() - t0;
        });
        print_compare_row("hot set 80/20 (find+erase+insert mixed)", sh, so, n);
    }

    // churn
    {
        const size_t KEYS = 500;
        Stats sh = run_bench([&]() -> double {
            HashMap<int, int> hmap;
            volatile int sink = 0;
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
            {
                int k = static_cast<int>(i % KEYS);
                if (hmap.contain(k)) { sink += hmap.at(k); hmap.erase(k); }
                else hmap.insert(k, static_cast<int>(i));
            }
            (void)sink; return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            std::unordered_map<int, int> umap;
            volatile int sink = 0;
            double t0 = clock_ms();
            for (size_t i = 0; i < n; i++)
            {
                int k = static_cast<int>(i % KEYS);
                std::unordered_map<int,int>::iterator it = umap.find(k);
                if (it != umap.end()) { sink += it->second; umap.erase(it); }
                else umap[k] = static_cast<int>(i);
            }
            (void)sink; return clock_ms() - t0;
        });
        print_compare_row("churn (insert/erase same keys, pool=500)", sh, so, n);
    }

    // batch update
    {
        const size_t BATCH = n / 10;
        Stats sh = run_bench([&]() -> double {
            HashMap<int, int> hmap;
            for (size_t i = 0; i < n; i++)
                hmap.insert(static_cast<int>(i), static_cast<int>(i));
            volatile int sink = 0;
            double t0 = clock_ms();
            for (int wave = 0; wave < 5; wave++)
            {
                size_t base = static_cast<size_t>(wave) * BATCH;
                for (size_t i = 0; i < BATCH; i++)
                {
                    hmap.erase(static_cast<int>(base + i));
                    hmap.insert(static_cast<int>(base + i), wave * 1000);
                }
                for (HashMap<int,int>::iterator it = hmap.begin(); it != hmap.end(); ++it)
                    sink += it->second;
            }
            (void)sink; return clock_ms() - t0;
        });
        Stats so = run_bench([&]() -> double {
            std::unordered_map<int, int> umap;
            for (size_t i = 0; i < n; i++)
                umap[static_cast<int>(i)] = static_cast<int>(i);
            volatile int sink = 0;
            double t0 = clock_ms();
            for (int wave = 0; wave < 5; wave++)
            {
                size_t base = static_cast<size_t>(wave) * BATCH;
                for (size_t i = 0; i < BATCH; i++)
                {
                    umap.erase(static_cast<int>(base + i));
                    umap[static_cast<int>(base + i)] = wave * 1000;
                }
                for (std::unordered_map<int,int>::iterator it = umap.begin(); it != umap.end(); ++it)
                    sink += it->second;
            }
            (void)sink; return clock_ms() - t0;
        });
        print_compare_row("batch update 5 waves 10% (erase+insert+iter)", sh, so, n);
    }
    std::cout << std::endl;
}

// ================================================================
//  Entry point benchmarks
// ================================================================

// Trick pour stringifier BENCH_RUNS dans le titre
#define STRINGIFY(x) #x
#define BENCH_RUNS_STR STRINGIFY(BENCH_RUNS)

int benchmark_main()
{
    std::cout << "---" << std::endl << std::endl;
    std::cout << "## Benchmarks (" << BENCH_RUNS << " runs par mesure)" << std::endl << std::endl;

    bench_hashmap_solo(100000);
    bench_scalability();
    bench_vs_stdmap(10000);
    bench_vs_stdmap(100000);
    bench_vs_unordered(10000);
    bench_vs_unordered(100000);
    bench_tombstone(100000);
    bench_patterns(100000);

    return 0;
}
