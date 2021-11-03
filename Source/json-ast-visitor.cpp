/**
 * Created by TekuConcept on October 29, 2021
 */

#include <random>
#include <sstream>
#include "wiki-ast-visitor.h"
#include "llvm/Support/raw_ostream.h"

using namespace wiki;

namespace uuid {
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::string generate_uuid_v4() {
        std::stringstream ss;
        int i;
        ss << std::hex;
        for (i = 0; i <  8; i++) ss << dis(gen);
        ss << "-";
        for (i = 0; i <  4; i++) ss << dis(gen);
        ss << "-4";
        for (i = 0; i <  3; i++) ss << dis(gen);
        ss << "-";
        ss << dis2(gen);
        for (i = 0; i <  3; i++) ss << dis(gen);
        ss << "-";
        for (i = 0; i < 12; i++) ss << dis(gen);
        return ss.str();
    }
}


JsonASTVisitor::JsonASTVisitor(
    const std::set<std::string> &ParsedTemplates,
    const clang::ASTContext* Context)
: ParsedTemplates(ParsedTemplates),
  Context(Context)
{
    jast["CXXRecordDecls"] = json::array();
}


~JsonASTVisitor()
{
    // TODO: sava json file
}


bool
JsonASTVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl *Decl)
{
    json record;

    M_AddFileName(record, Decl);
    M_AddRecordTitle(record, Decl);
    M_AddMemberFunctions(record, Decl);
    M_AddMemberProperties(record, Decl);

    jast["CXXRecordDecls"].push_back(record);
    return true;
}


void
JsonASTVisitor::M_AddFileName(
    json& out,
    const clang::CXXRecordDecl* D) const
{
    const clang::SourceManager& SrcMgr = Context->getSourceManager();
    clang::SourceLocation SourceLocation = D->getLocation();
    std::string filename = SourceLocation.printToString(SrcMgr);

    out["filename"] = filename;
}


/*
root: {
    kind: class|struct|...
    name: string
    parents: [
        {
            access: public|protected|private
            name: string
        },
        ...
    ]
}
*/
void
JsonASTVisitor::M_AddRecordTitle(
    json& out,
    const clang::CXXRecordDecl* Decl) const
{
    M_AddTemplate(out, Decl);

    // kind: class, struct, etc...
    out["kind"] = Decl->getKindName();
    out["name"] = Decl->getQualifiedNameAsString();
    out["parents"] = json::array();

    bool multi = false;
    for (auto base = Decl->bases_begin(); base != Decl->bases_end(); base++) {
        json parent;
        switch (base->getAccessSpecifier()) {
        case clang::AS_public:    parent["access"] = "public";    break;
        case clang::AS_protected: parent["access"] = "protected"; break;
        case clang::AS_private:   parent["access"] = "private";   break;
        case clang::AS_none:      parent["access"] = "private";   break;
        }
        parent["name"] = base->getType().getAsString();
        out["parents"].push_back(parent);
    }
}


/*
root: {
    template: [...]
}
*/
void
JsonASTVisitor::M_AddTemplate(
    json& out,
    const clang::CXXRecordDecl* Decl) const
{
    if (!Decl->isTemplated()) return;
    clang::TemplateDecl* Template = Decl->getDescribedTemplate();
    M_AddTemplateHelper(out, Template);
}


/*
template: [
    {
        type: class|string
        value: string
        name: string
        template: [...]
    },
    ...
]
template <type name = value, ...>
*/
void JsonASTVisitor::M_AddTemplateHelper(
    json& out,
    const clang::TemplateDecl* Template) const
{
    bool multi = false;
    json jtemplate = json::array();

    clang::TemplateParameterList* list = Template->getTemplateParameters();
    if (!list) goto end;

    for (clang::TemplateParameterList::iterator token = list->begin();
        token != list->end(); token++)
    {
        json targ;
        targ["value"] = "";

        std::string defarg;
        switch ((*token)->getKind()) {
        case clang::Decl::Kind::TemplateTemplateParm: {
            const clang::TemplateTemplateParmDecl* ttpd =
                static_cast<const clang::TemplateTemplateParmDecl*>(*token);
            M_AddTemplateHelper(targ, ttpd);
            if (ttpd->hasDefaultArgument())
                targ["value"] = "[-check-]";
            targ["type"] = "class";
        } break;
        case clang::Decl::Kind::TemplateTypeParm: {
            const clang::TemplateTypeParmDecl* ttpd =
                static_cast<const clang::TemplateTypeParmDecl*>(*token);
            if (ttpd->hasDefaultArgument())
                targ["value"] = ttpd->getDefaultArgument().getAsString();
            targ["type"] = "typename";
        } break;
        }

        targ["name"] = (*token)->getNameAsString();
        jtemplate
    }

    out["template"] = jtemplate;
}


