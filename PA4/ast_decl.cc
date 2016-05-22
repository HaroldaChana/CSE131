/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "symtable.h"        
#include "irgen.h"
         
Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
    Assert(n != NULL);
    (id=n)->SetParent(this); 
}

llvm::Value* VarDecl::Emit() {
	// printf("Emitting VarDecl Node\n");
	llvm::Value* value = NULL;
	llvm::AllocaInst* inst = NULL;
	
	if(GetAssign() != NULL) {
		value = GetAssign()->Emit();
	}
	else{
		value = llvm::Constant::getNullValue(irgen->ast_llvm(GetType(), irgen->GetContext()));
	}
	
	llvm::Constant* constant = dynamic_cast<llvm::Constant*>(value);

	if(symtab->is_global()) {
		new llvm::GlobalVariable(*irgen->GetOrCreateModule("Program_Module.bc"), irgen->ast_llvm(GetType(), irgen->GetContext()), true, llvm::GlobalValue::ExternalLinkage, constant, id->GetName());
	}
	else {
		inst = new llvm::AllocaInst(irgen->ast_llvm(GetType(), irgen->GetContext()),id->GetName(), irgen->GetBasicBlock());
		
		// new llvm::StoreInst(value, , irgen->GetBasicBlock());
	}

	return inst;
}

VarDecl::VarDecl(Identifier *n, Type *t, Expr *e) : Decl(n) {
	Assert(n != NULL && t != NULL);
	(type=t)->SetParent(this);
	if (e) (assignTo=e)->SetParent(this);
	typeq = NULL;
}

VarDecl::VarDecl(Identifier *n, TypeQualifier *tq, Expr *e) : Decl(n) {
	Assert(n != NULL && tq != NULL);
	(typeq=tq)->SetParent(this);
	if (e) (assignTo=e)->SetParent(this);
	type = NULL;
}

VarDecl::VarDecl(Identifier *n, Type *t, TypeQualifier *tq, Expr *e) : Decl(n) {
	Assert(n != NULL && t != NULL && tq != NULL);
	(type=t)->SetParent(this);
    (typeq=tq)->SetParent(this);
    if (e) (assignTo=e)->SetParent(this);
}
  
void VarDecl::PrintChildren(int indentLevel) { 
   if (typeq) typeq->Print(indentLevel+1);
   if (type) type->Print(indentLevel+1);
   if (id) id->Print(indentLevel+1);
   if (assignTo) assignTo->Print(indentLevel+1, "(initializer) ");
}

llvm::Value* FnDecl::Emit() {
	std::vector<llvm::Type*> argTypes;

	for(int i = 0; i < formals->NumElements(); i++) {
		formals->Nth(i)->Emit();
	
		argTypes.push_back(irgen->ast_llvm(formals->Nth(i)->GetType(), irgen->GetContext()));
	}

	llvm::ArrayRef<llvm::Type*> argArray(argTypes);
	llvm::FunctionType* funcTy = llvm::FunctionType::get(irgen->ast_llvm(GetType(), irgen->GetContext()), argArray, false);

	llvm::Function *f = llvm::cast<llvm::Function>(irgen->GetOrCreateModule("Program_Module.bc")->getOrInsertFunction(GetIdentifier()->GetName(), funcTy));

	int i = 0;

	for(llvm::Function::arg_iterator args = f->arg_begin(); args != f->arg_end(); args++) {
		args->setName(formals->Nth(i)->GetIdentifier()->GetName());

		i++;
	}

	irgen->SetBasicBlock(llvm::BasicBlock::Create(*irgen->GetContext(), "Function"));

	body->Emit();

	return NULL;
}

FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
    returnTypeq = NULL;
}

FnDecl::FnDecl(Identifier *n, Type *r, TypeQualifier *rq, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r != NULL && rq != NULL&& d != NULL);
    (returnType=r)->SetParent(this);
    (returnTypeq=rq)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
}

void FnDecl::SetFunctionBody(Stmt *b) { 
    (body=b)->SetParent(this);
}

void FnDecl::PrintChildren(int indentLevel) {
    if (returnType) returnType->Print(indentLevel+1, "(return type) ");
    if (id) id->Print(indentLevel+1);
    if (formals) formals->PrintAll(indentLevel+1, "(formals) ");
    if (body) body->Print(indentLevel+1, "(body) ");
}

