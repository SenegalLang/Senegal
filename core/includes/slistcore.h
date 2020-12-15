#ifndef SENEGAL_SLISTCORE_H
#define SENEGAL_SLISTCORE_H

#include "../../src/includes/svm.h"

void initListClass(VM* vm);
void addToList(VM* vm, GCList** list, Constant element);

#endif //SENEGAL_SLISTCORE_H
