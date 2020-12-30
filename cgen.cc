
//**************************************************************
//
// Code generator SKELETON
//
//
//**************************************************************

#include "cgen.h"
#include "cgen_gc.h"
#include <vector>

using namespace std;

extern void emit_string_constant(ostream& str, char *s);
extern int cgen_debug;

static char *CALL_REGS[] = {RDI, RSI, RDX, RCX, R8, R9};
static char *CALL_XMM[] = {XMM0, XMM1, XMM2, XMM3};

typedef SymbolTable<Symbol, int> ObjectEnvironment;
static ObjectEnvironment varNameToAddr;
static vector<char*> name_proc;

void cgen_helper(Decls decls, ostream& s);
void code(Decls decls, ostream& s);

//////////////////////////////////////////////////////////////////
//
//
//    Helper Functions
//  
//
//////////////////////////////////////////////////////////////////

// you can add any helper functions here


//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
Symbol 
    Int,
    Float,
    String,
    Bool,
    Void,
    Main,
    print
    ;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
    // 4 basic types and Void type
    Bool        = idtable.add_string("Bool");
    Int         = idtable.add_string("Int");
    String      = idtable.add_string("String");
    Float       = idtable.add_string("Float");
    Void        = idtable.add_string("Void");  
    // main function
    Main        = idtable.add_string("main");

    // classical function to print things, so defined here for call.
    print        = idtable.add_string("printf");
}


//*********************************************************
//
// Define method for code generation
//
//
//*********************************************************

void Program_class::cgen(ostream &os) 
{
  // spim wants comments to start with '#'
  os << "# start of generated code\n";

  initialize_constants();
  cgen_helper(decls,os);

  os << "\n# end of generated code\n";
}


//////////////////////////////////////////////////////////////////////////////
//
//  emit_* procedures
//
//  emit_X  writes code for operation "X" to the output stream.
//  There is an emit_X for each opcode X, as well as emit_ functions
//  for generating names according to the naming conventions (see emit.h)
//  and calls to support functions defined in the trap handler.
//
//  Register names and addresses are passed as strings.  See `emit.h'
//  for symbolic names you can use to refer to the strings.
//
//////////////////////////////////////////////////////////////////////////////

static void emit_mov(const char *source, const char *dest, ostream& s)
{
  s << MOV << source << COMMA << dest << endl;
}

static void emit_rmmov(const char *source_reg, int offset, const char *base_reg, ostream& s)
{
  s << MOV << source_reg << COMMA << offset << "(" << base_reg << ")"
      << endl;
}

static void emit_mrmov(const char *base_reg, int offset, const char *dest_reg, ostream& s)
{
  s << MOV << offset << "(" << base_reg << ")" << COMMA << dest_reg  
      << endl;
}

static void emit_irmov(const char *immidiate, const char *dest_reg, ostream& s)
{
  s << MOV << "$" << immidiate << COMMA << dest_reg  
      << endl;
}

static void emit_irmovl(const char *immidiate, const char *dest_reg, ostream& s)
{
  s << MOVL << "$" << immidiate << COMMA << dest_reg  
      << endl;
}

static void emit_immov(const char *immidiate, int offset, const char *base_reg, ostream& s)
{
  s << MOV << "$" << immidiate << COMMA << "(" << offset << ")" << base_reg  
      << endl;
}

static void emit_add(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << ADD << source_reg << COMMA << dest_reg << endl;
}

static void emit_sub(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << SUB << source_reg << COMMA << dest_reg << endl;
}

static void emit_mul(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << MUL << source_reg << COMMA << dest_reg << endl;
}

static void emit_div(const char *dest_reg, ostream& s)
{
  s << DIV << dest_reg << endl;
}

static void emit_cqto(ostream &s)
{
  s << CQTO << endl;
}

static void emit_neg(const char *dest_reg, ostream& s)
{
  s << NEG << dest_reg << endl;
}

