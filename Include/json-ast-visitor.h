/**
 * Created by TekuConcept on November 3, 2021
 */

#ifndef JSON_GENERATOR_H
#define JSON_GENERATOR_H

#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "json.h"

using json = nlohmann::json;

namespace wiki {

    class JsonASTVisitor : public clang::RecursiveASTVisitor<JsonASTVisitor> {
    public:
        JsonASTVisitor(
            const std::set<std::string> &ParsedTemplates,
            const clang::ASTContext* Context);
        ~JsonASTVisitor();

        bool VisitCXXRecordDecl(clang::CXXRecordDecl *D);

    private:
        const std::set<std::string> &ParsedTemplates;
        const clang::ASTContext* Context;
        json jast;

        void M_AddFileName(json&, const clang::CXXRecordDecl*) const;
        void M_AddRecordTitle(json&, const clang::CXXRecordDecl*) const;
        void M_AddTemplate(json&, const clang::CXXRecordDecl*) const;
        void M_AddTemplateHelper(json&, const clang::TemplateDecl*) const;
        void M_AddMemberFunctions(json&, const clang::CXXRecordDecl*) const;
        void M_AddFunctionHelper(json&, const clang::FunctionDecl*, bool) const;
        void M_AddMemberProperties(json&, const clang::CXXRecordDecl*) const;
    };

} /* namespace wiki */

#endif
