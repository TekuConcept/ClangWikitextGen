/**
 * Created by TekuConcept on October 29, 2021
 */

//===----------------------------------------------------------------------===//
//
// Clang plugin which generates wikitext from an input file.
//
// clang                                \
//     -cc1                             \
//     -load ./wiki-generator.so        \
//     -plugin wikigen                  \
//     some-input-file.c
//
//===----------------------------------------------------------------------===//

#include <set>
#include <vector>
#include <string>

#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/raw_ostream.h"

#include "wiki-ast-consumer.h"

namespace wiki {

    class WikiASTPlugin : public clang::PluginASTAction {
    protected:
        std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
            clang::CompilerInstance &CI, llvm::StringRef) override
        { return std::make_unique<WikiASTConsumer>(CI, ParsedTemplates); }

        bool ParseArgs(
            const clang::CompilerInstance&,
            const std::vector<std::string>&) override
        { return true; }

    private:
        std::set<std::string> ParsedTemplates;
    };

} /* namespace wiki */

static clang::FrontendPluginRegistry::Add<wiki::WikiASTPlugin>
X("wikigen", "create wikitext");
