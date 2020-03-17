//#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define LIB_EXPORT __declspec(dllexport)
#else
#define LIB_EXPORT
#endif
#define WIN32_LEAN_AND_MEAN      // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdbool.h>
#include <aclapi.h>
#include <stdio.h>
#include <tchar.h>
    typedef struct {
		char** errors;
		char error_str[2048];
		int num_errors;
        bool has_errors;
	} mem_errors_t, *mem_errors_p;
    HANDLE test_create_file_mapping();
#ifdef __cplusplus
}
#endif
