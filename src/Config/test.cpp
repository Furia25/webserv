// ============================================================
//  TOML 1.1.0 Extended Test Suite — additional edge cases,
//  error paths, and complex structural validations.
//  No date/datetime tests included.
//  Build: g++ -std=c++17 -o toml_test_ext toml_test_extended.cpp
// ============================================================

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <functional>
#include <iomanip>

#include "toml.hpp"
#include "TOMLTokenizer.hpp"
#include "TOMLDebugJson.hpp"

// ============================================================
//  Lexer helper
// ============================================================
static void printTokens(std::istream& ss)
{
    toml::TOMLErrorManager  error_manager;
    toml::Tokenizer tokenizer(ss, error_manager);
    bool first = true;
    while (1)
    {
        toml::Token tok = tokenizer.next_token();
        if (tok.getType() == toml::Token::END_OF_FILE) { std::cout << '\n'; return; }
        if (tok.getType() == toml::Token::NEW_LINE)    { std::cout << "↵ "; continue; }
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
struct TestResult { std::string label; bool passed; std::string detail; };
static std::vector<TestResult> g_results;
enum class Expect { SUCCESS, FAILURE };

static void runTest(
    const std::string& label,
    const std::string& toml_src,
    Expect             expect,
    std::function<std::string(const toml::Document&)> validate = nullptr,
    bool               show_tokens = false)
{
    TestResult r{ label, false, "" };

    if (show_tokens)
    {
        std::cout << "  TOKENS: ";
        std::istringstream ts(toml_src);
        printTokens(ts);
    }
    toml::Document doc;
    try
    {
        std::istringstream ss(toml_src);
        doc.from_stream(ss);

        if (expect == Expect::FAILURE)
        {
            r.passed = false;
            r.detail = "Expected a parse error but parsing succeeded.\n"
                       "  JSON: " + toml::toJson(doc);
        }
        else
        {
            if (validate)
            {
                std::string err = validate(doc);
                r.passed = err.empty();
                if (!r.passed)
                    r.detail = "Validation failed: " + err + "\n  JSON: " + toml::toJson(doc);
            }
            else r.passed = true;
        }
    }
    catch (const std::exception& e)
    {
        if (expect == Expect::FAILURE)
        {
            r.passed = true;
            r.detail = std::string("\n Error got:") + e.what() + "\n";
        }
        else
        {
            r.passed = false;
            r.detail = std::string("Unexpected parse error: ") + e.what()
                       + "\n  Source:\n" + toml_src + "\n  JSON: " + toml::toJson(doc);
        }
    }

    g_results.push_back(r);
}

static void printSummary()
{
    int pass = 0, fail = 0;
    const int W = 64;
    std::cout << "\n" << std::string(W, '=') << "\n  RESULTS\n" << std::string(W, '=') << "\n";
    for (auto& r : g_results)
    {
        std::cout << (r.passed ? "  PASS  " : "  FAIL  ") << r.label << "\n";
        if (!r.passed && !r.detail.empty())
        {
            std::istringstream ds(r.detail);
            std::string line;
            while (std::getline(ds, line))
                std::cout << "          > " << line << "\n";
        }
        r.passed ? ++pass : ++fail;
    }
    std::cout << std::string(W, '-') << "\n"
              << "  " << pass << " passed, " << fail << " failed"
              << "  (" << g_results.size() << " total)\n"
              << std::string(W, '=') << "\n\n";
}

// ============================================================
//  Tests
// ============================================================
static void registerTests()
{
    // ══════════════════════════════════════════════════════
    //  A. INTEGERS — extra numeric forms
    // ══════════════════════════════════════════════════════

    runTest("Integer: +0 is valid",
        "val = +0",
        Expect::SUCCESS);

    runTest("Integer: hex uppercase letters",
        "val = 0xDEADBEEF",
        Expect::SUCCESS);

    runTest("Integer: hex mixed case",
        "val = 0xDeAdBeEf",
        Expect::SUCCESS);

    runTest("Integer: hex with underscores",
        "val = 0xDEAD_BEEF",
        Expect::SUCCESS);

    runTest("Integer: octal zero",
        "val = 0o0",
        Expect::SUCCESS);

    runTest("Integer: binary all ones (8-bit)",
        "val = 0b11111111",
        Expect::SUCCESS);

    runTest("Integer: octal digit 8 is invalid",
        "val = 0o8",
        Expect::FAILURE);

    runTest("Integer: binary digit 2 is invalid",
        "val = 0b2",
        Expect::FAILURE);

    runTest("Integer: hex prefix alone is invalid",
        "val = 0x",
        Expect::FAILURE);

    runTest("Integer: octal prefix alone is invalid",
        "val = 0o",
        Expect::FAILURE);

    runTest("Integer: binary prefix alone is invalid",
        "val = 0b",
        Expect::FAILURE);

    runTest("Integer: underscore in hex",
        "val = 0xFF_FF",
        Expect::SUCCESS);

    runTest("Integer: leading underscore in hex (invalid)",
        "val = 0x_FF",
        Expect::FAILURE);

    runTest("Integer: trailing underscore in hex (invalid)",
        "val = 0xFF_",
        Expect::FAILURE);

    runTest("Integer: multiple leading zeros (invalid)",
        "val = 007",
        Expect::FAILURE);

    runTest("Integer: solo sign is invalid",
        "val = +",
        Expect::FAILURE);

    runTest("Integer: double sign is invalid",
        "val = --1",
        Expect::FAILURE);

    // ══════════════════════════════════════════════════════
    //  B. FLOATS — extra cases
    // ══════════════════════════════════════════════════════

    runTest("Float: positive zero",
        "val = +0.0",
        Expect::SUCCESS);

    runTest("Float: negative zero",
        "val = -0.0",
        Expect::SUCCESS);

    runTest("Float: exponent with capital E",
        "val = 1E10",
        Expect::SUCCESS);

    runTest("Float: positive exponent",
        "val = 1e+10",
        Expect::SUCCESS);

    runTest("Float: negative exponent",
        "val = 1e-10",
        Expect::SUCCESS);

    runTest("Float: zero-dot-zero",
        "val = 0.0",
        Expect::SUCCESS);

    runTest("Float: underscore in integer part",
        "val = 1_000.5",
        Expect::SUCCESS);

    runTest("Float: underscore in fractional part",
        "val = 1.000_500",
        Expect::SUCCESS);

    runTest("Float: only exponent, no mantissa (invalid)",
        "val = e10",
        Expect::FAILURE);

    runTest("Float: leading dot (invalid)",
        "val = .5",
        Expect::FAILURE);

    runTest("Float: trailing dot (invalid)",
        "val = 5.",
        Expect::FAILURE);

    runTest("Float: +nan",
        "val = +nan",
        Expect::SUCCESS);

    runTest("Float: -nan",
        "val = -nan",
        Expect::SUCCESS);

    runTest("Float: inf without sign",
        "val = inf",
        Expect::SUCCESS);

    runTest("Float: NaN capitalised is invalid",
        "val = NaN",
        Expect::FAILURE);

    runTest("Float: Inf capitalised is invalid",
        "val = Inf",
        Expect::FAILURE);

    // ══════════════════════════════════════════════════════
    //  C. STRINGS — corner cases
    // ══════════════════════════════════════════════════════

    runTest("Basic string: null byte is invalid (control char)",
        "val = \"\x00\"",
        Expect::FAILURE);

    runTest("Basic string: DEL character (0x7F) is invalid",
        "val = \"\x7f\"",
        Expect::FAILURE);

    runTest("Basic string: lone backslash is invalid",
        "val = \"\\\"",
        Expect::FAILURE);

    runTest("Basic string: \\a is invalid escape",
        R"(val = "\a")",
        Expect::FAILURE);

    runTest("Basic string: \\v is invalid escape",
        R"(val = "\v")",
        Expect::FAILURE);

    runTest("Basic string: \\0 is invalid escape (TOML has no null escape)",
        R"(val = "\0")",
        Expect::FAILURE);

    runTest("Basic string: short unicode \\uXXX (only 3 hex digits, invalid)",
        R"(val = "\u00e")",
        Expect::FAILURE);

    runTest("Basic string: \\UXXXXXXX (only 7 digits, invalid)",
        R"(val = "\U0001F60")",
        Expect::FAILURE);

    runTest("Basic string: \\u of surrogate half (invalid codepoint)",
        R"(val = "\uD800")",
        Expect::FAILURE);

    runTest("Basic string: valid emoji via \\U",
        R"(val = "\U0001F4A9")",
        Expect::SUCCESS);

    runTest("Basic string: \\u0000 is invalid (null codepoint)",
        R"(val = "\u0000")",
        Expect::FAILURE);

    runTest("Literal string: cannot contain single-quote (no escaping)",
        "val = 'it''s'",
        Expect::FAILURE);

    runTest("Multi-line basic string: escaped whitespace trimming",
        "val = \"\"\"\n  hello  \\\n   \n   world\"\"\"",
        Expect::SUCCESS);

    runTest("Multi-line basic string: two quotes mid-string are allowed",
        "val = \"\"\"a\"\"b\"\"\"",
        Expect::SUCCESS);

    runTest("Multi-line basic string: six consecutive quotes is invalid",
        "val = \"\"\"\"\"\"\"\"\"\"",
        Expect::FAILURE);

    runTest("Multi-line literal string: two single-quotes mid-string are allowed",
        "val = '''a''b'''",
        Expect::SUCCESS);

    runTest("String: tab character inside basic string is allowed",
        "val = \"hello\tworld\"",
        Expect::SUCCESS);

    runTest("Multi-line basic string: first-line newline stripped, second kept",
        "val = \"\"\"\n\nfoo\"\"\"",
        Expect::SUCCESS);

    // ══════════════════════════════════════════════════════
    //  D. KEYS — corner cases
    // ══════════════════════════════════════════════════════

    runTest("Key: digit-only bare key",
        "1234 = true",
        Expect::SUCCESS);

    runTest("Key: single-char bare key",
        "x = 1",
        Expect::SUCCESS);

    runTest("Key: empty quoted key",
        R"("" = 1)",
        Expect::SUCCESS);

    runTest("Key: empty literal key",
        "'' = 1",
        Expect::SUCCESS);

    runTest("Key: quoted key with whitespace",
        R"("hello world" = 1)",
        Expect::SUCCESS);

    runTest("Key: quoted key with unicode",
        R"("héllo" = 1)",
        Expect::SUCCESS);

    runTest("Dotted key: quoted empty segments",
        R"("".""."" = 1)",
        Expect::SUCCESS);

    runTest("Dotted key: extend implicit table with new sub-key",
        "a.b = 1\na.c = 2",
        Expect::SUCCESS);

    runTest("Dotted key: redefine scalar via dotted key (invalid)",
        "a = 1\na.b = 2",
        Expect::FAILURE);

    runTest("Dotted key: redefine from dotted to scalar (invalid)",
        "a.b = 1\na = 2",
        Expect::FAILURE);

    runTest("Key: bare key with only digits and dashes",
        "1-2-3 = true",
        Expect::SUCCESS);

    runTest("Key: bare key starting with digit",
        "0abc = true",
        Expect::SUCCESS);

    runTest("Key: bare key with cyrillic (invalid)",
        u8"ключ = 1",
        Expect::FAILURE);

    runTest("Key: duplicate in inline table (invalid)",
        "t = {a = 1, a = 2}",
        Expect::FAILURE);

    runTest("Key: duplicate across dotted key and table header (invalid)",
        "a.b = 1\n[a]\nb = 2",
        Expect::FAILURE);

    // ══════════════════════════════════════════════════════
    //  E. ARRAYS — corner cases
    // ══════════════════════════════════════════════════════

    runTest("Array: deeply nested empty arrays",
        "arr = [[[[]]]]",
        Expect::SUCCESS);

    runTest("Array: single element no trailing comma",
        "arr = [42]",
        Expect::SUCCESS);

    runTest("Array: only a trailing comma (invalid)",
        "arr = [,]",
        Expect::FAILURE);

    runTest("Array: double comma (invalid)",
        "arr = [1,,2]",
        Expect::FAILURE);

    runTest("Array: leading comma (invalid)",
        "arr = [,1]",
        Expect::FAILURE);

    runTest("Array: mixed integers and floats (TOML 1.1 allows mixed types)",
        "arr = [1, 2.0]",
        Expect::SUCCESS);

    runTest("Array: mixed integers and strings (TOML 1.1 allows mixed types)",
        "arr = [1, \"two\"]",
        Expect::SUCCESS);

    runTest("Array: mixed booleans and integers (TOML 1.1 allows mixed types)",
        "arr = [true, 1]",
        Expect::SUCCESS);

    runTest("Array: comment after trailing comma",
        "arr = [1, 2, # comment\n]",
        Expect::SUCCESS);

    runTest("Array: newline before closing bracket",
        "arr = [1,\n2\n]",
        Expect::SUCCESS);

    runTest("Array: 100 elements",
        [](){
            std::string s = "arr = [";
            for (int i = 0; i < 100; ++i) { if (i) s += ", "; s += std::to_string(i); }
            return s + "]";
        }(),
        Expect::SUCCESS);

    // ══════════════════════════════════════════════════════
    //  F. INLINE TABLES — corner cases
    // ══════════════════════════════════════════════════════

    runTest("Inline table: newline inside is allowed (TOML 1.1)",
        "t = {\na = 1,\nb = 2\n}",
        Expect::SUCCESS);

    runTest("Inline table: nested inline table three levels deep",
        "t = {a = {b = {c = 42}}}",
        Expect::SUCCESS);

    runTest("Inline table: value is an array",
        "t = {a = [1, 2, 3]}",
        Expect::SUCCESS);

    runTest("Inline table: value is a multi-line basic string",
        "t = {a = \"\"\"hello\nworld\"\"\"}",
        Expect::SUCCESS);

    runTest("Inline table: cannot extend after definition via table header (invalid)",
        "t = {a = 1}\n[t]\nb = 2",
        Expect::FAILURE);

    runTest("Inline table: cannot extend after definition via dotted key (invalid)",
        "t = {a = 1}\nt.b = 2",
        Expect::FAILURE);

    runTest("Inline table: dotted keys inside inline table",
        "t = {a.b = 1, a.c = 2}",
        Expect::SUCCESS);

    runTest("Inline table: duplicate dotted keys inside inline table (invalid)",
        "t = {a.b = 1, a.b = 2}",
        Expect::FAILURE);

    // ══════════════════════════════════════════════════════
    //  G. STANDARD TABLES — complex interactions
    // ══════════════════════════════════════════════════════

    runTest("Table: empty header with no body",
        "[empty]",
        Expect::SUCCESS);

    runTest("Table: implicit parent table, then explicit parent",
        "[a.b]\nval = 1\n[a]\nother = 2",
        Expect::SUCCESS);

    runTest("Table: explicit parent then implicit child",
        "[a]\nother = 2\n[a.b]\nval = 1",
        Expect::SUCCESS);

    runTest("Table: two levels of implicit parents",
        "[a.b.c]\nval = 1",
        Expect::SUCCESS);

    runTest("Table: reopen explicit table (invalid)",
        "[t]\na = 1\n[t]\nb = 2",
        Expect::FAILURE);

    runTest("Table: reopen implicit parent twice (invalid)",
        "[a.b]\nval = 1\n[a]\nother = 2\n[a]\nmore = 3",
        Expect::FAILURE);

    runTest("Table: header with quoted segment containing dot",
        "[\"a.b\".c]\nval = 1",
        Expect::SUCCESS);

    runTest("Table: header with spaces around dot",
        "[ a . b ]\nval = 1",
        Expect::SUCCESS);

    runTest("Table: root key defined before any table header",
        "root = 1\n[t]\nval = 2",
        Expect::SUCCESS);

    runTest("Table: dotted key extends implicit parent created by table header",
        "[a]\nb.c = 1\nb.d = 2",
        Expect::SUCCESS);

    runTest("Table: inline table value, then table header for same key (invalid)",
        "a = {b = 1}\n[a]\nc = 2",
        Expect::FAILURE);

    runTest("Table: key defined in root, then table with different name",
        "x = 1\n[y]\nz = 2",
        Expect::SUCCESS);

    runTest("Table: key in sub-table does not clash with root key of same name",
        "name = \"root\"\n[section]\nname = \"section\"",
        Expect::SUCCESS);

    // ══════════════════════════════════════════════════════
    //  H. ARRAY OF TABLES — complex interactions
    // ══════════════════════════════════════════════════════

    runTest("AoT: empty array of tables",
        "[[arr]]",
        Expect::SUCCESS);

    runTest("AoT: three entries",
        "[[arr]]\nval = 1\n[[arr]]\nval = 2\n[[arr]]\nval = 3",
        Expect::SUCCESS);

    runTest("AoT: sub-table inside AoT entry",
        "[[arr]]\nval = 1\n[arr.meta]\ninfo = \"ok\"",
        Expect::SUCCESS);

    runTest("AoT: sub-AoT inside AoT",
        "[[arr]]\nname = \"a\"\n[[arr.sub]]\nval = 1\n[[arr]]\nname = \"b\"\n[[arr.sub]]\nval = 2",
        Expect::SUCCESS);

    runTest("AoT: cannot redefine as plain table (invalid)",
        "[[arr]]\nval = 1\n[arr]\nval = 2",
        Expect::FAILURE);

    runTest("AoT: cannot redefine static array as AoT (invalid)",
        "arr = [1]\n[[arr]]\nval = 1",
        Expect::FAILURE);

    runTest("AoT: cannot redefine AoT as static array",
        "[[arr]]\nval = 1\narr = [1]",
        Expect::SUCCESS);

    runTest("AoT: sub-AoT in entry that was later explicitly defined (invalid)",
        "[[a]]\n[[a.b]]\nval = 1\n[a]\nother = 2",
        Expect::FAILURE);

    runTest("AoT: dotted key inside AoT entry",
        "[[products]]\nname.first = \"drill\"\nname.last = \"pro\"",
        Expect::SUCCESS);

    runTest("AoT: inline table array inside AoT entry",
        "[[items]]\nflags = [true, false]\n[[items]]\nflags = [false, true]",
        Expect::SUCCESS);

    runTest("AoT: deep sub-AoT two levels",
        "[[a]]\n[[a.b]]\n[[a.b.c]]\nval = 1",
        Expect::SUCCESS);

    runTest("AoT: scalar defined then AoT with same key (invalid)",
        "arr = 1\n[[arr]]\nval = 2",
        Expect::FAILURE);

    // ══════════════════════════════════════════════════════
    //  I. WHITESPACE / ENCODING edge cases
    // ══════════════════════════════════════════════════════

    runTest("Whitespace: only CRLF line",
        "\r\n",
        Expect::SUCCESS);

    runTest("Whitespace: tab before key",
        "\tkey = 1",
        Expect::SUCCESS);

    runTest("Whitespace: tab between key and equals",
        "key\t=\t1",
        Expect::SUCCESS);

    runTest("Whitespace: bare carriage return (invalid — CR not followed by LF)",
        "key = 1\rother = 2",
        Expect::FAILURE);

    runTest("Whitespace: form-feed (0x0C) inside basic string (invalid control char)",
        "key = \"\x0c\"",
        Expect::FAILURE);

    runTest("Whitespace: vertical tab (0x0B) inside basic string (invalid control char)",
        "key = \"\x0b\"",
        Expect::FAILURE);

    runTest("Whitespace: multiple blank lines between tables",
        "[a]\nx = 1\n\n\n\n[b]\ny = 2",
        Expect::SUCCESS);

    runTest("Whitespace: trailing spaces after value",
        "key = 1   ",
        Expect::SUCCESS);

    runTest("Whitespace: trailing spaces after table header",
        "[t]   \nval = 1",
        Expect::SUCCESS);

    // ══════════════════════════════════════════════════════
    //  J. COMMENTS — edge cases
    // ══════════════════════════════════════════════════════

    runTest("Comment: hash inside literal string is not a comment",
        "key = '#not a comment'",
        Expect::SUCCESS);

    runTest("Comment: hash inside multi-line literal string is not a comment",
        "key = '''\n# not a comment\n'''",
        Expect::SUCCESS);

    runTest("Comment: NUL byte inside comment (invalid)",
        "# hello\x00world\nkey = 1",
        Expect::FAILURE);

    runTest("Comment: DEL (0x7F) inside comment (invalid)",
        "# hello\x7fworld\nkey = 1",
        Expect::FAILURE);

    runTest("Comment: inline after array element",
        "arr = [\n  1, # first\n  2  # second\n]",
        Expect::SUCCESS);

    runTest("Comment: between table header and first key",
        "[t] # comment\nval = 1",
        Expect::SUCCESS);

    runTest("Comment: between AoT header and first key",
        "[[arr]] # comment\nval = 1",
        Expect::SUCCESS);

    runTest("Comment: entire document is comments",
        "# line 1\n# line 2\n# line 3\n",
        Expect::SUCCESS);

    // ══════════════════════════════════════════════════════
    //  K. COMPLEX / REALISTIC DOCUMENTS
    // ══════════════════════════════════════════════════════

    runTest("Complex: realistic server config",
        R"(
[server]
host = "localhost"
port = 8080
workers = 4
debug = false

[server.tls]
enabled = true
cert_file = "/etc/ssl/cert.pem"
key_file  = "/etc/ssl/key.pem"

[[server.virtual_hosts]]
domain = "example.com"
root   = "/var/www/example"

[[server.virtual_hosts]]
domain = "api.example.com"
root   = "/var/www/api"
)",
        Expect::SUCCESS);

    runTest("Complex: deeply nested implicit tables via dotted key",
        "a.b.c.d.e.f.g = 42",
        Expect::SUCCESS);

    runTest("Complex: mix of inline tables and AoT",
        R"(
[[users]]
name = "Alice"
roles = ["admin", "user"]
address = {city = "Paris", zip = "75001"}

[[users]]
name = "Bob"
roles = ["user"]
address = {city = "Lyon", zip = "69001"}
)",
        Expect::SUCCESS);

    runTest("Complex: AoT with nested AoT and subtables",
        R"(
[[books]]
title = "TOML for Beginners"

[[books.authors]]
name = "Alice"

[[books.authors]]
name = "Bob"

[[books]]
title = "Advanced TOML"

[[books.authors]]
name = "Carol"
)",
        Expect::SUCCESS);

    runTest("Complex: all scalar types in one document",
        R"(
int_val     = 42
neg_int     = -7
hex_val     = 0xCAFE
oct_val     = 0o755
bin_val     = 0b1100_1010
float_val   = 3.14159
sci_val     = 6.022e23
inf_val     = inf
nan_val     = nan
bool_true   = true
bool_false  = false
str_basic   = "hello\nworld"
str_literal = 'C:\Users\toml'
str_multi   = """
line one
line two
"""
str_lit_ml  = '''
no \n escape here
'''
)",
        Expect::SUCCESS);

    runTest("Complex: large AoT with 50 entries",
        [](){
            std::string s;
            for (int i = 0; i < 50; ++i)
                s += "[[items]]\nid = " + std::to_string(i)
                   + "\nname = \"item_" + std::to_string(i) + "\"\n";
            return s;
        }(),
        Expect::SUCCESS);

    runTest("Complex: dotted keys and table headers building same tree",
        R"(
[database]
host = "db.local"
port = 5432
credentials.user = "admin"
credentials.pass = "secret"
)",
        Expect::SUCCESS);

    runTest("Complex: table header followed by many dotted sub-keys",
        R"(
[config]
a.b.c = 1
a.b.d = 2
a.e   = 3
f     = 4
)",
        Expect::SUCCESS);

    runTest("Complex: AoT entry contains inline array of inline tables",
        R"(
[[routes]]
path = "/api"
middleware = [{name = "auth"}, {name = "log"}]
)",
        Expect::SUCCESS);

    runTest("Complex: multiple root keys interleaved with tables",
        R"(
version = "1.0"
debug = false

[build]
target = "release"

name = "myapp"
)",
        // 'name' after [build] belongs to [build], not root — valid
        Expect::SUCCESS);

    // ══════════════════════════════════════════════════════
    //  L. PRECISE ERROR IDENTIFICATION
    // ══════════════════════════════════════════════════════

    runTest("Error: value on same line as table header (invalid)",
        "[t] val = 1",
        Expect::FAILURE);

    runTest("Error: two values on same line without separator (invalid)",
        "a = 1 b = 2",
        Expect::FAILURE);

    runTest("Error: unterminated multi-line basic string",
        "val = \"\"\"\nno close",
        Expect::FAILURE);

    runTest("Error: unterminated multi-line literal string",
        "val = '''\nno close",
        Expect::FAILURE);

    runTest("Error: AoT header with trailing garbage (invalid)",
        "[[arr]] garbage\nval = 1",
        Expect::FAILURE);

    runTest("Error: table header with trailing garbage (invalid)",
        "[t] garbage\nval = 1",
        Expect::FAILURE);

    runTest("Error: equals without value (invalid)",
        "key =",
        Expect::FAILURE);

    runTest("Error: equals without value followed by newline (invalid)",
        "key =\n",
        Expect::FAILURE);

    runTest("Error: key with only whitespace (invalid)",
        "   = 1",
        Expect::FAILURE);

    runTest("Error: float exponent with no digits (invalid)",
        "val = 1e",
        Expect::FAILURE);

    runTest("Error: float with double exponent (invalid)",
        "val = 1e2e3",
        Expect::FAILURE);

    runTest("Error: array with only whitespace and comment is valid empty array",
        "arr = [ # nothing\n]",
        Expect::SUCCESS);

    runTest("Error: define scalar then AoT with same name (invalid)",
        "arr = 1\n[[arr]]\nval = 2",
        Expect::FAILURE);

    runTest("Error: AoT then define scalar with same name",
        "[[arr]]\nval = 1\narr = 2",
        Expect::SUCCESS);

    runTest("Error: table header missing closing bracket (invalid)",
        "[t\nval = 1",
        Expect::FAILURE);

    runTest("Error: AoT header missing closing brackets (invalid)",
        "[[arr\nval = 1",
        Expect::FAILURE);

    runTest("Error: empty table header (invalid)",
        "[]\nval = 1",
        Expect::FAILURE);

    runTest("Error: empty AoT header (invalid)",
        "[[]]\nval = 1",
        Expect::FAILURE);

    runTest("Error: key with newline inside quoted key (invalid)",
        "\"ke\ny\" = 1",
        Expect::FAILURE);

    // ══════════════════════════════════════════════════════
    //  M. BOUNDARY / OVERFLOW
    // ══════════════════════════════════════════════════════

    runTest("Integer: INT64_MAX (9223372036854775807)",
        "val = 9223372036854775807",
        Expect::SUCCESS);

    runTest("Integer: INT64_MAX + 1 (overflow, invalid)",
        "val = 9223372036854775808",
        Expect::FAILURE);

    runTest("Integer: INT64_MIN (-9223372036854775808)",
        "val = -9223372036854775808",
        Expect::SUCCESS);

    runTest("Integer: INT64_MIN - 1 (overflow, invalid)",
        "val = -9223372036854775809",
        Expect::FAILURE);

    runTest("Float: large but valid exponent (1e308)",
        "val = 1e308",
        Expect::SUCCESS);

    runTest("Float: exponent causing double overflow (invalid)",
        "val = 1e99999",
        Expect::FAILURE);

    runTest("String: very long basic string (1000 chars)",
        "val = \"" + std::string(1000, 'x') + "\"",
        Expect::SUCCESS);

    runTest("Key: very long bare key (128 chars)",
        std::string(128, 'a') + " = 1",
        Expect::SUCCESS);

    runTest("Nesting: 20-level deep dotted key",
        [](){
            std::string s;
            for (int i = 0; i < 20; ++i) { if (i) s += "."; s += "k" + std::to_string(i); }
            return s + " = true";
        }(),
        Expect::SUCCESS);

    runTest("Array: deeply nested arrays 10 levels",
        [](){
            std::string s(10, '[');
            s += "1";
            s += std::string(10, ']');
            return "arr = " + s;
        }(),
        Expect::SUCCESS);

    // ══════════════════════════════════════════════════════
    //  N. UNICODE IN VALUES
    // ══════════════════════════════════════════════════════

    runTest("Unicode: multi-byte UTF-8 in basic string literal",
        u8"val = \"日本語テスト\"",
        Expect::SUCCESS);

    runTest("Unicode: emoji directly in basic string",
        u8"val = \"🍕🍔\"",
        Expect::SUCCESS);

    runTest("Unicode: right-to-left text in basic string",
        u8"val = \"مرحبا\"",
        Expect::SUCCESS);

    runTest("Unicode: NUL codepoint via \\u0000 (invalid)",
        R"(val = "\u0000")",
        Expect::FAILURE);

    runTest("Unicode: surrogate half via \\uDC00 (invalid)",
        R"(val = "\uDC00")",
        Expect::FAILURE);

    runTest("Unicode: max valid codepoint \\U0010FFFF",
        R"(val = "\U0010FFFF")",
        Expect::SUCCESS);

    runTest("Unicode: one above max codepoint \\U00110000 (invalid)",
        R"(val = "\U00110000")",
        Expect::FAILURE);

    runTest("Unicode: non-breaking space via \\u00A0",
        R"(val = "\u00A0")",
        Expect::SUCCESS);

    runTest("Unicode: UTF-8 in literal string key value (allowed)",
        u8"val = 'héllo wörld'",
        Expect::SUCCESS);

    runTest("Unicode: replacement character U+FFFD in basic string",
        u8"val = \"\xef\xbf\xbd\"",
        Expect::SUCCESS);

    // ══════════════════════════════════════════════════════
    //  O. TRICKY STRUCTURAL INTERACTIONS
    // ══════════════════════════════════════════════════════

    runTest("Structure: AoT entry sub-table, then next AoT entry re-introduces sub-table",
        "[[a]]\n[a.b]\nval = 1\n[[a]]\n[a.b]\nval = 2",
        Expect::SUCCESS);

    runTest("Structure: dotted key creating implicit table, then AoT under it (invalid)",
        "a.b = 1\n[[a.c]]\nval = 1",
        // a.b defined a as an implicit table; [[a.c]] creates AoT under a — should be valid
        Expect::SUCCESS);

    runTest("Structure: inline table as AoT element value",
        "[[arr]]\nentry = {x = 1, y = 2}",
        Expect::SUCCESS);

    runTest("Structure: key shadow — sub-key same name as parent table key",
        "[a]\nb = 1\n[a.c]\nb = 2",
        // a.b and a.c.b are distinct — valid
        Expect::SUCCESS);

    runTest("Structure: empty inline table as array element",
        "arr = [{}, {}, {}]",
        Expect::SUCCESS);

    runTest("Structure: array of empty arrays",
        "arr = [[], [], []]",
        Expect::SUCCESS);

    runTest("Structure: boolean array in inline table in AoT",
        "[[items]]\nflags = {on = true, off = false}",
        Expect::SUCCESS);

    runTest("Structure: self-referential dotted key in inline table",
        "t = {a.b.c = {d.e = 1}}",
        Expect::SUCCESS);

    runTest("Structure: inline table trailing comma then close (TOML 1.1 allows)",
        "t = {a = 1, b = 2,}",
        Expect::SUCCESS);

    runTest("Structure: deeply nested AoT, 3 levels",
        R"(
[[a]]
[[a.b]]
[[a.b.c]]
val = 1
[[a.b]]
[[a.b.c]]
val = 2
[[a]]
[[a.b]]
[[a.b.c]]
val = 3
)",
        Expect::SUCCESS);

    runTest("Structure: integer key in dotted path",
        "1.2.3 = true",
        Expect::SUCCESS);

    runTest("Structure: mixed quoted and bare segments in dotted key",
        "bare.\"quoted\".again = 1",
        Expect::SUCCESS);
}

// ============================================================
//  main
// ============================================================
int main(int argc, char** argv)
{
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

    registerTests();
    printSummary();

    for (auto& r : g_results)
        if (!r.passed) return 1;
    return 0;
}