static void emit_and(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << AND << source_reg << COMMA << dest_reg << endl;
}

static void emit_or(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << OR << source_reg << COMMA << dest_reg << endl;
}

static void emit_xor(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << XOR << source_reg << COMMA << dest_reg << endl;
}

static void emit_not(const char *dest_reg, ostream& s)
{
  s << NOT << " " << dest_reg << endl;
}

static void emit_movsd(const char *source, const char *dest, ostream& s)
{
  s << MOVSD << source << COMMA << dest << endl;
}

static void emit_movaps(const char *source, const char *dest, ostream& s)
{
  s << MOVAPS << source << COMMA << dest << endl;
}

static void emit_addsd(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << ADDSD << source_reg << COMMA << dest_reg << endl;
}

static void emit_subsd(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << SUBSD << source_reg << COMMA << dest_reg << endl;
}

static void emit_mulsd(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << MULSD << source_reg << COMMA << dest_reg << endl;
}

static void emit_divsd(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << DIVSD << source_reg << COMMA << dest_reg << endl;
}

static void emit_cmp(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << CMP << source_reg << COMMA << dest_reg << endl;
}

static void emit_test(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << TEST << source_reg << COMMA << dest_reg << endl;
}

static void emit_ucompisd(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << UCOMPISD << source_reg << COMMA << dest_reg << endl;
}

static void emit_xorpd(const char *source_reg, const char *dest_reg, ostream& s)
{
  s << XORPD << source_reg << COMMA << dest_reg << endl;
}

static void emit_jmp(const char *dest, ostream& s)
{
  s << JMP << " " << dest << endl;
}

static void emit_jl(const char *dest, ostream& s)
{
  s << JL << " " << dest << endl;
}

static void emit_jle(const char *dest, ostream& s)
{
  s << JLE << " " << dest << endl;
}

static void emit_je(const char *dest, ostream& s)
{
  s << JE << " " << dest << endl;
}

static void emit_jne(const char *dest, ostream& s)
{
  s << JNE << " " << dest << endl;
}

static void emit_jg(const char *dest, ostream& s)
{
  s << JG << " " << dest << endl;
}

static void emit_jge(const char *dest, ostream& s)
{
  s << JGE << " " << dest << endl;
}

static void emit_jb(const char *dest, ostream& s)
{
  s << JB << " " << dest << endl;
}

static void emit_jbe(const char *dest, ostream& s)
{
  s << JBE << " " << dest << endl;
}

static void emit_ja(const char *dest, ostream& s)
{
  s << JA << " " << dest << endl;
}

static void emit_jae(const char *dest, ostream& s)
{
  s << JAE << " " << dest << endl;
}

static void emit_jp(const char *dest, ostream& s)
{
  s << JP << " " << dest << endl;
}

static void emit_jz(const char *dest, ostream& s)
{
  s << JZ << " " << dest << endl;
}

static void emit_jnz(const char *dest, ostream& s)
{
  s << JNZ << " " << dest << endl;
}

static void emit_call(const char *dest, ostream& s)
{
  s << CALL << " " << dest << endl;
}

static void emit_ret(ostream& s)
{
  s << RET << endl;
}

static void emit_push(const char *reg, ostream& s)
{
  s << PUSH << " " << reg << endl;
}

static void emit_pop(const char *reg, ostream& s)
{
  s << POP << " " << reg << endl;
}

static void emit_leave(ostream& s)
{
  s << LEAVE << endl;
}

static void emit_position(const char *p, ostream& s)
{
  s << p << ":" << endl;
}

static void emit_float_to_int(const char *float_mmx, const char *int_reg, ostream& s)
{
  s << CVTTSD2SIQ << float_mmx << COMMA << int_reg << endl;
}

