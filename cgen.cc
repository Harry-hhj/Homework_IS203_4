
//**************************************************************
//
// Code generator SKELETON
//
//
//**************************************************************

#include "cgen.h"
#include "cgen_gc.h"
#include <vector>
#include <stack>
#include <cmath>

using namespace std;

extern void emit_string_constant(ostream &str, char *s);

extern int cgen_debug;

static char *CALL_REGS[] = {RDI, RSI, RDX, RCX, R8, R9};
static char *CALL_XMM[] = {XMM0, XMM1, XMM2, XMM3};

typedef SymbolTable<Symbol, int> ObjectEnvironment;
static ObjectEnvironment varNameToAddr; // with the help of name_proc, Symbol -> char[] related with addr
static vector<char *> name_proc;  // assist the varNameToAddr
static int curr_usage = 0;  // indicate the height between rsp and rbp
static int pos_available = 0;  // indicate which .POSX: is available
static stack<char const *> operandStack;  // TODO: some of its content cannot be cleared e.g. a+b;
static bool init_once = true;
struct LOOP {
    char const *back;
    char const *next;

    LOOP(char *b, char *n){
        back = b; next = n;
    }

    ~LOOP() {
        delete [] back;
        delete [] next;
    }
};  // restore the begining and next POS
static stack<LOOP *> LOOP_MSG;

void cgen_helper(Decls decls, ostream &s);

void code(Decls decls, ostream &s);

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
        print;

//
// Initializing the predefined symbols.
//
static void initialize_constants(void) {
    // 4 basic types and Void type
    Bool = idtable.add_string("Bool");
    Int = idtable.add_string("Int");
    String = idtable.add_string("String");
    Float = idtable.add_string("Float");
    Void = idtable.add_string("Void");
    // main function
    Main = idtable.add_string("main");

    // classical function to print things, so defined here for call.
    print = idtable.add_string("printf");
}


//*********************************************************
//
// Define method for code generation
//
//
//*********************************************************

