#include "memory.h"

/*
* Errors
*/

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


HANDLE test_create_file_mapping() {
    mem_errors_p errors = (mem_errors_p)malloc(sizeof(mem_errors_t));
    HANDLE hFile = CreateFile("./some-file.txt", GENERIC_READ|GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, get_ipc_sd(errors), OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hMapping = CreateFileMapping(hFile, get_ipc_sd(errors), PAGE_READWRITE, 0, 5000, "test");
    return hMapping;
}
// df