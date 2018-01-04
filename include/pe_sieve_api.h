#pragma once

#include <Windows.h>
#include "pe_sieve_types.h"

#ifdef PESIEVE_EXPORTS
#define PESIEVE_API __declspec(dllexport)
#else
#define PESIEVE_API __declspec(dllimport)
#endif

extern "C" {
	void PESIEVE_API __stdcall help(void);
	t_report PESIEVE_API __stdcall scan(t_params args);
};
