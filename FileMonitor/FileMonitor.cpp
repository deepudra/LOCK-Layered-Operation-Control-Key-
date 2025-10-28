#include <windows.h>
#include <winternl.h>  // Required for NtQuerySystemInformation
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define BUFFER_SIZE 1024  

std::wstring monitoredDirectory = L"C:\\UnityProjects\\Deepu\\LockWorkSpace"; // Change to your directory

void LogEvent(const std::string& message) {
    std::ofstream logFile("FileActivityLog.txt", std::ios::app);
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.close();
    }
}

// Function to check if the file is currently open
bool IsFileOpened(const std::wstring& filePath) {
    HANDLE hFile = CreateFileW(
        filePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        // If file cannot be opened, it may already be in use
        return true;
    }

    CloseHandle(hFile);
    return false;
}

void MonitorDirectory(const std::wstring& directoryPath) {
    HANDLE hDir = CreateFileW(
        directoryPath.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL
    );

    if (hDir == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open directory handle. Error: " << GetLastError() << std::endl;
        return;
    }

    char buffer[BUFFER_SIZE];
    DWORD bytesReturned;

    while (true) {
        if (ReadDirectoryChangesW(
            hDir, buffer, sizeof(buffer), TRUE,
            FILE_NOTIFY_CHANGE_FILE_NAME |
            FILE_NOTIFY_CHANGE_DIR_NAME |
            FILE_NOTIFY_CHANGE_ATTRIBUTES |
            FILE_NOTIFY_CHANGE_SIZE |
            FILE_NOTIFY_CHANGE_LAST_WRITE |
            FILE_NOTIFY_CHANGE_SECURITY,
            &bytesReturned, NULL, NULL))
        {
            FILE_NOTIFY_INFORMATION* fni = (FILE_NOTIFY_INFORMATION*)buffer;
            do {
                std::wstring fileName(fni->FileName, fni->FileName + fni->FileNameLength / sizeof(WCHAR));
                std::wstring fullFilePath = directoryPath + L"\\" + fileName;

                std::string eventType;
                switch (fni->Action) {
                case FILE_ACTION_ADDED: eventType = "Created"; break;
                case FILE_ACTION_REMOVED: eventType = "Deleted"; break;
                case FILE_ACTION_MODIFIED: eventType = "Modified"; break;
                case FILE_ACTION_RENAMED_OLD_NAME: eventType = "Renamed From"; break;
                case FILE_ACTION_RENAMED_NEW_NAME: eventType = "Renamed To"; break;
                default: eventType = "Unknown"; break;
                }

                std::string logMessage = eventType + ": " + std::string(fileName.begin(), fileName.end());
                std::cout << logMessage << std::endl;
                LogEvent(logMessage);

                // Detect if the file is opened
                if (IsFileOpened(fullFilePath)) {
                    std::string openMessage = "File Opened: " + std::string(fileName.begin(), fileName.end());
                    std::cout << openMessage << std::endl;
                    LogEvent(openMessage);
                }

                if (!fni->NextEntryOffset)
                    break;

                fni = (FILE_NOTIFY_INFORMATION*)((char*)fni + fni->NextEntryOffset);
            } while (true);
        }
        else {
            std::cerr << "ReadDirectoryChangesW failed. Error: " << GetLastError() << std::endl;
            break;
        }
    }

    CloseHandle(hDir);
}

int main() {
    std::wcout << L"Monitoring changes in: " << monitoredDirectory << L"\n";
    MonitorDirectory(monitoredDirectory);
    return 0;
}
