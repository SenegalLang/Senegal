#ifndef SENEGAL_SCONSTANT_UTILS_H
#define SENEGAL_SCONSTANT_UTILS_H

#include "svm.h"
#include "sparser.h"

void writeConstantPool(VM* vm, Compiler* compiler, ConstantPool* cp, Constant constant);
void freeConstantPool(VM* vm, Compiler* compiler, ConstantPool* cp);

#endif //SENEGAL_SCONSTANT_UTILS_H