static void emit_int_to_float(const char *int_reg, const char *float_mmx, ostream& s)
{
  s << CVTSI2SDQ << int_reg << COMMA << float_mmx << endl;
}
///////////////////////////////////////////////////////////////////////////////
//
// coding strings, ints, and booleans
//
// Seal has four kinds of constants: strings, ints, and booleans.
// This section defines code generation for each type.
//
// If you like, you can add any ***Entry::code_def() and ***Entry::code_ref()
// functions to help.
//
///////////////////////////////////////////////////////////////////////////////

//
// Strings
//
void StringEntry::code_ref(ostream& s)
{
  s << "$" << STRINGCONST_PREFIX << index;
}

//
// Emit code for a constant String.
//

void StringEntry::code_def(ostream& s)
{
  s << STRINGCONST_PREFIX << index << ":" << endl;
  s  << STRINGTAG ; emit_string_constant(s,str);                                                // align to word
}

//
// StrTable::code_string
// Generate a string object definition for every string constant in the 
// stringtable.
//
void StrTable::code_string_table(ostream& s)
{  
  for (List<StringEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s);
}

// the following 2 functions are useless, please DO NOT care about them
void FloatEntry::code_ref(ostream &s)
{
  s << FLOATTAG << index;
}

void IntEntry::code_def(ostream &s)
{
  s << GLOBAL;
}

//***************************************************
//
//  Emit global var and functions.
//
//***************************************************

static void emit_global_int(Symbol name, ostream& s) {
  s << GLOBAL << name << endl << 
  ALIGN << 8 << endl << 
  SYMBOL_TYPE << name << COMMA << OBJECT << endl <<
  SIZE << name << COMMA << 8 << endl << 
  name << ":" << endl << 
  INTTAG << 0 << endl;
}

static void emit_global_float(Symbol name, ostream& s) {
  s << GLOBAL << name << endl << 
  ALIGN << 8 << endl << 
  SYMBOL_TYPE << name << COMMA << OBJECT << endl <<
  SIZE << name << COMMA << 8 << endl << 
  name << ":" << endl <<
  FLOATTAG << 0 << endl <<
  FLOATTAG << 0 << endl;
}

static void emit_global_bool(Symbol name, ostream& s) {
  s << GLOBAL << name << endl << 
  ALIGN << 8 << endl << 
  SYMBOL_TYPE << name << COMMA << OBJECT << endl <<
  SIZE << name << COMMA << 8 << endl << 
  name << ":" << endl << 
  BOOLTAG << 0 << endl;
}

static bool sameType(Symbol name1, Symbol name2) {
    return strcmp(name1->get_string(), name2->get_string()) == 0;
}

int count_len_addr_reg_shift(const char *const reg, const int deviation) {
    if (deviation == 0) {
        return strlen(reg);
    }
    else if (deviation < 0) return -1;
    int tmp = deviation;
    int digit = 0;
    while (tmp != 0){
        ++digit;
        tmp /= 10;
    }
    return strlen(reg) + digit + 4;
}

static void addr_reg_shift(char* res, const char *const reg, const int deviation){
    if (deviation == 0){
        strcpy(res, reg);
        return;
    }
    else if (deviation < 0) return;
    res[0] = '-';
    int digit = 0;
    int tmp = deviation;
    while (tmp != 0){
        ++digit;
        tmp /= 10;
    }
    char digit_[digit+1];
    sprintf(digit_, "%d", deviation);
    for (int i = 0; i < digit; i++) res[i+1] = digit_[i];
    res[digit+1] = '\0';
    res[digit+1] = '(';
    tmp = strlen(reg);
    for (int i = 0; i < tmp; ++i) res[digit+2+i] = reg[i];
    res[digit+strlen(reg)+2] = ')';
    res[digit+strlen(reg)+3] = '\0';
    return;
}

