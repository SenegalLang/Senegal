#ifdef OPCODE

OPCODE(OPCODE_TRUE)
OPCODE(OPCODE_FALSE)

// BITWISE OP
OPCODE(OPCODE_AND)
OPCODE(OPCODE_OR)
OPCODE(OPCODE_XOR)
OPCODE(OPCODE_LSHIFT)
OPCODE(OPCODE_RSHIFT)
OPCODE(OPCODE_BITNOT)

// Arithmetic
OPCODE(OPCODE_ADD) // Addition
OPCODE(OPCODE_SUB) // Subtract
OPCODE(OPCODE_MUL) // Multiply
OPCODE(OPCODE_MOD) // Multiply
OPCODE(OPCODE_DIV) // Divide
OPCODE(OPCODE_INC) // Increment
OPCODE(OPCODE_DEC) // Decrement
OPCODE(OPCODE_POW) // Power

// Comparisons
OPCODE(OPCODE_EQUAL)
OPCODE(OPCODE_NOTEQ) // Not equal
OPCODE(OPCODE_GREATER)
OPCODE(OPCODE_LESSER)
OPCODE(OPCODE_GE) // Greater or equal
OPCODE(OPCODE_LE) // Lesser or equal

OPCODE(OPCODE_NEG) // Negate
OPCODE(OPCODE_NOT)

// Stack
OPCODE(OPCODE_DUP) // Duplicate
OPCODE(OPCODE_DUP2) // Duplicate 2nd
OPCODE(OPCODE_POP)
OPCODE(OPCODE_POPN)

// LOAD
OPCODE(OPCODE_LOAD)
OPCODE(OPCODE_LOADN1)
OPCODE(OPCODE_LOAD0)
OPCODE(OPCODE_LOAD1)
OPCODE(OPCODE_LOAD2)
OPCODE(OPCODE_LOAD3)
OPCODE(OPCODE_LLOAD)

// Map and list
OPCODE(OPCODE_NEWMAP)
OPCODE(OPCODE_NEWLIST)

// TODO(Calamity210): add superinstructions for (SET)ACCESS
OPCODE(OPCODE_ACCESS)
OPCODE(OPCODE_SETACCESS)

// class
OPCODE(OPCODE_NEWCLASS)
OPCODE(OPCODE_NEWFINALCLASS)
OPCODE(OPCODE_SETFIELD)
OPCODE(OPCODE_GETFIELD)
OPCODE(OPCODE_NEWMETHOD)
OPCODE(OPCODE_NEWSTATICMETHOD)
OPCODE(OPCODE_NEWFIELD)
OPCODE(OPCODE_NEWSTATICFIELD)
OPCODE(OPCODE_INHERIT)
OPCODE(OPCODE_GETSUPER)
OPCODE(OPCODE_SUPERINVOKE)

// Globals
OPCODE(OPCODE_NEWGLOB)
OPCODE(OPCODE_GETGLOB)
OPCODE(OPCODE_SETGLOB)

// Locals
OPCODE(OPCODE_GETLOC)
OPCODE(OPCODE_GETLOC0)
OPCODE(OPCODE_GETLOC1)
OPCODE(OPCODE_GETLOC2)
OPCODE(OPCODE_GETLOC3)
OPCODE(OPCODE_GETLOC4)
OPCODE(OPCODE_GETLOC5)
OPCODE(OPCODE_SETLOC)
OPCODE(OPCODE_SETLOC0)
OPCODE(OPCODE_SETLOC1)
OPCODE(OPCODE_SETLOC2)
OPCODE(OPCODE_SETLOC3)
OPCODE(OPCODE_SETLOC4)
OPCODE(OPCODE_SETLOC5)

OPCODE(OPCODE_GETUPVAL)
OPCODE(OPCODE_SETUPVAL)
OPCODE(OPCODE_CLOSEUPVAL)

// Flow
OPCODE(OPCODE_JMP)
OPCODE(OPCODE_JF)
OPCODE(OPCODE_LOOP)
OPCODE(OPCODE_BREAK)
OPCODE(OPCODE_CLOSURE)
OPCODE(OPCODE_INVOKE)

// CALL
OPCODE(OPCODE_CALL)
OPCODE(OPCODE_CALL0)
OPCODE(OPCODE_CALL1)
OPCODE(OPCODE_CALL2)
OPCODE(OPCODE_CALL3)
OPCODE(OPCODE_CALL4)
OPCODE(OPCODE_CALL5)
OPCODE(OPCODE_CALL6)
OPCODE(OPCODE_CALL7)
OPCODE(OPCODE_CALL8)

OPCODE(OPCODE_NULL)
OPCODE(OPCODE_SUSPEND)
OPCODE(OPCODE_THROW)
OPCODE(OPCODE_YIELD)
OPCODE(OPCODE_RET)

#endif