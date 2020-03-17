#include "memapi.h"

/*
* Errors
*/
mem_errors_p get_reporter() {
    mem_errors_p errors = (mem_errors_p)malloc(sizeof(mem_errors_t));
	errors->num_errors = 0;
    errors->has_errors = false;
	errors->errors = (char**)malloc(sizeof(char) * 1024 * 16);
    return errors;
}
char* GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = GetLastError();
    if(errorMessageID == 0)
        return ""; //No error message has been recorded

    LPSTR messageBuffer = (LPSTR)malloc(1024 * sizeof(char));
    size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    char message[1024];
    sprintf_s(message,1024, messageBuffer);
    //Free the buffer.
    free(messageBuffer);
    return message;
}
char* print_errors(mem_errors_p errors) {
    // errors->error_str = (char*)malloc(sizeof(char) * 2048);
	for (int i = 0; i < errors->num_errors; i++) {
		sprintf_s(errors->error_str, 1024, errors->errors[i]);
		if (i != errors->num_errors - 1) sprintf_s(errors->error_str, 1024, "\r\n");
	}
    
    return errors->error_str;
}
void report(mem_errors_p errors, char* error) {
    errors->errors[errors->num_errors] = (char*)malloc(1024*sizeof(char));
    sprintf_s(errors->errors[errors->num_errors], 1024, error);
    errors->num_errors++;
    errors->has_errors=true;
    
}
void report_last(mem_errors_p errors, char* error) {
    char *e = (char*)malloc(sizeof(char) * 1024);
    sprintf_s(e, 1024, "%s Error: %s", error, GetLastErrorAsString()); 
    report(errors, e);
}
void report_last_explicit(mem_errors_p errors, char* error, char* explicitError) {
    char *e = (char*)malloc(sizeof(char) * 1024);
    sprintf_s(e, 1024, "%s Error: %s", error, explicitError); 
    report(errors, e);
}

LIB_EXPORT bool mem_mapping_errored(mem_mapping_p map) {
    return map->reporter->has_errors;
}
LIB_EXPORT char* mem_mapping_get_errors(mem_mapping_p map) {
    return print_errors(map->reporter);
}
LIB_EXPORT bool mem_mapview_errored(mem_mapview_p view) {
    return view->reporter->has_errors;
}
LIB_EXPORT char* mem_mapview_get_errors(mem_mapview_p view) {
    return print_errors(view->reporter);
}
/*
* See: https://docs.microsoft.com/en-us/windows/win32/secauthz/creating-a-security-descriptor-for-a-new-object-in-c--?redirectedfrom=MSDN
* IPC Access
*/
PSECURITY_ATTRIBUTES get_ipc_sd(mem_errors_p error) {
    DWORD dwRes, dwDisposition;
    PSID pEveryoneSID = NULL;
    PACL pACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    EXPLICIT_ACCESS ea[2];
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld =
            SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    SECURITY_ATTRIBUTES sa;
    LONG lRes;
    HKEY hkSub = NULL;

    // Create a well-known SID for the Everyone group.
    if(!AllocateAndInitializeSid(&SIDAuthWorld, 1,
                     SECURITY_WORLD_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pEveryoneSID)) {
                         report_last(error, "AllocateAndInitializeSid Error");
                         goto Cleanup;
                     }
    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
    ea->grfAccessPermissions = KEY_ALL_ACCESS;
    ea->grfAccessMode = SET_ACCESS;
    ea->grfInheritance = INHERIT_NO_PROPAGATE;
    ea->Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea->Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP; 
    ea->Trustee.ptstrName = (LPTSTR) pEveryoneSID;
    if(!SetEntriesInAcl(2, ea, NULL, &pACL) != ERROR_SUCCESS) {
        report_last(error, "SetEntriesInAcl");
        goto Cleanup;
    }

    pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    if(pSD == NULL) {
        report_last(error, "LocalAlloc Error");
        goto Cleanup;
    }
    if(!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
        report_last(error, "InitializeSecurityDescriptor");
        goto Cleanup;
    }
    if(!SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE)) {
        report_last(error, "SetSecurityDescriptorDacl");
        goto Cleanup;
    }
    PSECURITY_ATTRIBUTES ret_val;
    ret_val = (PSECURITY_ATTRIBUTES)malloc( sizeof(SECURITY_ATTRIBUTES));
    ret_val->lpSecurityDescriptor = pSD;
    ret_val->bInheritHandle = true;
    ret_val->nLength = sizeof(ret_val);
    return ret_val;
    Cleanup:
    if (pEveryoneSID) 
        FreeSid(pEveryoneSID);
    if (pACL) 
        LocalFree(pACL);
    if (pSD) 
        LocalFree(pSD);
    return NULL;
}


