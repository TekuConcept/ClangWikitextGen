/**
 * Created by TekuConcept on October 29, 2021
 */

#ifndef WIKI_GENERATOR_H
#define WIKI_GENERATOR_H

#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace wiki {

    class WikiASTVisitor : public clang::RecursiveASTVisitor<WikiASTVisitor> {
    public:
        WikiASTVisitor(
            const std::set<std::string> &ParsedTemplates,
            const clang::ASTContext* Context);

        bool VisitCXXRecordDecl(clang::CXXRecordDecl *D);

    private:
        const std::set<std::string> &ParsedTemplates;
        const clang::ASTContext* Context;

        void M_PrintFileName(llvm::raw_ostream&, const clang::CXXRecordDecl*) const;
        void M_PrintRecordTitle(llvm::raw_ostream&, const clang::CXXRecordDecl*) const;
        void M_PrintTemplate(llvm::raw_ostream&, const clang::CXXRecordDecl*) const;
        void M_PrintTemplateHelper(llvm::raw_ostream&, const clang::TemplateDecl*) const;
        void M_PrintMemberFunctions(llvm::raw_ostream&, const clang::CXXRecordDecl*) const;
        void M_PrintFunctionHelper(llvm::raw_ostream&, const clang::FunctionDecl*, bool) const;
        void M_PrintMemberProperties(llvm::raw_ostream&, const clang::CXXRecordDecl*) const;
    };

} /* namespace wiki */

#endif
