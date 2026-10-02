#include <print>
#include <fstream>
#include <sstream>

#include "generation.h"
#include "parser.h"
#include "sugar.h"
#include "tokenizer.h"
#include "arena.h"



int main(int argc, char* argv[]) {

	std::string contents;
	{
		std::fstream stream("../resources/test.kata",std::ios::in);
		std::stringstream contents_stream;
		contents_stream << stream.rdbuf();
		contents = contents_stream.str();
	}

	print("Contents: {}",contents);

	Tokenizer tokenizer(contents);

	const std::vector<Token> tokens = tokenizer.tokenize();

	print("Tokenized: {}",tokens.size());

	Parser parser(tokens);
	if (auto root = parser.parse()) {
		print("Successfully parsed!");
		Generator gen(root.value());

		String fileName = "gen";
		String filePath = "../resources/";
		String fileFullPath = filePath + fileName + ".asm";
		std::fstream output(fileFullPath,std::ios::out);
		output << gen.generate();

		output.close();
		std::stringstream cmd;
		cmd << "nasm -felf64 " << fileFullPath << " ; ld.lld "<< filePath + fileName <<".o -o " << filePath + fileName;

		print("Running command: {}",cmd.str());
		system(cmd.str().c_str());
	}



	return 0;
}