/**
 * Created by TekuConcept on October 29, 2021
 */

#ifndef WIKI_AST_CONSUMER_H
#define WIKI_AST_CONSUMER_H

#include "clang/AST/AST.h"

namespace wiki {

class WikiASTConsumer : public clang::ASTConsumer {
public:
    WikiASTConsumer(
        clang::CompilerInstance &Instance,
        std::set<std::string> ParsedTemplates);

    void HandleTranslationUnit(clang::ASTContext& context) override;

private:
    clang::CompilerInstance &Instance;
    std::set<std::string> ParsedTemplates;
};

} /* namespace wiki */

#endif
