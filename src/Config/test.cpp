#include <iostream>
#include <sstream>
#include <fstream>
#include "TOMLDocument.hpp"
#include "TOMLTokenizer.hpp"
#include "TOMLDebugJson.hpp"

static void tomlLexer(std::istream& ss)
{
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

static void testToml(const std::string& label, const std::string& toml_src)
{
    std::cout << "=== " << label << " ===\n";
    std::cout << "TOML:\n" << toml_src << "\n";
    try
    {
        std::cout << "TOKENS:\n";
        std::istringstream lex_ss(toml_src);
        tomlLexer((std::istream&)lex_ss);
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
    if (argc > 1)
    {
        std::cout << "=== Fichier: " << argv[1] << " ===\n";
        try
        {
            std::fstream stream(argv[1]);
            std::cout << "TOKENS:\n";
            tomlLexer(stream);
            std::cout << "\n";
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