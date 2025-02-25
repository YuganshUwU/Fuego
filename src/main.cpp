#include<iostream>
#include<sstream>
#include<fstream>
#include <optional>
#include <vector>

#include "generation.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Incorrect usage. Correct usage is ..." << std::endl;
        std::cerr << "fue <input.fue>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string content;
    {
        std::stringstream content_stream;
        std::fstream input(argv[1], std::ios::in);
        content_stream << input.rdbuf();
        content = content_stream.str();
    }

    Tokenizer tokenizer(std::move(content));
    std::vector<Token> token = tokenizer.tokenize();

    Parser parser(std::move(token));
    std::optional<NodeProg> prog = parser.parse_prog();

    if (!prog.has_value()) {
        std::cerr << "Invalid Program" << std::endl;
        exit(EXIT_FAILURE);
    }

    {
        Generator generator(prog.value());
        std::fstream file("out.asm", std::ios::out);
        file << generator.gen_prog();
    }

    system("nasm -f elf64 out.asm");
    system("ld -o out out.o");

    return EXIT_SUCCESS;
}