void code_global_data(Decls decls, ostream &str)
{
    for (int i = decls->first(); decls->more(i); i = decls->next(i)) {
        Decl tmp_decl = decls->nth(i);
        if (!tmp_decl->isCallDecl()) {
            VariableDecl variableDecl = static_cast<VariableDecl>(tmp_decl);
            Symbol type_tmp = variableDecl->getType();
            if (sameType(type_tmp, Int)){
                emit_global_int(variableDecl->getName(), str);
            }
            else if (sameType(type_tmp, Float)){
                emit_global_float(variableDecl->getName(), str);
            }
            else if (sameType(type_tmp, Bool)){
                emit_global_bool(variableDecl->getName(), str);
            }
            // add to scope
            varNameToAddr.addid(variableDecl->getName(), new int(name_proc.size()));
            int len = strlen(variableDecl->getName()->get_string()) + 7;
            name_proc.push_back(new char[len]);
            strcpy(name_proc[name_proc.size()-1], variableDecl->getName()->get_string());
            name_proc[name_proc.size()-1][strlen(variableDecl->getName()->get_string())+6] = '\0';
            name_proc[name_proc.size()-1][strlen(variableDecl->getName()->get_string())+5] = ')';
            name_proc[name_proc.size()-1][strlen(variableDecl->getName()->get_string())+4] = 'p';
            name_proc[name_proc.size()-1][strlen(variableDecl->getName()->get_string())+3] = 'i';
            name_proc[name_proc.size()-1][strlen(variableDecl->getName()->get_string())+2] = 'r';
            name_proc[name_proc.size()-1][strlen(variableDecl->getName()->get_string())+1] = '%';
            name_proc[name_proc.size()-1][strlen(variableDecl->getName()->get_string())] = '(';
            //            variableDecl->code(str); Note that this function is for temporary variableDecls in callDecl.
        }
    }
}

void code_calls(Decls decls, ostream &str) {
    str << SECTION << RODATA << endl;

    for (int i = decls->first(); decls->more(i); i = decls->next(i)) {
        Decl tmp_decl = decls->nth(i);
        if (tmp_decl->isCallDecl()) {
            CallDecl call = static_cast<CallDecl>(tmp_decl);
            call->code(str);
        }
    }
}

//***************************************************
//
//  Emit code to start the .text segment and to
//  declare the global names.
//
//***************************************************



//********************************************************
//
// Cgen helper helps to initialize and call code() function.
// You can do any initializing operations here
//
//********************************************************

void cgen_helper(Decls decls, ostream& s)
{
    code(decls, s);
}


void code(Decls decls, ostream& s)
{
    cgen_debug=1;
    varNameToAddr.enterscope();
    if (cgen_debug) cout << "Coding global data\n";
    code_global_data(decls, s);

    if (cgen_debug) cout << "Coding calls\n";
    code_calls(decls, s);
    varNameToAddr.exitscope();
}

//******************************************************************
//
//   Fill in the following methods to produce code for the
//   appropriate expression.  You may add or remove parameters
//   as you wish, but if you do, remember to change the parameters
//   of the declarations in `seal-decl.h', `seal-expr.h' and `seal-stmt.h'
//   Sample code for constant integers, strings, and booleans are provided.
//   
//*****************************************************************

