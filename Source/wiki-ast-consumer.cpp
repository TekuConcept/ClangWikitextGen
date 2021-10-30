/**
 * Created by TekuConcept on October 29, 2021
 */

#include <set>
#include <string>

#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/raw_ostream.h"

#include "wiki-ast-consumer.h"
#include "wiki-ast-visitor.h"

using namespace wiki;

WikiASTConsumer::WikiASTConsumer(
    clang::CompilerInstance &Instance,
    std::set<std::string> ParsedTemplates)
: Instance(Instance), ParsedTemplates(ParsedTemplates) {}


void WikiASTConsumer::HandleTranslationUnit(clang::ASTContext& context)
{
    WikiASTVisitor generator(ParsedTemplates, &context);
    generator.TraverseDecl(context.getTranslationUnitDecl());
}
