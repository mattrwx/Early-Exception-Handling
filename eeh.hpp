#include <Windows.h>
#include <vector>

void AddEarlyExceptionHandler(ULONG First, LONG(*Handler)(EXCEPTION_POINTERS*));

LONG RemoveEarlyExceptionHandler(LONG(*Handler)(EXCEPTION_POINTERS*));