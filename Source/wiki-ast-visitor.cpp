/**
 * Created by TekuConcept on October 29, 2021
 */

#include "wiki-ast-visitor.h"
#include "llvm/Support/raw_ostream.h"

using namespace wiki;

WikiASTVisitor::WikiASTVisitor(
    const std::set<std::string> &ParsedTemplates,
    const clang::ASTContext* Context)
: ParsedTemplates(ParsedTemplates),
  Context(Context) {}


bool
WikiASTVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl *Decl)
{
    M_PrintFileName(llvm::outs(), Decl);
    llvm::outs() << "\n";
    M_PrintRecordTitle(llvm::outs(), Decl);
    llvm::outs() << "\n";
    M_PrintMemberFunctions(llvm::outs(), Decl);
    M_PrintMemberProperties(llvm::outs(), Decl);
    llvm::outs() << "\n";
    return true;
}


void
WikiASTVisitor::M_PrintFileName(
    llvm::raw_ostream& out,
    const clang::CXXRecordDecl* D) const
{
    const clang::SourceManager& SrcMgr = Context->getSourceManager();
    clang::SourceLocation SourceLocation = D->getLocation();
    std::string filename = SourceLocation.printToString(SrcMgr);
    out << filename << ": ";
}


void
WikiASTVisitor::M_PrintRecordTitle(
    llvm::raw_ostream& out,
    const clang::CXXRecordDecl* Decl) const
{
    M_PrintTemplate(llvm::outs(), Decl);

    // kind: class, struct, etc...
    out << Decl->getKindName() << " ";
    Decl->printQualifiedName(out);

    bool multi = false;
    for (auto base = Decl->bases_begin(); base != Decl->bases_end(); base++) {
        if (multi) out << ", ";
        else {
            out << " : ";
            multi = true;
        }

        switch (base->getAccessSpecifier()) {
        case clang::AS_public:    out << "public ";    break;
        case clang::AS_protected: out << "protected "; break;
        case clang::AS_private:   out << "private ";   break;
        case clang::AS_none:                           break;
        }

        out << base->getType().getAsString();
    }

    out << ";";
}


void
WikiASTVisitor::M_PrintTemplate(
    llvm::raw_ostream& out,
    const clang::CXXRecordDecl* Decl) const
{
    if (!Decl->isTemplated()) return;
    clang::TemplateDecl* Template = Decl->getDescribedTemplate();
    M_PrintTemplateHelper(out, Template);
}


void WikiASTVisitor::M_PrintTemplateHelper(
    llvm::raw_ostream& out,
    const clang::TemplateDecl* Template) const
{
    bool multi = false;
    out << "template <";

    clang::TemplateParameterList* list = Template->getTemplateParameters();
    if (!list) goto end;

    for (clang::TemplateParameterList::iterator token = list->begin();
        token != list->end(); token++)
    {
        if (multi) out << ", ";
        else multi = true;

        std::string defarg;
        switch ((*token)->getKind()) {
        case clang::Decl::Kind::TemplateTemplateParm: {
            const clang::TemplateTemplateParmDecl* ttpd =
                static_cast<const clang::TemplateTemplateParmDecl*>(*token);
            M_PrintTemplateHelper(out, ttpd);
            if (ttpd->hasDefaultArgument())
                defarg = " = [...]";
            out << "class";
        } break;
        case clang::Decl::Kind::TemplateTypeParm: {
            const clang::TemplateTypeParmDecl* ttpd =
                static_cast<const clang::TemplateTypeParmDecl*>(*token);
            if (ttpd->hasDefaultArgument())
                defarg = ttpd->getDefaultArgument().getAsString();
            out << "typename";
        } break;
        }

        std::string tname = (*token)->getNameAsString();
        if (tname.size() > 0) out << " " << tname;
        if (defarg.size() > 0) out << " = " << defarg;
    }

    end: out << "> ";
}