void Program_class::cgen(ostream &os) {
    // spim wants comments to start with '#'
    os << "# start of generated code\n";

    initialize_constants();
    cgen_helper(decls, os);

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

static void emit_mov(const char *source, const char *dest, ostream &s) {
    s << MOV << source << COMMA << dest << endl;
}

static void emit_rmmov(const char *source_reg, int offset, const char *base_reg, ostream &s) {
    s << MOV << source_reg << COMMA << offset << "(" << base_reg << ")"
      << endl;
}

static void emit_mrmov(const char *base_reg, int offset, const char *dest_reg, ostream &s) {
    s << MOV << offset << "(" << base_reg << ")" << COMMA << dest_reg
      << endl;
}

static void emit_irmov(const char *immidiate, const char *dest_reg, ostream &s) {
    s << MOV << "$" << immidiate << COMMA << dest_reg
      << endl;
}

static void emit_irmovl(const char *immidiate, const char *dest_reg, ostream &s) {
    s << MOVL << "$" << immidiate << COMMA << dest_reg
      << endl;
}

static void emit_immov(const char *immidiate, int offset, const char *base_reg, ostream &s) {
    s << MOV << "$" << immidiate << COMMA << "(" << offset << ")" << base_reg
      << endl;
}

static void emit_add(const char *source_reg, const char *dest_reg, ostream &s) {
    s << ADD << source_reg << COMMA << dest_reg << endl;
}

static void emit_sub(const char *source_reg, const char *dest_reg, ostream &s) {
    s << SUB << source_reg << COMMA << dest_reg << endl;
}

static void emit_mul(const char *source_reg, const char *dest_reg, ostream &s) {
    s << MUL << source_reg << COMMA << dest_reg << endl;
}

static void emit_div(const char *dest_reg, ostream &s) {
    s << DIV << dest_reg << endl;
}

static void emit_cqto(ostream &s) {
    s << CQTO << endl;
}

static void emit_neg(const char *dest_reg, ostream &s) {
    s << NEG << dest_reg << endl;
}

static void emit_and(const char *source_reg, const char *dest_reg, ostream &s) {
    s << AND << source_reg << COMMA << dest_reg << endl;
}

static void emit_or(const char *source_reg, const char *dest_reg, ostream &s) {
    s << OR << source_reg << COMMA << dest_reg << endl;
}

static void emit_xor(const char *source_reg, const char *dest_reg, ostream &s) {
    s << XOR << source_reg << COMMA << dest_reg << endl;
}

static void emit_not(const char *dest_reg, ostream &s) {
    s << NOT << " " << dest_reg << endl;
}

static void emit_movsd(const char *source, const char *dest, ostream &s) {
    s << MOVSD << source << COMMA << dest << endl;
}

static void emit_movaps(const char *source, const char *dest, ostream &s) {
    s << MOVAPS << source << COMMA << dest << endl;
}

static void emit_addsd(const char *source_reg, const char *dest_reg, ostream &s) {
    s << ADDSD << source_reg << COMMA << dest_reg << endl;
}

static void emit_subsd(const char *source_reg, const char *dest_reg, ostream &s) {
    s << SUBSD << source_reg << COMMA << dest_reg << endl;
}

static void emit_mulsd(const char *source_reg, const char *dest_reg, ostream &s) {
    s << MULSD << source_reg << COMMA << dest_reg << endl;
}

static void emit_divsd(const char *source_reg, const char *dest_reg, ostream &s) {
    s << DIVSD << source_reg << COMMA << dest_reg << endl;
}

static void emit_cmp(const char *source_reg, const char *dest_reg, ostream &s) {
    s << CMP << source_reg << COMMA << dest_reg << endl;
}

static void emit_test(const char *source_reg, const char *dest_reg, ostream &s) {
    s << TEST << source_reg << COMMA << dest_reg << endl;
}

static void emit_ucompisd(const char *source_reg, const char *dest_reg, ostream &s) {
    s << UCOMPISD << source_reg << COMMA << dest_reg << endl;
}

static void emit_xorpd(const char *source_reg, const char *dest_reg, ostream &s) {
    s << XORPD << source_reg << COMMA << dest_reg << endl;
}

static void emit_jmp(const char *dest, ostream &s) {
    s << JMP << " " << dest << endl;
}

static void emit_jl(const char *dest, ostream &s) {
    s << JL << " " << dest << endl;
}

static void emit_jle(const char *dest, ostream &s) {
    s << JLE << " " << dest << endl;
}

static void emit_je(const char *dest, ostream &s) {
    s << JE << " " << dest << endl;
}

static void emit_jne(const char *dest, ostream &s) {
    s << JNE << " " << dest << endl;
}

static void emit_jg(const char *dest, ostream &s) {
    s << JG << " " << dest << endl;
}

static void emit_jge(const char *dest, ostream &s) {
    s << JGE << " " << dest << endl;
}

static void emit_jb(const char *dest, ostream &s) {
    s << JB << " " << dest << endl;
}

static void emit_jbe(const char *dest, ostream &s) {
    s << JBE << " " << dest << endl;
}

static void emit_ja(const char *dest, ostream &s) {
    s << JA << " " << dest << endl;
}

static void emit_jae(const char *dest, ostream &s) {
    s << JAE << " " << dest << endl;
}

static void emit_jp(const char *dest, ostream &s) {
    s << JP << " " << dest << endl;
}

static void emit_jz(const char *dest, ostream &s) {
    s << JZ << " " << dest << endl;
}

static void emit_jnz(const char *dest, ostream &s) {
    s << JNZ << " " << dest << endl;
}

static void emit_call(const char *dest, ostream &s) {
    s << CALL << " " << dest << endl;
}

static void emit_ret(ostream &s) {
    s << RET << endl;
}

static void emit_push(const char *reg, ostream &s) {
    s << PUSH << " " << reg << endl;
}

static void emit_pop(const char *reg, ostream &s) {
    s << POP << " " << reg << endl;
}

static void emit_leave(ostream &s) {
    s << LEAVE << endl;
}

static void emit_position(const char *p, ostream &s) {
    s << p << ":" << endl;
}

static void emit_float_to_int(const char *float_mmx, const char *int_reg, ostream &s) {
    s << CVTTSD2SIQ << float_mmx << COMMA << int_reg << endl;
}

static void emit_int_to_float(const char *int_reg, const char *float_mmx, ostream &s) {
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
void StringEntry::code_ref(ostream &s) {
    s << "$" << STRINGCONST_PREFIX << index;
}

//
// Emit code for a constant String.
//

void StringEntry::code_def(ostream &s) {
    s << STRINGCONST_PREFIX << index << ":" << endl;
    s << STRINGTAG;
    emit_string_constant(s, str);                                                // align to word
}

//
// StrTable::code_string
// Generate a string object definition for every string constant in the 
// stringtable.
//
void StrTable::code_string_table(ostream &s) {
    for (List<StringEntry> *l = tbl; l; l = l->tl())
        l->hd()->code_def(s);
}

// the following 2 functions are useless, please DO NOT care about them
void FloatEntry::code_ref(ostream &s) {
    s << FLOATTAG << index;
}

void IntEntry::code_def(ostream &s) {
    s << GLOBAL;
}

//***************************************************
//
//  Emit global var and functions.
//
//***************************************************

static void emit_global_int(Symbol name, ostream &s) {
    s << GLOBAL << name << endl <<
      ALIGN << 8 << endl <<
      SYMBOL_TYPE << name << COMMA << OBJECT << endl <<
      SIZE << name << COMMA << 8 << endl <<
      name << ":" << endl <<
      INTTAG << 0 << endl;
}

static void emit_global_float(Symbol name, ostream &s) {
    s << GLOBAL << name << endl <<
      ALIGN << 8 << endl <<
      SYMBOL_TYPE << name << COMMA << OBJECT << endl <<
      SIZE << name << COMMA << 8 << endl <<
      name << ":" << endl <<
      FLOATTAG << 0 << endl <<
      FLOATTAG << 0 << endl;
}

static void emit_global_bool(Symbol name, ostream &s) {
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
    } else if (deviation < 0) return -1;
    int tmp = deviation;
    int digit = 0;
    while (tmp != 0) {
        ++digit;
        tmp /= 10;
    }
    return strlen(reg) + digit + 4;
}

static void addr_reg_shift(char *res, const char *const reg, const int deviation) {
    if (deviation == 0) {
        strcpy(res, reg);
        return;
    } else if (deviation < 0) return;
    res[0] = '-';
    int digit = 0;
    int tmp = deviation;
    while (tmp != 0) {
        ++digit;
        tmp /= 10;
    }
    char digit_[digit + 1];
    sprintf(digit_, "%d", deviation);
    for (int i = 0; i < digit; i++) res[i + 1] = digit_[i];
    res[digit + 1] = '\0';
    res[digit + 1] = '(';
    tmp = strlen(reg);
    for (int i = 0; i < tmp; ++i) res[digit + 2 + i] = reg[i];
    res[digit + strlen(reg) + 2] = ')';
    res[digit + strlen(reg) + 3] = '\0';
    return;
}

void code_global_data(Decls decls, ostream &str) {
    init_once = true;
    for (int i = decls->first(); decls->more(i); i = decls->next(i)) {
        decls->nth(i)->code(str);
    }
    init_once = false;

    str << DATA << endl;
    for (int i = decls->first(); decls->more(i); i = decls->next(i)) {
        Decl tmp_decl = decls->nth(i);
        if (!tmp_decl->isCallDecl()) {
            VariableDecl variableDecl = static_cast<VariableDecl>(tmp_decl);
            Symbol type_tmp = variableDecl->getType();
            if (sameType(type_tmp, Int)) {
                emit_global_int(variableDecl->getName(), str);
            } else if (sameType(type_tmp, Float)) {
                emit_global_float(variableDecl->getName(), str);
            } else if (sameType(type_tmp, Bool)) {
                emit_global_bool(variableDecl->getName(), str);
            }
            // add to scope
            varNameToAddr.addid(variableDecl->getName(), new int(name_proc.size()));
            int len = strlen(variableDecl->getName()->get_string()) + 7;
            name_proc.push_back(new char[len]);
            strcpy(name_proc[name_proc.size() - 1], variableDecl->getName()->get_string());
            name_proc[name_proc.size() - 1][strlen(variableDecl->getName()->get_string()) + 6] = '\0';
            name_proc[name_proc.size() - 1][strlen(variableDecl->getName()->get_string()) + 5] = ')';
            name_proc[name_proc.size() - 1][strlen(variableDecl->getName()->get_string()) + 4] = 'p';
            name_proc[name_proc.size() - 1][strlen(variableDecl->getName()->get_string()) + 3] = 'i';
            name_proc[name_proc.size() - 1][strlen(variableDecl->getName()->get_string()) + 2] = 'r';
            name_proc[name_proc.size() - 1][strlen(variableDecl->getName()->get_string()) + 1] = '%';
            name_proc[name_proc.size() - 1][strlen(variableDecl->getName()->get_string())] = '(';
            //            variableDecl->code(str); Note that this function is for temporary variableDecls in callDecl.
        }
    }

    str << SECTION << RODATA << endl;
    // code string
    stringtable.code_string_table(str);
}

void code_calls(Decls decls, ostream &str) {
    str << TEXT << endl;
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

void cgen_helper(Decls decls, ostream &s) {
    code(decls, s);
}


void code(Decls decls, ostream &s) {
    cgen_debug = 0;
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
    if (init_once) {
        getBody()->code(s);
        return;
    }
    if (cgen_debug) cout << "--- CallDecl_class::code :: name " << name->get_string() << " ---\n";
    varNameToAddr.enterscope();

    // Header part
    s << GLOBAL << name << endl <<
      SYMBOL_TYPE << name << ", " << FUNCTION << endl <<
      name << ':' << endl;

    // save workspace first
    //TODO:caller to protect R10, R11
    emit_push(RBP, s);
    emit_mov(RSP, RBP, s);
    emit_push(RBX, s);
    emit_push(R12, s);
    emit_push(R13, s);
    emit_push(R14, s);
    emit_push(R15, s);
    if (name->get_string() == Main->get_string()) curr_usage = 0;
    else curr_usage = 40;


    Variables params = getVariables();
    int int_num = 0;
    int float_num = 0;
    for (int i = params->first(); params->more(i); i = params->next(i)) {
        // new stack piece
        emit_sub("$8", RSP, s);
        // move param to sub stack
        curr_usage += 8;
        int len = count_len_addr_reg_shift(RBP, curr_usage);
        char reg[len];
        addr_reg_shift(reg, RBP, curr_usage);
        if (sameType(params->nth(i)->getType(), Float)) emit_movsd(CALL_XMM[float_num++], reg, s);
        else emit_mov(CALL_REGS[int_num++], reg, s);
        // add to scope
        varNameToAddr.addid(params->nth(i)->getName(), new int(name_proc.size()));
        char *c = new char[len];
        strcpy(c, reg);
        name_proc.push_back(c);
    }
    // check body TODO
    getBody()->code(s);

    // after return
    s << SIZE << name << COMMA << ".-" << name << endl;

    varNameToAddr.exitscope();
    if (cgen_debug) cout << "--- CallDecl_class::code :: name " << name->get_string() << " ---\n";
}

void StmtBlock_class::code(ostream &s) {
    if (init_once) {
        Stmts localStmts = getStmts();
        for (int i = localStmts->first(); localStmts->more(i); i = localStmts->next(i))
            localStmts->nth(i)->code(s);
        return;
    }
    if (cgen_debug) cout << "--- StmtBlock_class::code " << " ---\n";
    varNameToAddr.enterscope();

    VariableDecls localVarDecls = getVariableDecls();
    for (int i = localVarDecls->first(); localVarDecls->more(i); i = localVarDecls->next(i)) {
        // new stack piece
        emit_sub("$8", RSP, s);
        // move param to sub stack
        curr_usage += 8;
        int len = count_len_addr_reg_shift(RBP, curr_usage);
        char reg[len];
        addr_reg_shift(reg, RBP, curr_usage);
        // add to scope
        varNameToAddr.addid(localVarDecls->nth(i)->getName(), new int(name_proc.size()));
        char *c = new char[len];
        strcpy(c, reg);
        name_proc.push_back(c);
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
    if (init_once) {
        getCondition()->code(s);
        getThen()->code(s);
        getElse()->code(s);
        return;
    }
    if (cgen_debug) cout << "--- IfStmt_class::code " << " ---\n";

    // get 2 POS name
    int pos_usable = pos_available; // pos_usable: else, pos_usable+1: basic basic after ifstmt
    pos_available += 2;
    getCondition()->code(s);
    const char *c = operandStack.top();
    operandStack.pop();
    emit_mov(c, RAX, s);
    delete c;
    emit_test(RAX, RAX, s);
    int tmp = pos_usable;
    int digit = 0;
    if (tmp == 0) digit = 1;
    else
        while (tmp != 0) {
            ++digit;
            tmp /= 10;
        }
    char *pos_char = new char[digit + 6];
    sprintf(pos_char, "%s%d", POSITION, pos_usable);
    emit_jz(pos_char, s);
    getThen()->code(s);
    tmp = pos_usable + 1;
    digit = 0;
    if (tmp == 0) digit = 1;
    else
        while (tmp != 0) {
            ++digit;
            tmp /= 10;
        }
    char *pos_char1 = new char[digit + 6];
    sprintf(pos_char1, "%s%d", POSITION, pos_usable + 1);
    emit_jmp(pos_char1, s);

    emit_position(pos_char, s);
    getElse()->code(s);

    emit_position(pos_char1, s);
    delete[]pos_char;
    delete[]pos_char1;

    if (cgen_debug) cout << "--- IfStmt_class::code " << " ---\n";
}

void WhileStmt_class::code(ostream &s) {
    if (init_once) {
        condition->code(s);
        body->code(s);
        return;
    }

    if (cgen_debug) cout << "--- WhileStmt_class::code " << " ---\n";

    // get 2 POS name
    int pos_usable = pos_available; // pos_usable: body; pos_usable+1: outside while
    pos_available += 2;
    int tmp = pos_usable;
    int digit = 0;
    if (tmp == 0) digit = 1;
    else
        while (tmp != 0) {
            ++digit;
            tmp /= 10;
        }
    char *pos_char = new char[digit + 6];
    sprintf(pos_char, "%s%d", POSITION, pos_usable);
    tmp = pos_usable + 1;
    digit = 0;
    if (tmp == 0) digit = 1;
    else
        while (tmp != 0) {
            ++digit;
            tmp /= 10;
        }
    char *pos_char1 = new char[digit + 6];
    sprintf(pos_char1, "%s%d", POSITION, pos_usable + 1);

    // add while message to LOOP_MSG
    char * msg1 = new char[strlen(pos_char)];
    strcpy(msg1, pos_char);
    char * msg2 = new char[strlen(pos_char1)];
    strcpy(msg2, pos_char1);
    LOOP *msg = new LOOP(msg1, msg2);
    LOOP_MSG.push(msg);

    // loop: check -> run -> back
    emit_position(pos_char, s);
    condition->code(s);
    const char *c = operandStack.top();
    operandStack.pop();
    emit_mov(c, RAX, s);
    emit_test(RAX, RAX, s);
    emit_jz(pos_char1, s);
    body->code(s);
    emit_jmp(pos_char, s);

    // next stmt
    emit_position(pos_char1, s);

    delete []pos_char;
    delete []pos_char1;

    if (cgen_debug) cout << "--- WhileStmt_class::code " << " ---\n";
}

void ForStmt_class::code(ostream &s) {
    if (init_once) {
        initexpr->code(s);
        condition->code(s);
        loopact->code(s);
        body->code(s);
        return;
    }

    if (cgen_debug) cout << "--- ForStmt_class::code " << " ---\n";

    initexpr->code(s);

    // get 3 POS name
    int pos_usable = pos_available; // pos_usable: condition; pos_usable+1: loopact; pos_usable+2: outside for
    pos_available += 3;
    int tmp = pos_usable;
    int digit = 0;
    if (tmp == 0) digit = 1;
    else
        while (tmp != 0) {
            ++digit;
            tmp /= 10;
        }
    char *pos_char = new char[digit + 6];
    sprintf(pos_char, "%s%d", POSITION, pos_usable);
    tmp = pos_usable + 1;
    digit = 0;
    if (tmp == 0) digit = 1;
    else
        while (tmp != 0) {
            ++digit;
            tmp /= 10;
        }
    char *pos_char1 = new char[digit + 6];
    sprintf(pos_char1, "%s%d", POSITION, pos_usable + 1);
    tmp = pos_usable + 1;
    digit = 0;
    if (tmp == 0) digit = 1;
    else
        while (tmp != 0) {
            ++digit;
            tmp /= 10;
        }
    char *pos_char2 = new char[digit + 6];
    sprintf(pos_char2, "%s%d", POSITION, pos_usable + 2);

    // add while message to LOOP_MSG
    char * msg1 = new char[strlen(pos_char)];
    strcpy(msg1, pos_char);
    char * msg2 = new char[strlen(pos_char2)];
    strcpy(msg2, pos_char1);
    LOOP *msg = new LOOP(msg1, msg2);
    LOOP_MSG.push(msg);

    emit_position(pos_char, s);
    condition->code(s);
    const char *c = operandStack.top();
    operandStack.pop();
    emit_mov(c, RAX, s);
    emit_test(RAX, RAX, s);
    emit_jz(pos_char2, s);
    body->code(s);

    emit_position(pos_char1, s);
    loopact->code(s);
    emit_jmp(pos_char, s);

    emit_position(pos_char2, s);

    delete []pos_char;
    delete []pos_char1;
    delete []pos_char2;

    if (cgen_debug) cout << "--- ForStmt_class::code " << " ---\n";
}

void ReturnStmt_class::code(ostream &s) {
    if (init_once) {
        value->code(s);
        return;
    }
    if (cgen_debug) cout << "--- ReturnStmt_class::code ---\n";

    // clear LOOP_MSG first
//    while (!LOOP_MSG.empty()) {
//        LOOP *l = LOOP_MSG.top();
//        LOOP_MSG.pop();
//        delete l;
//    }

    // put the result into %rax
    value->code(s);
    const char *c = operandStack.top();
    if (c != nullptr) {
        operandStack.pop();
        emit_mov(c, RAX, s);
        delete c;
    }

    // restore previous workspace
    emit_pop(R15, s);
    emit_pop(R14, s);
    emit_pop(R13, s);
    emit_pop(R12, s);
    emit_pop(RBX, s);
    // go back
    emit_leave(s);
    emit_ret(s);

    if (cgen_debug) cout << "--- ReturnStmt_class::code ---\n";
}

void ContinueStmt_class::code(ostream &s) {
    if (init_once) {
        return;
    }

    if (cgen_debug) cout << "--- ContinueStmt_class::code ---\n";

    LOOP *msg = LOOP_MSG.top();
//    LOOP_MSG.pop();
    emit_jmp(msg->back, s);
//    delete msg;

    if (cgen_debug) cout << "--- ContinueStmt_class::code ---\n";
}

void BreakStmt_class::code(ostream &s) {
    if (init_once) {
        return;
    }

    if (cgen_debug) cout << "--- BreakStmt_class::code ---\n";

    LOOP *msg = LOOP_MSG.top();
//    LOOP_MSG.pop();
    emit_jmp(msg->next, s);
//    delete msg;

    if (cgen_debug) cout << "--- BreakStmt_class::code ---\n";
}

void Call_class::code(ostream &s) {
    if (init_once) {
        for (int i = actuals->first(); actuals->more(i); i = actuals->next(i)) actuals->nth(i)->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Call_class::code ---\n";

    if (strcmp(name->get_string(), print->get_string())==0) {
        int int_num_tmp = 0;
        int float_num_tmp = 0;
        int float_num = 0;
        for (int i = actuals->first(); actuals->more(i); i = actuals->next(i)) {
            actuals->nth(i)->code(s);
            if (sameType(actuals->nth(i)->getType(), Float)) ++float_num;
        }
        stack<char const *> stk;
        for (int i = actuals->first(); actuals->more(i); i = actuals->next(i)) {
            const char *a = operandStack.top();
            operandStack.pop();
            stk.push(a);
        }
        for (int i = actuals->first(); actuals->more(i); i = actuals->next(i)) {
            const char *a = stk.top();
            stk.pop();
            if (sameType(actuals->nth(i)->getType(), Float)) {
                emit_movsd(a, CALL_XMM[float_num_tmp++], s);
            }
            else emit_mov(a, CALL_REGS[int_num_tmp++], s);
            delete [] a;
        }
        // get stack space ready for the result
        emit_sub("$8", RSP, s);
        curr_usage += 8;
        int res_addr = curr_usage;
        int tmp = float_num;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char ctmp[digit];
        sprintf(ctmp, "%d", float_num);
        emit_irmovl(ctmp, EAX, s);
        // store the workspace (caller reg)
        emit_push(R10, s);
        emit_push(R11, s);
        curr_usage += 16;
        // call
        emit_call(name->get_string(), s);
        // restore workspace
        emit_pop(R11, s);
        emit_pop(R10, s);
        curr_usage -= 16;
    }
    else {
        int int_num_tmp = 0;
        int float_num_tmp = 0;
        for (int i = actuals->first(); actuals->more(i); i = actuals->next(i)) {
            actuals->nth(i)->code(s);
        }
        stack<char const *> stk;
        for (int i = actuals->first(); actuals->more(i); i = actuals->next(i)) {
            const char *a = operandStack.top();
            operandStack.pop();
            stk.push(a);
        }
        for (int i = actuals->first(); actuals->more(i); i = actuals->next(i)) {
            const char *a = stk.top();
            stk.pop();
            if (sameType(actuals->nth(i)->getType(), Float)) emit_movsd(a, CALL_XMM[float_num_tmp++], s);
            else emit_mov(a, CALL_REGS[int_num_tmp++], s);
            delete [] a;
        }
        // store the workspace (caller reg)
        emit_push(R10, s);
        emit_push(R11, s);
        curr_usage += 16;
        // call
        emit_call(name->get_string(), s);
        // restore workspace
        emit_pop(R11, s);
        emit_pop(R10, s);
        curr_usage -= 16;
        if (!sameType(getType(), Void)) {
            // get stack space ready for the result
            emit_sub("$8", RSP, s);
            curr_usage += 8;
            int res_addr = curr_usage;
            int len = count_len_addr_reg_shift(RBP, res_addr);
            char reg[len];
            addr_reg_shift(reg, RBP, res_addr);
            // get the result
            emit_mov(RAX, reg, s);
            char *c = new char[len];
            strcpy(c, reg);
            operandStack.push(c);
        }
    }

    if (cgen_debug) cout << "--- Call_class::code ---\n";

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
    if (init_once) {
        expr->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Actual_class::code ---\n";

    expr->code(s);

    if (cgen_debug) cout << "--- Actual_class::code ---\n";
}

void Assign_class::code(ostream &s) {
    if (init_once) {
        value->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Assign_class::code ---\n";

    // TODO: unable to assign to global string var! no definition
    value->code(s);
    // assign value value_addr -> RAX -> lvalue_addr with the help of varNameToAddr
    const char *c = operandStack.top();
    operandStack.pop();
    emit_mov(c, RAX, s);
    delete c;
    int *idx = varNameToAddr.lookup(lvalue);
    const char *addr = name_proc[*idx];
    emit_mov(RAX, addr, s);

    // put result into the operandStack
    char *str = new char[strlen(addr)];
    strcpy(str, addr);
    operandStack.push(str);

    if (cgen_debug) cout << "--- Assign_class::code ---\n";
}

void Add_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        e2->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Add_class::code ---\n";

    // caution: their order
    e1->code(s);
    e2->code(s);

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    // case: both Int
    if (sameType(e1->getType(), Int) && sameType(e2->getType(), Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, R12, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RBX, s);
        delete c;
        emit_add(RBX, R12, s);
        // store result
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(R12, reg, s);

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: both Float
    else if (sameType(e1->getType(), Float) && sameType(e2->getType(), Float)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM5, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM4, s);
        delete c;
        emit_addsd(XMM4, XMM5, s);
        // store result
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_movsd(XMM5, reg, s);

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Int, Float
    else if (sameType(e1->getType(), Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM5, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RBX, s);
        emit_int_to_float(RBX, XMM4, s);
        delete c;
        emit_addsd(XMM4, XMM5, s);
        // store result
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_movsd(XMM5, reg, s);

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Float, Int
    else {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RBX, s);
        emit_int_to_float(RBX, XMM5, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM4, s);
        delete c;
        emit_addsd(XMM4, XMM5, s);
        // store result
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_movsd(XMM5, reg, s);

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }

    if (cgen_debug) cout << "--- Add_class::code ---\n";
}

void Minus_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        e2->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Minus_class::code ---\n";

    // caution: their order
    e1->code(s);
    e2->code(s);

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    // case: both Int
    if (sameType(e1->getType(), Int) && sameType(e2->getType(), Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, R12, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RBX, s);
        delete c;
        emit_sub(R12, RBX, s);
        // store result
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RBX, reg, s);

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: both Float
    else if (sameType(e1->getType(), Float) && sameType(e2->getType(), Float)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM5, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM4, s);
        delete c;
        emit_subsd(XMM4, XMM5, s);
        // store result
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_movsd(XMM5, reg, s);

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Int, Float
    else if (sameType(e1->getType(), Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM5, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RBX, s);
        emit_int_to_float(RBX, XMM4, s);
        delete c;
        emit_subsd(XMM4, XMM5, s);
        // store result
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_movsd(XMM5, reg, s);

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Float, Int
    else {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RBX, s);
        emit_int_to_float(RBX, XMM5, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM4, s);
        delete c;
        emit_subsd(XMM5, XMM4, s);
        // store result
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_movsd(XMM4, reg, s);

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }

    if (cgen_debug) cout << "--- Minus_class::code ---\n";
}

void Multi_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        e2->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Multi_class::code ---\n";

    // caution: their order
    e1->code(s);
    e2->code(s);

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    // case: both Int
    if (sameType(e1->getType(), Int) && sameType(e2->getType(), Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, R12, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RBX, s);
        delete c;
        emit_mul(R12, RBX, s);
        // store result
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RBX, reg, s);

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: both Float
    else if (sameType(e1->getType(), Float) && sameType(e2->getType(), Float)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM5, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM4, s);
        delete c;
        emit_mulsd(XMM5, XMM4, s);
        // store result
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_movsd(XMM4, reg, s);

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Int, Float
    else if (sameType(e1->getType(), Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM5, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RBX, s);
        emit_int_to_float(RBX, XMM4, s);
        delete c;
        emit_mulsd(XMM5, XMM4, s);
        // store result
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_movsd(XMM4, reg, s);

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Float, Int
    else {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RBX, s);
        emit_int_to_float(RBX, XMM5, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM4, s);
        delete c;
        emit_mulsd(XMM5, XMM4, s);
        // store result
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_movsd(XMM4, reg, s);

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }

    if (cgen_debug) cout << "--- Multi_class::code ---\n";
}

void Divide_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        e2->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Divide_class::code ---\n";

    // caution: their order
    e1->code(s);
    e2->code(s);

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    // case: both Int
    if (sameType(e1->getType(), Int) && sameType(e2->getType(), Int)) {
        const char *c1 = operandStack.top();
        operandStack.pop();
        const char *c2 = operandStack.top();
        operandStack.pop();
        emit_mov(c2, RAX, s);
        emit_cqto(s);
        emit_mov(c1, RBX, s);
        delete c1;
        delete c2;
        emit_div(RBX, s);
        // store result
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: both Float
    else if (sameType(e1->getType(), Float) && sameType(e2->getType(), Float)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM5, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM4, s);
        delete c;
        emit_divsd(XMM5, XMM4, s);
        // store result
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_movsd(XMM4, reg, s);

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Int, Float
    else if (sameType(e1->getType(), Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM5, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RBX, s);
        emit_int_to_float(RBX, XMM4, s);
        delete c;
        emit_divsd(XMM5, XMM4, s);
        // store result
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_movsd(XMM4, reg, s);

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Float, Int
    else {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RBX, s);
        emit_int_to_float(RBX, XMM5, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM4, s);
        delete c;
        emit_divsd(XMM5, XMM4, s);
        // store result
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_movsd(XMM4, reg, s);

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }

    if (cgen_debug) cout << "--- Divide_class::code ---\n";
}

void Mod_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        e2->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Mod_class::code ---\n";

    // caution: their order
    e1->code(s);
    e2->code(s);

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    const char *c1 = operandStack.top();
    operandStack.pop();
    const char *c2 = operandStack.top();
    operandStack.pop();
    emit_mov(c2, RAX, s);
    emit_cqto(s);
    emit_mov(c1, RBX, s);
    delete c1;
    delete c2;
    emit_div(RBX, s);
    // store result
    int len = count_len_addr_reg_shift(RBP, res_addr);
    char reg[len];
    addr_reg_shift(reg, RBP, res_addr);
    emit_mov(RDX, reg, s);

    // put result into the operandStack
    char *str = new char[len];
    strcpy(str, reg);
    operandStack.push(str);

    if (cgen_debug) cout << "--- Mod_class::code ---\n";
}

void Neg_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        return;
    }

    e1->code(s);
    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    const char *c = operandStack.top();
    operandStack.pop();
    emit_mov(c, RAX, s);
    emit_neg(RAX, s);
    // store result
    int len = count_len_addr_reg_shift(RBP, res_addr);
    char reg[len];
    addr_reg_shift(reg, RBP, res_addr);
    emit_mov(RAX, reg, s);

    // put result into the operandStack
    char *str = new char[len];
    strcpy(str, reg);
    operandStack.push(str);
}

void Lt_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        e2->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Lt_class::code ---\n";

    // caution: their order
    e1->code(s);
    Symbol type1 = e1->getType();
    e2->code(s);
    Symbol type2 = e2->getType();

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    // case: both Int
    if (sameType(type1, Int) && sameType(type2, Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RDX, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RAX, s);
        delete c;
        emit_cmp(RDX, RAX, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_jl(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: both Float
    else if (sameType(type1, Float) && sameType(type2, Float)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, XMM0, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, XMM1, s);
        delete c;
        emit_ucompisd(XMM0, XMM1, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_jb(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Int Float
    else if (sameType(type1, Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM1, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        // convert Int to Float
        emit_mov(c, RAX, s);
        emit_int_to_float(RAX, XMM0, s);
        delete c;
        emit_ucompisd(XMM1, XMM0, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_jb(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Float, Int
    else {
        const char *c = operandStack.top();
        operandStack.pop();
        // convert Int to Float
        emit_mov(c, RAX, s);
        emit_int_to_float(RAX, XMM0, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM1, s);
        emit_ucompisd(XMM0, XMM1, s);
        delete c;
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_jb(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }

    if (cgen_debug) cout << "--- Lt_class::code ---\n";
}

void Le_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        e2->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Le_class::code ---\n";

    // caution: their order
    e1->code(s);
    Symbol type1 = e1->getType();
    e2->code(s);
    Symbol type2 = e2->getType();

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    // case: both Int
    if (sameType(type1, Int) && sameType(type2, Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RDX, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RAX, s);
        delete c;
        emit_cmp(RDX, RAX, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_jle(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: both Float
    else if (sameType(type1, Float) && sameType(type2, Float)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, XMM0, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, XMM1, s);
        delete c;
        emit_ucompisd(XMM0, XMM1, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_jbe(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Int Float
    else if (sameType(type1, Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM1, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        // convert Int to Float
        emit_mov(c, RAX, s);
        emit_int_to_float(RAX, XMM0, s);
        delete c;
        emit_ucompisd(XMM1, XMM0, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_jbe(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Float, Int
    else {
        const char *c = operandStack.top();
        operandStack.pop();
        // convert Int to Float
        emit_mov(c, RAX, s);
        emit_int_to_float(RAX, XMM0, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM1, s);
        emit_ucompisd(XMM0, XMM1, s);
        delete c;
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_jbe(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }

    if (cgen_debug) cout << "--- Le_class::code ---\n";
}

void Equ_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        e2->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Eqe_class::code ---\n";

    // caution: their order
    e1->code(s);
    Symbol type1 = e1->getType();
    e2->code(s);
    Symbol type2 = e2->getType();

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    // case: both Int
    if (sameType(type1, Int) && sameType(type2, Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RDX, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RAX, s);
        delete c;
        emit_cmp(RDX, RAX, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_je(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: both Float
    else if (sameType(type1, Float) && sameType(type2, Float)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, XMM0, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, XMM1, s);
        delete c;
        emit_ucompisd(XMM0, XMM1, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_je(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Int Float
    else if (sameType(type1, Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM1, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        // convert Int to Float
        emit_mov(c, RAX, s);
        emit_int_to_float(RAX, XMM0, s);
        delete c;
        emit_ucompisd(XMM1, XMM0, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_je(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Float, Int
    else {
        const char *c = operandStack.top();
        operandStack.pop();
        // convert Int to Float
        emit_mov(c, RAX, s);
        emit_int_to_float(RAX, XMM0, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM1, s);
        emit_ucompisd(XMM0, XMM1, s);
        delete c;
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_je(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }

    if (cgen_debug) cout << "--- Equ_class::code ---\n";
}

void Neq_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        e2->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Neq_class::code ---\n";

    // caution: their order
    e1->code(s);
    Symbol type1 = e1->getType();
    e2->code(s);
    Symbol type2 = e2->getType();

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    // case: both Int
    if (sameType(type1, Int) && sameType(type2, Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RDX, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RAX, s);
        delete c;
        emit_cmp(RDX, RAX, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_jne(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: both Float
    else if (sameType(type1, Float) && sameType(type2, Float)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, XMM0, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, XMM1, s);
        delete c;
        emit_ucompisd(XMM0, XMM1, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_jne(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Int Float
    else if (sameType(type1, Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM1, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        // convert Int to Float
        emit_mov(c, RAX, s);
        emit_int_to_float(RAX, XMM0, s);
        delete c;
        emit_ucompisd(XMM1, XMM0, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_jne(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Float, Int
    else {
        const char *c = operandStack.top();
        operandStack.pop();
        // convert Int to Float
        emit_mov(c, RAX, s);
        emit_int_to_float(RAX, XMM0, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM1, s);
        emit_ucompisd(XMM0, XMM1, s);
        delete c;
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_jne(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }

    if (cgen_debug) cout << "--- Neq_class::code ---\n";
}

void Ge_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        e2->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Ge_class::code ---\n";

    // caution: their order
    e1->code(s);
    Symbol type1 = e1->getType();
    e2->code(s);
    Symbol type2 = e2->getType();

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    // case: both Int
    if (sameType(type1, Int) && sameType(type2, Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RDX, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RAX, s);
        delete c;
        emit_cmp(RDX, RAX, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_jge(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: both Float
    else if (sameType(type1, Float) && sameType(type2, Float)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, XMM0, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, XMM1, s);
        delete c;
        emit_ucompisd(XMM0, XMM1, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_jae(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Int Float
    else if (sameType(type1, Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM1, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        // convert Int to Float
        emit_mov(c, RAX, s);
        emit_int_to_float(RAX, XMM0, s);
        delete c;
        emit_ucompisd(XMM1, XMM0, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_jae(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Float, Int
    else {
        const char *c = operandStack.top();
        operandStack.pop();
        // convert Int to Float
        emit_mov(c, RAX, s);
        emit_int_to_float(RAX, XMM0, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM1, s);
        emit_ucompisd(XMM0, XMM1, s);
        delete c;
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_jae(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }

    if (cgen_debug) cout << "--- Ge_class::code ---\n";
}

void Gt_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        e2->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Gt_class::code ---\n";

    // caution: their order
    e1->code(s);
    Symbol type1 = e1->getType();
    e2->code(s);
    Symbol type2 = e2->getType();

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    // case: both Int
    if (sameType(type1, Int) && sameType(type2, Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RDX, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, RAX, s);
        delete c;
        emit_cmp(RDX, RAX, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_jg(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: both Float
    else if (sameType(type1, Float) && sameType(type2, Float)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_mov(c, XMM0, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_mov(c, XMM1, s);
        delete c;
        emit_ucompisd(XMM0, XMM1, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_ja(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Int Float
    else if (sameType(type1, Int)) {
        const char *c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM1, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        // convert Int to Float
        emit_mov(c, RAX, s);
        emit_int_to_float(RAX, XMM0, s);
        delete c;
        emit_ucompisd(XMM1, XMM0, s);
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_ja(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }
        // case: Float, Int
    else {
        const char *c = operandStack.top();
        operandStack.pop();
        // convert Int to Float
        emit_mov(c, RAX, s);
        emit_int_to_float(RAX, XMM0, s);
        delete c;
        c = operandStack.top();
        operandStack.pop();
        emit_movsd(c, XMM1, s);
        emit_ucompisd(XMM0, XMM1, s);
        delete c;
        int tmp = pos_available;
        int pos = pos_available;
        int digit = 0;
        if (tmp == 0) digit = 1;
        else
            while (tmp != 0) {
                ++digit;
                tmp /= 10;
            }
        char pos_char[digit + 6];
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_ja(pos_char, s);
        // res = 0
        emit_mov("$0", RAX, s);
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_jmp(pos_char, s);
        // res = 1 in new branch
        sprintf(pos_char, "%s%d", POSITION, pos);
        emit_position(pos_char, s);
        emit_mov("$1", RAX, s);
        // store the result in this must-pass branch
        sprintf(pos_char, "%s%d", POSITION, pos + 1);
        emit_position(pos_char, s);
        int len = count_len_addr_reg_shift(RBP, res_addr);
        char reg[len];
        addr_reg_shift(reg, RBP, res_addr);
        emit_mov(RAX, reg, s);

        // update available POS name, consuming 2
        pos_available += 2;

        // put result into the operandStack
        char *str = new char[len];
        strcpy(str, reg);
        operandStack.push(str);
    }

    if (cgen_debug) cout << "--- Gt_class::code ---\n";
}

void And_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        e2->code(s);
        return;
    }
    if (cgen_debug) cout << "--- And_class::code ---\n";

    // caution: their order
    e1->code(s);
    e2->code(s);

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    const char *c = operandStack.top();
    operandStack.pop();
    emit_mov(c, RDX, s);
    delete c;
    c = operandStack.top();
    operandStack.pop();
    emit_mov(c, RAX, s);
    emit_and(RAX, RDX, s);

    int len = count_len_addr_reg_shift(RDX, res_addr);
    char reg[len];
    addr_reg_shift(reg, RDX, res_addr);
    emit_mov(RDX, reg, s);

    // put result into the operandStack
    char *str = new char[len];
    strcpy(str, reg);
    operandStack.push(str);

    if (cgen_debug) cout << "--- And_class::code ---\n";
}

void Or_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        e2->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Or_class::code ---\n";

    // caution: their order
    e1->code(s);
    e2->code(s);

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    const char *c = operandStack.top();
    operandStack.pop();
    emit_mov(c, RDX, s);
    delete c;
    c = operandStack.top();
    operandStack.pop();
    emit_mov(c, RAX, s);
    emit_or(RAX, RDX, s);

    int len = count_len_addr_reg_shift(RDX, res_addr);
    char reg[len];
    addr_reg_shift(reg, RDX, res_addr);
    emit_mov(RDX, reg, s);

    // put result into the operandStack
    char *str = new char[len];
    strcpy(str, reg);
    operandStack.push(str);

    if (cgen_debug) cout << "--- Or_class::code ---\n";
}

void Xor_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        e2->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Xor_class::code ---\n";

    // caution: their order
    e1->code(s);
    e2->code(s);

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    const char *c = operandStack.top();
    operandStack.pop();
    emit_mov(c, RDX, s);
    delete c;
    c = operandStack.top();
    operandStack.pop();
    emit_mov(c, RAX, s);
    delete c;
    emit_xor(RAX, RDX, s);

    int len = count_len_addr_reg_shift(RBP, res_addr);
    char reg[len];
    addr_reg_shift(reg, RBP, res_addr);
    emit_mov(RBP, reg, s);

    // put result into the operandStack
    char *str = new char[len];
    strcpy(str, reg);
    operandStack.push(str);

    if (cgen_debug) cout << "--- Xor_class::code ---\n";
}

void Not_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Not_class::code ---\n";

    // caution: their order
    e1->code(s);

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    const char *c = operandStack.top();
    operandStack.pop();
    emit_mov(c, RAX, s);
    delete c;
    emit_mov("$0x0000000000000001", RDX, s);
    emit_xor(RDX, RAX, s);
    int len = count_len_addr_reg_shift(RBP, res_addr);
    char reg[len];
    addr_reg_shift(reg, RBP, res_addr);
    emit_mov(RAX, reg, s);

    // put result into the operandStack
    char *str = new char[len];
    strcpy(str, reg);
    operandStack.push(str);

    if (cgen_debug) cout << "--- Not_class::code ---\n";
}

void Bitnot_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Bitnot_class::code ---\n";

    // caution: their order
    e1->code(s);

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    const char *c = operandStack.top();
    operandStack.pop();
    emit_mov(c, RAX, s);
    delete c;
    emit_not(RAX, s);
    int len = count_len_addr_reg_shift(RBP, res_addr);
    char reg[len];
    addr_reg_shift(reg, RBP, res_addr);
    emit_mov(RAX, reg, s);

    // put result into the operandStack
    char *str = new char[len];
    strcpy(str, reg);
    operandStack.push(str);

    if (cgen_debug) cout << "--- Bitnot_class::code ---\n";
}

void Bitand_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Bitand_class::code ---\n";

    // caution: their order
    e1->code(s);
    e2->code(s);

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    const char *c = operandStack.top();
    operandStack.pop();
    emit_mov(c, RDX, s);
    delete c;
    c = operandStack.top();
    operandStack.pop();
    emit_mov(c, RAX, s);
    delete c;
    emit_and(RAX, RDX, s);
    int len = count_len_addr_reg_shift(RBP, res_addr);
    char reg[len];
    addr_reg_shift(reg, RBP, res_addr);
    emit_mov(RDX, reg, s);

    // put result into the operandStack
    char *str = new char[len];
    strcpy(str, reg);
    operandStack.push(str);

    if (cgen_debug) cout << "--- Bitand_class::code ---\n";
}

void Bitor_class::code(ostream &s) {
    if (init_once) {
        e1->code(s);
        return;
    }
    if (cgen_debug) cout << "--- Bitand_class::code ---\n";

    // caution: their order
    e1->code(s);
    e2->code(s);

    // get stack space ready for the result
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    int res_addr = curr_usage;
    // compute the result
    const char *c = operandStack.top();
    operandStack.pop();
    emit_mov(c, RDX, s);
    delete c;
    c = operandStack.top();
    operandStack.pop();
    emit_mov(c, RAX, s);
    delete c;
    emit_or(RAX, RDX, s);
    int len = count_len_addr_reg_shift(RBP, res_addr);
    char reg[len];
    addr_reg_shift(reg, RBP, res_addr);
    emit_mov(RDX, reg, s);

    // put result into the operandStack
    char *str = new char[len];
    strcpy(str, reg);
    operandStack.push(str);

    if (cgen_debug) cout << "--- Bitand_class::code ---\n";
}

void Const_int_class::code(ostream &s) {
    if (init_once) {
        return;
    }
    if (cgen_debug) cout << "--- Const_int_class::code ---\n";

    // get stack space
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    // assign the value -> %rax -> addr
    char val[strlen(value->get_string()) + 1] = "$";
    for (int i = 0; i <= int(strlen(value->get_string())); ++i)
        val[i + 1] = value->get_string()[i];
    emit_mov(val, RAX, s);
    int len = count_len_addr_reg_shift(RBP, curr_usage);
    char reg[len];
    addr_reg_shift(reg, RBP, curr_usage);
    emit_mov(RAX, reg, s);
    // push into the operandStack
    char *c = new char[len];
    strcpy(c, reg);
    operandStack.push(c);

    if (cgen_debug) cout << "--- Const_int_class::code ---\n";
}

void Const_string_class::code(ostream &s) {
    if (init_once) {
        stringtable.add_string(value->get_string());
        return;
    }

    if (cgen_debug) cout << "--- Const_string_class::code ---\n";
    // get stack space
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    // get .LCX
    int tmp = -1;
    for (int i = stringtable.first(); stringtable.more(i); i = stringtable.next(i))
        if (strcmp(stringtable.lookup(i)->get_string(), value->get_string()) == 0) tmp = i;
    int digit = 0;
    if (tmp == 0) digit = 1;
    else
        while (tmp != 0) {
            ++digit;
            tmp /= 10;
        }
    char *c1 = new char[digit + 5];
    strcpy(c1, "$.LC");
    char c2[digit + 1];
    sprintf(c2, "%d", tmp);
    for (int i = 0; i < int(strlen(c2)); ++i) {
        c1[4 + i] = c2[i];
    }
    c1[digit + 4] = '\0';
    // assign the value -> %rax -> addr
    emit_mov(c1, RAX, s);
    int len = count_len_addr_reg_shift(RBP, curr_usage);
    char reg[len];
    addr_reg_shift(reg, RBP, curr_usage);
    emit_mov(RAX, reg, s);
    // push into the operandStack
    char *c = new char[len];
    strcpy(c, reg);
    operandStack.push(c);
    if (cgen_debug) cout << "--- Const_string_class::code ---\n";
}

void Const_float_class::code(ostream &s) {
    if (init_once) {
        return;
    }
    if (cgen_debug) cout << "--- Const_float_class::code ---\n";

    // get stack space
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    // convert string to double
    int decimal = 0;
    for (int i = 0; i < value->get_len(); ++i)
        if (value->get_string()[i] == '.') decimal = i;
    double digit = 0;
    for (int i = 0; i < value->get_len(); ++i) {
        if (i == decimal) continue;
        if (i < decimal)
            digit += (value->get_string()[i] - '0') * pow(10, decimal - 1 - i);
        else
            digit += (value->get_string()[i] - '0') * pow(10, decimal - i);
    }
    // convert double to hex byte to byte
    char tmp[19] = "$0x";
    unsigned char *p = (unsigned char *) (&digit);
    int idx = 3;
    for (int i = sizeof(digit) - 1; i >= 0; --i) {
        char s[3];
        sprintf(s, "%02x", static_cast<int>(*(p + i)));
        tmp[idx++] = s[0];
        tmp[idx++] = s[1];
    }
    tmp[idx] = '\0';
    if (cgen_debug) cout << "Finish convertion from double " << digit << " to hex " << tmp << ".\n";
    // assign the value -> %rax -> addr
    emit_mov(tmp, RAX, s);
    int len = count_len_addr_reg_shift(RBP, curr_usage);
    char reg[len];
    addr_reg_shift(reg, RBP, curr_usage);
    emit_mov(RAX, reg, s);
    // push into the operandStack
    char *c = new char[len];
    strcpy(c, reg);
    operandStack.push(c);

    if (cgen_debug) cout << "--- Const_float_class::code ---\n";
}

void Const_bool_class::code(ostream &s) {
    if (init_once) {
        return;
    }
    if (cgen_debug) cout << "--- Const_bool_class::code ---\n";

    // get stack space
    emit_sub("$8", RSP, s);
    curr_usage += 8;
    // assign the value -> %rax -> addr
    char tmp[3] = "$1";
    if (!value) tmp[1] = '0';
    emit_mov(tmp, RAX, s);
    int len = count_len_addr_reg_shift(RBP, curr_usage);
    char reg[len];
    addr_reg_shift(reg, RBP, curr_usage);
    emit_mov(RAX, reg, s);
    // push into the operandStack
    char *c = new char[len];
    strcpy(c, reg);
    operandStack.push(c);

    if (cgen_debug) cout << "--- Const_bool_class::code ---\n";
}

void Object_class::code(ostream &s) {
    if (init_once) {
        return;
    }
    if (cgen_debug) cout << "--- Object_class::code ---\n";
    // lookup its addr in varNameToAddr
    int pos = *(varNameToAddr.lookup(var));
    char *addr = new char[strlen(name_proc[pos])];
    strcpy(addr, name_proc[pos]);
    // put addr into the operandStack
    operandStack.push(addr);

    if (cgen_debug) cout << "--- Object_class::code ---\n";
}

void No_expr_class::code(ostream &s) {
    if (init_once) {
        return;
    }
    if (cgen_debug) cout << "--- No_expr_class::code ---\n";

    const char *c = nullptr;
    operandStack.push(c);

    if (cgen_debug) cout << "--- No_expr_class::code ---\n";
}