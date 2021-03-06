// compiler_run.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string>
using namespace std;

#include "../compiler/program.h"
#include "../compiler/lexical_analyzer.h"
#include "../compiler/syntax_analyzer.h"
#include "../compiler/semantic_analyzer.h"
#include "../compiler/bytecode_generator.h"
#include "../compiler/virtual_machine.h"
#include <boost/wave.hpp>
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>
#include <fstream>
#include <streambuf>

using namespace compiler;
using namespace std;

int main(int argc, char **argv)
{
	namespace fs = boost::filesystem;

	fs::path target_file;
	fs::path pa = fs::path(argv[0]).parent_path();
	if (argc == 1) target_file = pa / "main.c";
	else target_file = argv[1];
	string file_name = target_file.string();
	ifstream fin(file_name);

	string code((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());

	typedef boost::wave::cpplexer::lex_iterator<boost::wave::cpplexer::lex_token<>> lex_iterator_type;
	typedef boost::wave::context<string::iterator, lex_iterator_type> context_type;

	context_type ctx(code.begin(), code.end(), file_name.c_str());
	ctx.add_sysinclude_path(pa.string().c_str());

	stringstream ss;
	for (auto it = ctx.begin(); it != ctx.end(); it++)
		ss << (*it).get_value();

	code = ss.str();

	program_ptr p = make_shared<program>(file_name, code);
	lexical_analyzer analyzer(p);
	auto res = analyzer.analyze();
	syntax_analyzer sa(p, res);
	auto tree = sa.analyze();
	semantic_analyzer sem(p);
	sem.analyze(tree);
	bytecode_generator generator(p);
	virtual_machine vm(generator.build(tree));

	vm.init();
	vm.start();
    return 0;
}

