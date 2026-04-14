#include <iostream>
#include <sstream>
#include <fstream>
#include "TOMLDocument.hpp"
#include "TOMLTokenizer.hpp"
#include "TOMLDebugJson.hpp"

// ============================================================
//  TOML Test Suite — comprehensive edge-case coverage
//  Build: g++ -std=c++17 -o toml_test toml_test.cpp
// ============================================================

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <functional>
#include <iomanip>


// ────────────────────────────────────────────────────────────

// ============================================================
//  Lexer helper
// ============================================================
static void printTokens(std::istream& ss)
{
    toml::Tokenizer tokenizer(ss);
    bool first = true;
    while (1)
    {
        toml::Token tok = tokenizer.next_token();
        if (tok.getType() == toml::Token::END_OF_FILE) {std::cout << '\n'; return ;}
        if (tok.getType() == toml::Token::NEW_LINE) { std::cout << "↵ "; continue; }
        if (!first) std::cout << " ";
        first = false;

        std::cout << tok.toString();
        switch (tok.getType())
        {
            case toml::Token::STRING:
            case toml::Token::MULTI_STRING:
            case toml::Token::MULTI_LITERAL_STRING:
            case toml::Token::LITERAL_STRING:
            case toml::Token::IDENT:
                std::cout << ":" << std::quoted(tok.getLiteral());
                break;
            default: break;
        }
    }
    std::cout << "\n";
}

// ============================================================
//  Test infrastructure
// ============================================================
struct TestResult
{
    std::string label;
    bool        passed;
    std::string detail;   // shown only on failure
};

static std::vector<TestResult> g_results;

enum class Expect { SUCCESS, FAILURE };

static void runTest(
    const std::string&              label,
    const std::string&              toml_src,
    Expect                          expect,
    // optional validator: receives the parsed doc, returns "" on ok or an error msg
    std::function<std::string(const toml::Document&)> validate = nullptr,
    bool                            show_tokens = false)
{
    TestResult r{ label, false, "" };

    if (show_tokens)
    {
        std::cout << "  TOKENS: ";
        std::istringstream ts(toml_src);
        printTokens(ts);
    }

    try
    {
        toml::Document doc;
        std::istringstream ss(toml_src);
        doc.from_stream(ss);

        if (expect == Expect::FAILURE)
        {
            r.passed = false;
            r.detail = "Expected a parse error but parsing succeeded.\n"
                       "  JSON output: " + toml::toJson(doc);
        }
        else
        {
            if (validate)
            {
                std::string err = validate(doc);
                if (!err.empty())
                {
                    r.passed = false;
                    r.detail = "Validation failed: " + err + "\n"
                               "  JSON: " + toml::toJson(doc);
                }
                else r.passed = true;
            }
            else r.passed = true;
        }
    }
    catch (const std::exception& e)
    {
        if (expect == Expect::FAILURE)
        {
            r.passed = true;   // error was expected
        }
        else
        {
            r.passed = false;
            r.detail = std::string("Unexpected parse error: ") + e.what() + "\n"
                       "  Source:\n" + toml_src;
        }
    }

    g_results.push_back(r);
}

static void printSummary()
{
    int pass = 0, fail = 0;
    const int W = 58;

    std::cout << "\n";
    std::cout << std::string(W + 12, '=') << "\n";
    std::cout << "  RESULTS\n";
    std::cout << std::string(W + 12, '=') << "\n";

    for (auto& r : g_results)
    {
        std::string status = r.passed ? "  PASS  " : "  FAIL  ";
        std::cout << status << r.label << "\n";
        if (!r.passed && !r.detail.empty())
        {
            // indent detail lines
            std::istringstream ds(r.detail);
            std::string line;
            while (std::getline(ds, line))
                std::cout << "          > " << line << "\n";
        }
        r.passed ? ++pass : ++fail;
    }

    std::cout << std::string(W + 12, '-') << "\n";
    std::cout << "  " << pass << " passed, " << fail << " failed"
              << "  (" << g_results.size() << " total)\n";
    std::cout << std::string(W + 12, '=') << "\n\n";
}

// ============================================================
//  Convenience helpers for building validators
// ============================================================
// These assume toml::Document exposes a get<T>(key) style accessor.
// Adjust the accessor calls to match your actual API.

