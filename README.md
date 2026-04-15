# Early-Exception-Handling
A fast and reasonably stealthy way of handling exceptions in usermode.

### Overview
The goal of this was to be a drop-in replacement for vectored exception handling. One issue with VEH is that you can't always ensure that your exceptions will be caught first even if you pass `true` as the first argument. If there are conflicting VEHs, then you will either have to hook AddVectoredExceptionHandler or periodically check to ensure you are the most recently added "first" handler. Our _Early Exception Handler_ works by dataptr swapping `Wow64PrepareForException` to point to our exception handler. This exception handler will then go and call all of the handlers added by `AddEarlyExceptionHandler()`. Finally using ZwContinue to resume execution.

### Usage
```cpp
#include "eeh.hpp"
#include <print>


LONG exception_logger(EXCEPTION_POINTERS* exception_info)
{
	std::println("Exception! {:X}", exception_info->ExceptionRecord->ExceptionCode);
	return EXCEPTION_CONTINUE_EXECUTION;
}

int main()
{
	AddEarlyExceptionHandler(true, &exception_logger);

	__try { *(int*)0 = 0; }
	__except (EXCEPTION_EXECUTE_HANDLER) { std::println("Caught access violation"); }

	RemoveEarlyExceptionHandler(&exception_logger);

	__try { *(int*)0 = 0; }
	__except (EXCEPTION_EXECUTE_HANDLER) { std::println("Caught access violation"); }
}
```