void CallDecl_class::code(ostream &s) {
    if (cgen_debug) cout << "--- CallDecl_class::code :: name " << name->get_string() << " ---\n";
    varNameToAddr.enterscope();

    // Header part
    s << GLOBAL << name << endl <<
    SYMBOL_TYPE << name << ", " << FUNCTION << endl <<
    name << ':' << endl;

    // save workspace first
    //    TODO:caller to protect R10, R11
    emit_push(RBP, s);
    emit_mov(RSP, RBP, s);
    emit_push(RBX, s);
    emit_push(R12, s);
    emit_push(R13, s);
    emit_push(R14, s);
    emit_push(R15, s);
    int usage = 40;

    Variables params = getVariables();
    for (int i = params->first(); params->more(i); i = params->next(i)){
        // new stack piece
        emit_sub("$8", RSP, s);
        // move param to sub stack
        usage += 8;
        int len = count_len_addr_reg_shift(CALL_REGS[i], usage);
        char reg[len];
        addr_reg_shift(reg, CALL_REGS[i], usage);
        emit_mov(CALL_REGS[i], reg,s);
        // add to scope TODO
        varNameToAddr.addid(params->nth(i)->getName(), new int(name_proc.size()));
        char *c = new char[len];
        strcpy(c, reg);
        name_proc.push_back(c);
    }
    // check body TODO
    getBody()->code(s);

//not this problem zhuang tai ji cun qi mei bao cun
    // restore previous workspace
    emit_pop(R15, s);
    emit_pop(R14, s);
    emit_pop(R13, s);
    emit_pop(R12, s);
    emit_pop(RBX, s);

    // return
    emit_leave(s);
    emit_ret(s);
    s << SIZE << name << COMMA << ".-" << name <<endl;

    varNameToAddr.exitscope();
    if (cgen_debug) cout << "--- CallDecl_class::code :: name " << name->get_string() << " ---\n";
}

void StmtBlock_class::code(ostream &s){
    if (cgen_debug) cout << "--- StmtBlock_class::code " << " ---\n";
    varNameToAddr.enterscope();

    VariableDecls localVarDecls = getVariableDecls();
    for (int i = localVarDecls->first(); localVarDecls->more(i); i = localVarDecls->next(i)) {
        VariableDecl localVarDecl = localVarDecls->nth(i);
        localVarDecl->code(s);
    }
    Stmts localStmts = getStmts();
    Stmt localStmt;
    for (int i = localStmts->first(); localStmts->more(i); i = localStmts->next(i)) {
        localStmt = localStmts->nth(i);
        localStmt->code(s);
    }

    varNameToAddr.exitscope();
    if (cgen_debug) cout << "--- StmtBlock_class::code " << " ---\n";
}

void IfStmt_class::code(ostream &s) {

}

void WhileStmt_class::code(ostream &s) {

}

void ForStmt_class::code(ostream &s) {

}

void ReturnStmt_class::code(ostream &s) {

}

void ContinueStmt_class::code(ostream &s) {

}

void BreakStmt_class::code(ostream &s) {
}

void Call_class::code(ostream &s) {
  
  //
  /*
   if function name is printf

    // please set %eax to the number of Float parameters, num.
    //  把%eax赋值为Float类型的参数个数, num
    emit_sub("$8", RSP, s);
    emit_irmovl(num, EAX, s);
    emit_call("printf", s);
      
    return;
  }
  */
  //
}

void Actual_class::code(ostream &s) {

}

void Assign_class::code(ostream &s) {

}

void Add_class::code(ostream &s) {

}

void Minus_class::code(ostream &s) {

}

void Multi_class::code(ostream &s) {

}

void Divide_class::code(ostream &s) {

}

void Mod_class::code(ostream &s) {

}

void Neg_class::code(ostream &s) {

}

void Lt_class::code(ostream &s) {

}

void Le_class::code(ostream &s) {

}

void Equ_class::code(ostream &s) {

}

void Neq_class::code(ostream &s) {

}

void Ge_class::code(ostream &s) {

}

void Gt_class::code(ostream &s) {

}

void And_class::code(ostream &s) {

}

void Or_class::code(ostream &s) {

}

void Xor_class::code(ostream &s) {

}

void Not_class::code(ostream &s) {

}

void Bitnot_class::code(ostream &s) {

}

void Bitand_class::code(ostream &s) {

}

void Bitor_class::code(ostream &s) {

}

void Const_int_class::code(ostream &s) {

}

void Const_string_class::code(ostream &s) {

}

void Const_float_class::code(ostream &s) {

}

void Const_bool_class::code(ostream &s) {

}

void Object_class::code(ostream &s) {

}

void No_expr_class::code(ostream &s) {

}