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
#include <synchapi.h>
#include <stdio.h>
#include <tchar.h>
    typedef struct {
		char** errors;
		char error_str[2048];
		int num_errors;
        bool has_errors;
	} mem_errors_t, *mem_errors_p;
	typedef struct {
		mem_errors_p reporter;
		LPHANDLE hMap;
		LPHANDLE hFile;
	} mem_mapping_t, *mem_mapping_p;
	typedef struct {
		mem_errors_p reporter;
		mem_mapping_p map;
		int offset_hi;
		int offset_lo;
		size_t size;
		LPVOID data;
	} mem_mapview_t, *mem_mapview_p;
	LIB_EXPORT bool mem_mapping_errored(mem_mapping_p map);
	LIB_EXPORT char* mem_mapping_get_errors(mem_mapping_p map);
	LIB_EXPORT bool mem_mapview_errored(mem_mapview_p view);
	LIB_EXPORT char* mem_mapview_get_errors(mem_mapview_p view);
    LIB_EXPORT mem_mapping_p mem_create_file_mapping(const char* filename, const char* mapname, int size_hi, int size_lo);
	LIB_EXPORT mem_mapping_p mem_open_file_mapping(const char* mapname);
	LIB_EXPORT mem_mapview_p mem_create_view(mem_mapping_p map, int offset_hi, int offset_lo, size_t size);
	LIB_EXPORT mem_mapview_p mem_mapview_stub();
	LIB_EXPORT bool mem_remap_view(mem_mapping_p map, mem_mapview_p view);
	LIB_EXPORT bool mem_write_to_view(mem_mapview_p view, int offset_hi, int offset_lo, unsigned char *bytes, int len);
	LIB_EXPORT char *mem_read_view(mem_mapview_p view, int offset_hi, int offset_lo, size_t *read);
	LIB_EXPORT bool mem_persist_view(mem_mapview_p view);
	LIB_EXPORT DWORD mem_get_sys_granularity();
#ifdef __cplusplus
}
#endif