void
WikiASTVisitor::M_PrintMemberFunctions(
    llvm::raw_ostream& out,
    const clang::CXXRecordDecl* Decl) const
{
    bool has_functions = false;
    for (auto method = Decl->method_begin(); method != Decl->method_end(); method++) {
        if (!has_functions) {
            out << "----------\n";
            has_functions = true;
        }
        M_PrintFunctionHelper(out, static_cast<const clang::FunctionDecl*>(*method), false);
        out << ";\n";
    };

    const clang::DeclContext* DeclContext =
        static_cast<const clang::DeclContext*>(Decl);
    for (auto const *NextDecl : DeclContext->decls()) {
        auto const *method =
            llvm::dyn_cast<clang::FunctionTemplateDecl>(NextDecl);
        if (!method) continue;
        if (!has_functions) {
            out << "----------\n";
            has_functions = true;
        }
        M_PrintTemplateHelper(out,
            static_cast<const clang::TemplateDecl*>(method));
        out << "\n";
        clang::NamedDecl* Template = method->getTemplatedDecl();
        M_PrintFunctionHelper(out,
            static_cast<const clang::FunctionDecl*>(Template), true);
        out << "\n";
    }

    if (has_functions) out << "----------\n";
}


void
WikiASTVisitor::M_PrintFunctionHelper(
    llvm::raw_ostream& out,
    const clang::FunctionDecl* method,
    bool isTemplate) const
{
    switch (method->getAccess()) {
    case clang::AS_public:    out << "public ";    break;
    case clang::AS_protected: out << "protected "; break;
    case clang::AS_private:   out << "private ";   break;
    case clang::AS_none:                           break;
    }

    if (method->isConsteval()) out << "consteval ";
    if (!method->isImplicit() &&
        method->getKind() == clang::Decl::Kind::CXXConstructor)
        out << "explicit ";
    if (method->isConstexpr()) out << "constexpr ";
    if (method->isStatic()) out << "static ";
    if (method->isInlined()) out << "inline ";
    if (!isTemplate) {
        const auto* mmethod = static_cast<const clang::CXXMethodDecl*>(method);
        if (mmethod->isVirtual()) out << "virtual ";
        if (mmethod->isVolatile()) out << "volatile ";
    }

    clang::QualType return_type = method->getReturnType();
    out << return_type.getAsString() << " ";
    out << method->getNameAsString();

    if (method->param_empty()) out << "()";
    else {
        bool has_params = false;
        out << "(";
        for (auto param = method->param_begin();
            param != method->param_end(); param++)
        {
            if (has_params) out << ", ";
            else has_params = true;
            out << (*param)->getType().getAsString();
            auto name = (*param)->getNameAsString();
            if (name.size() > 0) out << " " << name;
        }
        out << ")";
    }

    if (!isTemplate) {
        const auto* mmethod = static_cast<const clang::CXXMethodDecl*>(method);
        if (mmethod->isConst()) out << " const";
    }

    if (method->isPure()) out << "= 0";
    if (method->isDeleted()) out << " = delete";
    if (method->isDefaulted()) out << " = default";
}


void
WikiASTVisitor::M_PrintMemberProperties(
    llvm::raw_ostream& out,
    const clang::CXXRecordDecl* Decl) const
{
    bool has_fields = false;
    const clang::DeclContext* DeclContext =
        static_cast<const clang::DeclContext*>(Decl);
    for (auto const *NextDecl : DeclContext->decls()) {
        auto const *field =
            llvm::dyn_cast<clang::FieldDecl>(NextDecl);
        if (!field) continue;
        if (!has_fields) {
            out << "++++++++++\n";
            has_fields = true;
        }

        switch (field->getAccess()) {
        case clang::AS_public:    out << "public ";    break;
        case clang::AS_protected: out << "protected "; break;
        case clang::AS_private:   out << "private ";   break;
        case clang::AS_none:                           break;
        }

        out << field->getType().getAsString() << " ";
        out << field->getNameAsString();
        out << "\n";
    }
    if (has_fields) out << "++++++++++\n";
}
