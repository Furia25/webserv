#include <iostream>
#include <sstream>
#include <fstream>
#include "TOMLDocument.hpp"
#include "TOMLTokenizer.hpp"
#include "TOMLDebugJson.hpp"

static void tomlLexer(const std::string& src)
{
	std::istringstream ss(src);
	toml::Tokenizer tokenizer(ss);
	while (!tokenizer.eof())
	{
		toml::Token token = tokenizer.next_token();
		if (token.getType() == toml::Token::NEW_LINE)
		{
			std::cout << "[NL]";
			continue ;
		}
		std::cout << "[";
		switch (token.getType())
		{
			#define X(name, ...) case toml::Token::name: std::cout << #name; break;
			_TOML_TOKEN_TYPES_
			#undef X
		}
		switch (token.getType())
		{
			case toml::Token::STRING:
			case toml::Token::MULTI_STRING:
			case toml::Token::MULTI_LITERAL_STRING:
			case toml::Token::LITERAL_STRING:
			case toml::Token::IDENT: std::cout << " \"" << token.getLiteral() << "\""; break;
		}
		std::cout << "]";
	}
}

// Utilitaire : parse une string TOML et affiche le JSON résultant
static void testToml(const std::string& label, const std::string& toml_src)
{
    std::cout << "=== " << label << " ===\n";
    std::cout << "TOML:\n" << toml_src << "\n";
    try
    {
		std::cout << "TOKENS:\n";
		tomlLexer(toml_src);
		std::cout << "\n";
        toml::Document doc;
        std::istringstream ss(toml_src);
        doc.from_stream(ss);
        std::cout << "JSON:\n" << toml::toJson(doc) << "\n";
    }
    catch (const std::exception& e)
    {
        std::cout << "ERREUR: " << e.what() << "\n";
    }
    std::cout << "\n";
}

int main(int argc, char **argv)
{
    // --- Cas basiques ---
    testToml("Clés simples",
        "name = \"Alice\"\n"
        "age = 30\n"
        "active = true\n"
    );

    testToml("Flottants et booléens",
        "pi = 3.14159\n"
        "ratio = 1.0e-3\n"
        "flag = false\n"
    );

    testToml("Table imbriquée",
        "[database]\n"
        "host = \"localhost\"\n"
        "port = 5432\n"
        "name = \"mydb\"\n"
    );

    testToml("Tableau inline",
        "primes = [2, 3, 5, 7, 11]\n"
    );

    testToml("Tableau de tables",
        "[[fruits]]\n"
        "name = \"apple\"\n"
        "color = \"red\"\n"
        "\n"
        "[[fruits]]\n"
        "name = \"banana\"\n"
        "color = \"yellow\"\n"
    );

    testToml("Table inline (brace)",
        "point = { x = 1, y = 2 }\n"
    );

    testToml("Clés pointées",
        "a.b.c = 42\n"
    );

    testToml("String avec caractères spéciaux",
        "msg = \"hello\\nworld\\ttab\"\n"
    );

    testToml("Entiers bases différentes",
        "hex = 0xFF\n"
        "oct = 0o77\n"
        "bin = 0b1010\n"
    );

    // --- Cas depuis un fichier si fourni en argument ---
    if (argc > 1)
    {
        std::cout << "=== Fichier: " << argv[1] << " ===\n";
        try
        {
            toml::Document doc;
            doc.from_file(argv[1]);
            std::cout << toml::toJson(doc) << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << "ERREUR: " << e.what() << "\n";
        }
    }

    return 0;
}