LIB_EXPORT mem_mapping_p mem_create_file_mapping(const char* filename, const char* mapname, int size_hi, int size_lo) {
    mem_mapping_p map = (mem_mapping_p)malloc(sizeof(mem_mapping_t));
    map->reporter = get_reporter();
    HANDLE hFile = filename[0] == 0 ? INVALID_HANDLE_VALUE : CreateFile(filename, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, get_ipc_sd(map->reporter), OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(filename[0] != 0 && hFile == INVALID_HANDLE_VALUE) {
        report_last(map->reporter, "CreateFile");
    }
    HANDLE hMapping = CreateFileMapping(hFile, get_ipc_sd(map->reporter), PAGE_READWRITE, size_hi, size_lo, mapname);
    if(hMapping == NULL) {
        report_last(map->reporter,"CreateFileMapping");
    }
    map->hMap = hMapping;
    map->hFile = hFile;
    return map;
}
LIB_EXPORT mem_mapping_p mem_open_file_mapping(const char* mapname) {
    mem_mapping_p map = (mem_mapping_p)malloc(sizeof(mem_mapping_t));
    map->reporter = get_reporter();
    map->hMap = (LPHANDLE)malloc(sizeof(HANDLE));
    map->hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, true, mapname);
    if(map->hMap == NULL) {
        report_last(map->reporter,"OpenFileMapping");
    }
    HANDLE invalid = INVALID_HANDLE_VALUE;
    map->hFile = &invalid;
    return map;
}
LIB_EXPORT mem_mapview_p mem_create_view(mem_mapping_p map, int offset_hi, int offset_lo, size_t size) {
    mem_mapview_p view = (mem_mapview_p)malloc(sizeof(mem_mapview_t));
    view->reporter=get_reporter();
    view->map=map;
    view->offset_hi = offset_hi;
    view->offset_lo = offset_lo;
    view->size=size;
    view->data = (LPVOID)malloc(size);
    view->data = MapViewOfFile(map->hMap, FILE_MAP_ALL_ACCESS, offset_hi, offset_lo, size);
    if(view->data == NULL) {
        printf("Map Handle: %x\r\n\r\n", map->hMap);
        report_last(view->reporter,"MapViewOfFile");
    }
    return view;
}

LIB_EXPORT bool mem_remap_view(mem_mapping_p map, mem_mapview_p view) {
    view->map=map;
    view->data = MapViewOfFile(map->hMap, FILE_MAP_ALL_ACCESS, view->offset_hi, view->offset_lo, view->size);
    if(view->data == NULL) {
        report_last(view->reporter, "MapViewOfFile");
        return false;
    }
    return true;
}
LIB_EXPORT bool mem_write_to_view(mem_mapview_p view, unsigned char* bytes, size_t len) {
    __try {
        ((DWORD*)(view->data))[0] = len;
        int offset = sizeof(DWORD) / sizeof(char);
        for(int i=0; i< len;i++) {
            ((char*)(view->data))[i + offset] = bytes[i];
        }
    }
    __except(GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_EXECUTION) {
        report_last(view->reporter,"mem_write_to_view");
        report_last_explicit(view->reporter, "mem_write_to_view", "Failed to write to view.");
        return false;
    }
    return true;
}
LIB_EXPORT char* mem_read_view(mem_mapview_p view, size_t *read) {
    __try {
        DWORD len = *((LPDWORD) view->data);
        char* ret = (char*)malloc(sizeof(char) * len);
        int offset = sizeof(DWORD) / sizeof(char);
        for(int i=offset;i<len;i++) {
            char byte = ((char*)(view->data))[i];
            ret[i] = byte;
        }
        *read = len;
        return ret;
    } 
     __except(GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_EXECUTION) {
        report_last(view->reporter,"mem_read_view");
        report_last_explicit(view->reporter, "mem_read_view", "Failed to write to view.");
        return NULL;
    }
}
LIB_EXPORT bool mem_persist_view(mem_mapview_p view) {
    if(!FlushViewOfFile(view->data, 0)) {
        report_last(view->reporter, "FlushViewOfFile");
        return false;
    }
    return true;
}
LIB_EXPORT mem_mapview_p mem_mapview_stub() {
    return (mem_mapview_p)NULL ;
}