// ============================================================
//  Test definitions
// ============================================================
static void registerTests()
{
    // ── 1. BASIC KEY / VALUE TYPES ─────────────────────────

    runTest("Integer: decimal",
        "val = 42",
        Expect::SUCCESS);

    runTest("Integer: positive sign",
        "val = +17",
        Expect::SUCCESS);

    runTest("Integer: negative",
        "val = -100",
        Expect::SUCCESS);

    runTest("Integer: underscore separator",
        "val = 1_000_000",
        Expect::SUCCESS);

    runTest("Integer: hex prefix",
        "val = 0xFF",
        Expect::SUCCESS);

    runTest("Integer: octal prefix",
        "val = 0o77",
        Expect::SUCCESS);

    runTest("Integer: binary prefix",
        "val = 0b1010",
        Expect::SUCCESS);

    runTest("Integer: leading underscore (invalid)",
        "val = _42",
        Expect::FAILURE);

    runTest("Integer: trailing underscore (invalid)",
        "val = 42_",
        Expect::FAILURE);

    runTest("Integer: double underscore (invalid)",
        "val = 1__000",
        Expect::FAILURE);

    runTest("Float: basic",
        "val = 3.14",
        Expect::SUCCESS);

    runTest("Float: with exponent",
        "val = 6.626e-34",
        Expect::SUCCESS);

    runTest("Float: +inf",
        "val = +inf",
        Expect::SUCCESS);

    runTest("Float: -inf",
        "val = -inf",
        Expect::SUCCESS);

    runTest("Float: nan",
        "val = nan",
        Expect::SUCCESS);

    runTest("Float: no digit before dot (invalid)",
        "val = .5",
        Expect::FAILURE);

    runTest("Float: no digit after dot (invalid)",
        "val = 5.",
        Expect::FAILURE);

    runTest("Boolean: true",
        "val = true",
        Expect::SUCCESS);

    runTest("Boolean: false",
        "val = false",
        Expect::SUCCESS);

    runTest("Boolean: capitalised (invalid)",
        "val = True",
        Expect::FAILURE);

    // ── 2. STRINGS ─────────────────────────────────────────

    runTest("Basic string: simple",
        R"(val = "hello world")",
        Expect::SUCCESS);

    runTest("Basic string: escape sequences",
        R"(val = "tab:\there\nnewline")",
        Expect::SUCCESS);

    runTest("Basic string: unicode escape \\uXXXX",
        R"(val = "\u00e9")",
        Expect::SUCCESS);

    runTest("Basic string: unicode escape \\UXXXXXXXX",
        R"(val = "\U0001F600")",
        Expect::SUCCESS);

    runTest("Basic string: invalid escape",
        R"(val = "\q")",
        Expect::FAILURE);

    runTest("Basic string: unterminated (invalid)",
        R"(val = "oops)",
        Expect::FAILURE);

    runTest("Literal string: no escapes",
        R"(val = 'C:\Users\tom')",
        Expect::SUCCESS);

    runTest("Literal string: backslash stays literal",
        R"(val = 'backslash: \')",
        Expect::SUCCESS);

    runTest("Multi-line basic string: simple",
        "val = \"\"\"\nhello\nworld\n\"\"\"",
        Expect::SUCCESS);

    runTest("Multi-line basic string: escape newline (line continuation)",
        "val = \"\"\"\nThe quick \\\n    brown fox\"\"\"",
        Expect::SUCCESS);

    runTest("Multi-line basic string: first newline stripped",
        "val = \"\"\"\nfoo\"\"\"",
        Expect::SUCCESS);

    runTest("Multi-line basic string: embedded quotes (up to 2)",
        "val = \"\"\"She said \"hi\".\"\"\"",
        Expect::SUCCESS);

    runTest("Multi-line literal string: simple",
        "val = '''\nhello\n'''",
        Expect::SUCCESS);

    runTest("Multi-line literal string: no escape processing",
        "val = '''\n\\n is not a newline\n'''",
        Expect::SUCCESS);

    runTest("String: empty basic",
        R"(val = "")",
        Expect::SUCCESS);

    runTest("String: empty literal",
        R"(val = '')",
        Expect::SUCCESS);

    // ── 3. KEYS ────────────────────────────────────────────

    runTest("Bare key: alphanumeric and dashes",
        "some-key_1 = true",
        Expect::SUCCESS);

    runTest("Quoted key: basic string",
        R"("dotted.key" = 1)",
        Expect::SUCCESS);

    runTest("Quoted key: literal string",
        R"('127.0.0.1' = "ip")",
        Expect::SUCCESS);

    runTest("Dotted key: simple",
        "a.b.c = 1",
        Expect::SUCCESS);

    runTest("Dotted key: whitespace around dots",
        "a . b . c = 1",
        Expect::SUCCESS);

    runTest("Dotted key: implicit table creation",
        "fruit.name = \"apple\"\nfruit.color = \"red\"",
        Expect::SUCCESS);

    runTest("Key: empty bare key (invalid)",
        "= 1",
        Expect::FAILURE);

    runTest("Key: redefine existing key (invalid)",
        "key = 1\nkey = 2",
        Expect::FAILURE);

    // ── 4. ARRAYS ──────────────────────────────────────────

    runTest("Array: integers",
        "arr = [1, 2, 3]",
        Expect::SUCCESS);

    runTest("Array: strings",
        R"(arr = ["a", "b", "c"])",
        Expect::SUCCESS);

    runTest("Array: nested",
        "arr = [[1, 2], [3, 4]]",
        Expect::SUCCESS);

    runTest("Array: trailing comma",
        "arr = [1, 2, 3,]",
        Expect::SUCCESS);

    runTest("Array: multiline",
        "arr = [\n  1,\n  2,\n  3\n]",
        Expect::SUCCESS);

    runTest("Array: empty",
        "arr = []",
        Expect::SUCCESS);

    runTest("Array of tables inline",
        "arr = [{a=1}, {a=2}]",
        Expect::SUCCESS);

    // ── 5. INLINE TABLES ───────────────────────────────────

    runTest("Inline table: simple",
        R"(point = {x = 1, y = 2})",
        Expect::SUCCESS);

    runTest("Inline table: nested",
        R"(a = {b = {c = 1}})",
        Expect::SUCCESS);

    runTest("Inline table: trailing comma",
        R"(t = {a = 1,})",
        Expect::SUCCESS);

    runTest("Inline table: multiline",
        "t = {\n  a = 1\n}",
        Expect::SUCCESS);

    runTest("Inline table: empty",
        "t = {}",
        Expect::SUCCESS);

    // ── 6. STANDARD TABLES ─────────────────────────────────

    runTest("Table: basic header",
        "[table]\nkey = 1",
        Expect::SUCCESS);

    runTest("Table: multiple",
        "[a]\nkey = 1\n[b]\nkey = 2",
        Expect::SUCCESS);

    runTest("Table: dotted header",
        "[a.b.c]\nval = true",
        Expect::SUCCESS);

    runTest("Table: redefine existing table (invalid)",
        "[t]\na = 1\n[t]\nb = 2",
        Expect::FAILURE);

    runTest("Table: super-table after sub-table",
        "[a.b]\nval = 1\n[a]\nother = 2",
        Expect::SUCCESS);

    runTest("Table: key clash with table (invalid)",
        "[a]\nb = 1\n[a.b]\nc = 2",
        Expect::FAILURE);

    // ── 7. ARRAY OF TABLES ─────────────────────────────────

    runTest("Array of tables: basic",
        "[[products]]\nname = \"hammer\"\n[[products]]\nname = \"nail\"",
        Expect::SUCCESS);

    runTest("Array of tables: with subtable",
        "[[fruits]]\nname = \"apple\"\n[fruits.physical]\ncolor = \"red\"",
        Expect::SUCCESS);

    runTest("Array of tables: redefine as plain table (invalid)",
        "[[arr]]\nval = 1\n[arr]\nval = 2",
        Expect::FAILURE);

    runTest("Array of tables: statically defined array clash (invalid)",
        "arr = [1,2]\n[[arr]]\nval = 3",
        Expect::FAILURE);

    // ── 8. COMMENTS ────────────────────────────────────────

    runTest("Comment: end of line",
        "key = 1  # this is a comment",
        Expect::SUCCESS);

    runTest("Comment: full line",
        "# entire line comment\nkey = 1",
        Expect::SUCCESS);

    runTest("Comment: hash inside string is NOT a comment",
        R"(key = "hello # world")",
        Expect::SUCCESS);

    // ── 9. WHITESPACE / NEWLINES ───────────────────────────

    runTest("Whitespace: CRLF line endings",
        "key = 1\r\nother = 2",
        Expect::SUCCESS);

    runTest("Whitespace: blank lines between key-vals",
        "a = 1\n\n\nb = 2",
        Expect::SUCCESS);

    runTest("Whitespace: spaces around equals",
        "key   =   42",
        Expect::SUCCESS);

    // ── 10. EDGE / ERROR CASES ─────────────────────────────

    runTest("Empty document",
        "",
        Expect::SUCCESS);

    runTest("Only comments and blank lines",
        "# comment\n\n# another\n",
        Expect::SUCCESS);

    runTest("Key without value (invalid)",
        "key",
        Expect::FAILURE);

    runTest("Value without key (invalid)",
        "= 1",
        Expect::FAILURE);

    runTest("Bare key with space (invalid)",
        "key one = 1",
        Expect::FAILURE);

    runTest("Unclosed table header (invalid)",
        "[table",
        Expect::FAILURE);

    runTest("Unclosed array (invalid)",
        "arr = [1, 2",
        Expect::FAILURE);

    runTest("Unclosed inline table (invalid)",
        "t = {a = 1",
        Expect::FAILURE);

    runTest("Integer overflow (beyond int64 — implementation-defined)",
        "val = 99999999999999999999",
        Expect::FAILURE);   // adjust to SUCCESS if your lib wraps silently

    runTest("Float: double decimal point (invalid)",
        "val = 1.2.3",
        Expect::FAILURE);

    runTest("Bare key with dot treated as dotted (not single key)",
        "a.b = 1\n[a]\nc = 2",
        Expect::SUCCESS);   // a.b and [a].c coexist

    runTest("Redefine implicit table via dotted key (invalid)",
        "a.b = 1\n[a]\nb = 2",
        Expect::FAILURE);   // b already defined via dotted key

    runTest("Deep nesting: table inside array of tables",
        "[[a]]\n[[a.b]]\nval = 1\n[[a]]\n[[a.b]]\nval = 2",
        Expect::SUCCESS);

    runTest("Multi-line basic string: three quotes inside (invalid)",
        "val = \"\"\"foo \"\"\" bar\"\"\"",
        // The first \"\"\" closes the string; 'bar' is stray.
        Expect::FAILURE);

    runTest("Unicode bare key characters (invalid)",
        u8"héllo = 1",
        Expect::FAILURE);   // bare keys are restricted to A-Z a-z 0-9 - _

    runTest("Literal string: single quotes inside (invalid — no escaping)",
        "val = 'it\\'s'",
        Expect::FAILURE);

    runTest("Large table: many keys in one table",
        [](){
            std::string s = "[big]\n";
            for (int i = 0; i < 100; ++i)
                s += "key" + std::to_string(i) + " = " + std::to_string(i) + "\n";
            return s;
        }(),
        Expect::SUCCESS);

    runTest("Deeply dotted key: 5 levels",
        "a.b.c.d.e = 42",
        Expect::SUCCESS);

    runTest("Mixed array of arrays (same inner type)",
        "arr = [[1,2],[3,4]]",
        Expect::SUCCESS);

    runTest("Inline table in array",
        "arr = [{x=1, y=2}, {x=3, y=4}]",
        Expect::SUCCESS);

    runTest("Key: only dashes",
        "--- = true",
        Expect::SUCCESS);

    runTest("Key: only underscores",
        "___ = true",
        Expect::SUCCESS);

    runTest("Quoted dotted key segments",
        R"("a.b"."c.d" = 1)",
        Expect::SUCCESS);

    runTest("String: all standard escape sequences",
        R"(val = "\b\t\n\f\r\"\\"")",
        Expect::SUCCESS);

    runTest("Float: underscore in mantissa",
        "val = 9_224_617.445_991_228_313",
        Expect::SUCCESS);

    runTest("Integer: zero",
        "val = 0",
        Expect::SUCCESS);

    runTest("Integer: negative zero",
        "val = -0",
        Expect::SUCCESS);

    runTest("Array: booleans",
        "arr = [true, false, true]",
        Expect::SUCCESS);

    runTest("Array: floats",
        "arr = [1.0, 2.5, -3.14]",
        Expect::SUCCESS);

    runTest("Duplicate array-of-table key across parent tables (invalid)",
        "[[a]]\n[[a.b]]\nval = 1\n[a]\nother = 2",
        Expect::FAILURE);

    runTest("Newline inside basic string (invalid)",
        "val = \"line1\nline2\"",
        Expect::FAILURE);

    runTest("Newline inside literal string (invalid)",
        "val = 'line1\nline2'",
        Expect::FAILURE);
}

// ============================================================
//  main
// ============================================================
int main(int argc, char** argv)
{
    // ── optional: run against a file from the command line
    if (argc > 1)
    {
        std::cout << "=== File: " << argv[1] << " ===\n";
        try
        {
            std::fstream fs(argv[1]);
            std::cout << "Tokens: ";
            printTokens(fs);

            toml::Document doc;
            doc.from_file(argv[1]);
            std::cout << "JSON:\n" << toml::toJson(doc) << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << "ERROR: " << e.what() << "\n";
        }
        return 0;
    }

    // ── run test suite
    registerTests();
    printSummary();

    // exit code: 0 = all passed, 1 = some failed
    for (auto& r : g_results)
        if (!r.passed) return 1;
    return 0;
}