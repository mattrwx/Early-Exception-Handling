#include "eeh.hpp"

namespace
{
	bool init_complete{};

	void(*ZwContinue)(CONTEXT*, bool);
	LONG(*Wow64PrepareForExecution)(EXCEPTION_RECORD*, CONTEXT*);

	std::vector<LONG(*)(EXCEPTION_POINTERS*)> handlers{};

	void exception_handler(EXCEPTION_RECORD* exception_record, CONTEXT* context_record)
	{
		EXCEPTION_POINTERS exception_info{ exception_record, context_record };

		for (auto handler : handlers)
			if (handler(&exception_info) == EXCEPTION_CONTINUE_EXECUTION)
				ZwContinue(context_record, false);

		if (Wow64PrepareForExecution)
			(Wow64PrepareForExecution)(exception_record, context_record);
	}

	bool init()
	{
		auto ntdll = GetModuleHandleA("ntdll.dll");
		if (!ntdll)
			return false;


		auto dispatcher = (uintptr_t)GetProcAddress(ntdll, "KiUserExceptionDispatcher");
		ZwContinue = (void(*)(CONTEXT*, bool))(GetProcAddress(ntdll, "ZwContinue"));

		auto rel_addr = *(int32_t*)(dispatcher + 4);
		void** function_ptr = (void**)(dispatcher + 8 + rel_addr);

		Wow64PrepareForExecution = (LONG(*)(EXCEPTION_RECORD*, CONTEXT*))(*function_ptr);

		DWORD old_protect{};
		VirtualProtect(function_ptr, sizeof(void*), PAGE_EXECUTE_READWRITE, &old_protect);
		*function_ptr = &exception_handler;
		VirtualProtect(function_ptr, sizeof(void*), old_protect, &old_protect);

		init_complete = true;
		return true;
	}
}

void AddEarlyExceptionHandler(ULONG First, LONG(*Handler)(EXCEPTION_POINTERS*))
{
	if (!init_complete)
		if (!init())
			return;

	if (First)
		handlers.insert(handlers.begin(), Handler);
	else
		handlers.push_back(Handler);
}

LONG RemoveEarlyExceptionHandler(LONG(*Handler)(EXCEPTION_POINTERS*))
{
	auto it = std::find(handlers.begin(), handlers.end(), Handler);

	if (it == handlers.end())
		return false;

	handlers.erase(it);

	return true;
}