/*
root: {
    methods: [
        {

        },
        ...
    ]
}
*/
void
JsonASTVisitor::M_AddMemberFunctions(
    json& out,
    const clang::CXXRecordDecl* Decl) const
{
    out["methods"] = json::array();

    bool has_functions = false;
    for (auto method = Decl->method_begin(); method != Decl->method_end(); method++) {
        json method;
        M_AddFunctionHelper(method, static_cast<const clang::FunctionDecl*>(*method), false);
        out["methods"].push_back(method);
    };

    const clang::DeclContext* DeclContext =
        static_cast<const clang::DeclContext*>(Decl);
    for (auto const *NextDecl : DeclContext->decls()) {
        auto const *method =
            llvm::dyn_cast<clang::FunctionTemplateDecl>(NextDecl);
        if (!method) continue;
        json method;

        M_AddTemplateHelper(method,
            static_cast<const clang::TemplateDecl*>(method));

        clang::NamedDecl* Template = method->getTemplatedDecl();
        M_AddFunctionHelper(method,
            static_cast<const clang::FunctionDecl*>(Template), true);

        out["methods"].push_back(method);
    }
}



/*
method: {
    access: public|protected|private
    name: string
    return: string
    params: [
        {
            type: string
            name: string
            index: number
        },
        ...
    ]
    consteval: bool
    constexpr: bool
    explicit: bool
    static: bool
    inline: bool
    virtual: bool
    volatile: bool
    const: bool
    pure: bool
    deleted: bool
    default: bool
}
*/
void
JsonASTVisitor::M_AddFunctionHelper(
    json& out,
    const clang::FunctionDecl* method,
    bool isTemplate) const
{
    switch (method->getAccess()) {
    case clang::AS_public:    out["access"] = "public ";    break;
    case clang::AS_protected: out["access"] = "protected "; break;
    case clang::AS_private:   out["access"] = "private ";   break;
    case clang::AS_none:      out["access"] = "private ";   break;
    }

    out["name"] = method->getNameAsString();
    out["params"] = json::array();
    
    clang::QualType return_type = method->getReturnType();
    out["return"] = return_type.getAsString();

    out["consteval"] = method->isConsteval();
    out["explicit"]  = !method->isImplicit() &&
        method->getKind() == clang::Decl::Kind::CXXConstructor);
    out["constexpr"] = method->isConstexpr();
    out["static"]    = method->isStatic();
    out["inline"]    = method->isInlined();
    if (!isTemplate) {
        const auto* mmethod = static_cast<const clang::CXXMethodDecl*>(method);
        out["virtual"]   = mmethod->isVirtual();
        out["volatile"]  = mmethod->isVolatile();
        out["const"]     = mmethod->isConst();
    }
    out["pure"]    = method->isPure();
    out["deleted"] = method->isDeleted();
    out["default"] = method->isDefaulted();

    size_t index = 0;
    for (auto param = method->param_begin();
        param != method->param_end(); param++)
    {
        json jparam;
        jparam["type"] = (*param)->getType().getAsString();
        jparam["name"] = (*param)->getNameAsString();
        jparam["index"] = index;
        out["params"].push_back(jparam);
        index++;
    }
}


/*
root: {
    properties: [
        {
            access: public|protected|private
            type: string
            name: string
        },
        ...
    ]
}
*/
void
JsonASTVisitor::M_AddMemberProperties(
    json& out,
    const clang::CXXRecordDecl* Decl) const
{
    out["properties"] = json::array();

    bool has_fields = false;
    const clang::DeclContext* DeclContext =
        static_cast<const clang::DeclContext*>(Decl);

    for (auto const *NextDecl : DeclContext->decls()) {
        auto const *field =
            llvm::dyn_cast<clang::FieldDecl>(NextDecl);
        if (!field) continue;

        json field;

        switch (field->getAccess()) {
        case clang::AS_public:    field["access"] = "public";    break;
        case clang::AS_protected: field["access"] = "protected"; break;
        case clang::AS_private:   field["access"] = "private";   break;
        case clang::AS_none:      field["access"] = "private";   break;
        }

        field["type"] = field->getType().getAsString() << " ";
        field["name"] = field->getNameAsString();

        out["properties"].push_back(field);
    }
}
