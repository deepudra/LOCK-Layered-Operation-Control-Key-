#include <windows.h>
#include <winevt.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <locale>
#include <codecvt>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <ctime>
#include "json.hpp"
#include <tlhelp32.h>
#include <regex>

#pragma comment(lib, "wevtapi.lib")

using json = nlohmann::json;

using namespace std;

unordered_map<string, string> extensionMap;


std::string wstringToString(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}
std::wstring stringToWstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

bool containsFTP(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file: " << filePath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Convert line to lowercase for case-insensitive comparison
        std::string lowerLine = line;
        std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);

        if (lowerLine.find("ftp") != std::string::npos) {
            return true;
        }
    }

    return false;
}

bool FirstTimeFTP=false;

void extractFTPDetails(const std::string& filePath) {
        //if (FirstTimeFTP == false) {
        //    FirstTimeFTP = true;
        /*if (!containsFTP(filePath)) {
            std::cerr << "Error: No FTP-related code found in the file." << std::endl;
            return;
        }
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file." << std::endl;
            return;
        }
        std::string line;
        std::regex userRegex(R"(\.add_user\(\"([^\"]+)\",\s*\"([^\"]+)\")");
        std::regex portRegex(R"(FTPServer\(\(\"[0-9.]+\",\s*(\d+)\))");
        std::string username, password, port;
        while (std::getline(file, line)) {
            std::smatch match;
            if (std::regex_search(line, match, userRegex)) {
                if (match.size() > 2) {
                    username = match[1];
                    password = match[2];
                }
            }
            if (std::regex_search(line, match, portRegex)) {
                if (match.size() > 1) {
                    port = match[1];
                }
            }
        }
        file.close();*/

        std::string username, password, port;
        username = "user";
        password = "password";
        port = "2121";

        if (!username.empty() && !password.empty() && !port.empty()) {
            std::cout << "Username: " << username << std::endl;
            std::cout << "Password: " << password << std::endl;
            std::cout << "Port: " << port << std::endl;
        }
        else {
            std::cerr << "Error: Unable to extract FTP details." << std::endl;
        }

        std::string script_name = "FTPserver.py";

        std::string command = "python " + script_name +
            " --username " + username +
            " --password " + password +
            " --port " + port;

        //system(command.c_str());
        std::wstring wCommand = stringToWstring(command);
        STARTUPINFOW si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        si.cb = sizeof(si);
        if (CreateProcessW(NULL, &wCommand[0], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
            std::cout << "Python script started successfully.\n";
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else {
            std::cerr << "Failed to start Python script.\n";
        }
    //}
}


string GetLogFileName() {
    time_t now = time(0);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    stringstream ss;
    ss << "D" << (timeinfo.tm_mday < 10 ? "0" : "") << timeinfo.tm_mday
        << "M" << (timeinfo.tm_mon + 1 < 10 ? "0" : "") << (timeinfo.tm_mon + 1)
        << "Y" << (timeinfo.tm_year + 1900) << ".txt";

    return ss.str();
}

void WriteLog(const string& logMessage) {
    string logFileName = "Log/" + GetLogFileName();
    ofstream logFile(logFileName, ios::app); // Append mode

    if (!logFile) {
        cerr << "Error: Cannot open log file!" << endl;
        return;
    }
    time_t now = time(0);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    logFile << "["
        << (timeinfo.tm_hour < 10 ? "0" : "") << timeinfo.tm_hour << ":"
        << (timeinfo.tm_min < 10 ? "0" : "") << timeinfo.tm_min << ":"
        << (timeinfo.tm_sec < 10 ? "0" : "") << timeinfo.tm_sec
        << "] " << logMessage << endl;

    logFile.close();
}


string ToUpperCase(string str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

vector<uint8_t> ReadFileSignature(const string& filePath, size_t numBytes = 8) {
    ifstream file(filePath, ios::binary);
    if (!file) {
        cerr << "Error: Cannot open file " << filePath << endl;
        return {};
    }
    vector<uint8_t> signature(numBytes);
    file.read(reinterpret_cast<char*>(signature.data()), numBytes);
    signature.resize(file.gcount());
    return signature;
}

std::wstring GetParentDirectory(const std::wstring& path) {
    size_t lastSlash = path.find_last_of(L"/\\");
    if (lastSlash != std::wstring::npos) {
        return path.substr(0, lastSlash); 
    }
    return L"";
}

bool HaveSameParentDirectory(const std::wstring& path1, const std::wstring& path2) {
    return GetParentDirectory(path1) == GetParentDirectory(path2);
}


vector<uint8_t> GetSignatureFromJson(const string& jsonPath, const string& fileExt) {
    ifstream file(jsonPath);
    if (!file.is_open()) {
        cerr << "Error: Cannot open JSON file " << jsonPath << endl;
        return {};
    }
    json jsonData;
    file >> jsonData;
    if (!jsonData.contains("EXT") || !jsonData.contains("Signature")) {
        cerr << "Error: JSON structure is incorrect" << endl;
        return {};
    }

    if (jsonData["EXT"] != fileExt) {
        cerr << "Error: File extension mismatch in JSON" << endl;
        return {};
    }
    string signatureStr = jsonData["Signature"];
    vector<uint8_t> signatureBytes;
    stringstream ss(signatureStr);
    string hexByte;
    while (ss >> hexByte) {
        signatureBytes.push_back(stoul(hexByte, nullptr, 16));
    }
    return signatureBytes;
}

bool IfJsonExist(const string& jsonPath) {
    ifstream file(jsonPath);
    if (!file.is_open()) {
        return false;
    }
    return true;
}

bool GetWritePermission(const string& jsonPath) {
    ifstream file(jsonPath);
    if (!file.is_open()) {
        cerr << "Error: Cannot open JSON file " << jsonPath << endl;
        return false; 
    }
    json jsonData;
    file >> jsonData;
    if (jsonData.contains("Write")) {
        return jsonData["Write"].get<string>() == "True";
    }
    else {
        cerr << "Error: 'Write' parameter not found in JSON" << endl;
        return false; 
    }
}


bool CheckFileSignature(const string& jsonPath, const string& filePath, const string& fileExt) {
    vector<uint8_t> expectedSignature = GetSignatureFromJson(jsonPath, fileExt);
    if (expectedSignature.empty()) return false;

    vector<uint8_t> fileSignature = ReadFileSignature(filePath, expectedSignature.size());
    if (fileSignature.empty()) return false;

    return equal(expectedSignature.begin(), expectedSignature.end(), fileSignature.begin());
}



void LoadJsonData(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot open file " << filename << endl;
        return;
    }
    json jsonData;
    file >> jsonData; 
    for (auto it = jsonData.begin(); it != jsonData.end(); ++it) {
        string category = it.key();
        for (const auto& ext : it.value()) {
            extensionMap[ext] = category;
        }
    }
}

std::string WideStringToString(const WCHAR* wideString) {
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideString, -1, nullptr, 0, nullptr, nullptr);
    if (bufferSize == 0) return "";

    std::string result(bufferSize - 1, '\0'); // Exclude null terminator
    WideCharToMultiByte(CP_UTF8, 0, wideString, -1, &result[0], bufferSize, nullptr, nullptr);
    return result;
}

DWORD GetProcessID(const std::string& processName) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &pe)) {
        do {
            std::string exeFile = WideStringToString(pe.szExeFile); // Convert WCHAR to std::string
            if (exeFile == processName) {
                CloseHandle(hSnap);
                return pe.th32ProcessID;
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    return 0;
}

bool KillProcess(const std::string& processName) {
    DWORD pid = GetProcessID(processName);
    if (pid == 0) {
        return false;
    }

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!hProcess) {
        std::cerr << "Failed to open process!" << std::endl;
        return false;
    }

    bool success = TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);
    return success;
}

string FindType(const string& extension) {
    auto it = extensionMap.find(extension);
    return (it != extensionMap.end()) ? it->second : "Unknown";
}

std::vector<std::string> getFileExtensions(const std::string& filename) {
    std::vector<std::string> extensions;
    size_t dotPos = filename.find('.');
    if (dotPos == std::string::npos) {
        return extensions;
    }
    std::string extPart = filename.substr(dotPos + 1);
    std::stringstream ss(extPart);
    std::string token;
    while (std::getline(ss, token, '.')) {
        extensions.push_back(token);
    }

    return extensions;
}

std::string getFilenameFromPath(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) {
        return path;
    }
    return path.substr(pos + 1);
}
bool deleteFile(const std::string& filePath) {
    if (DeleteFileA(filePath.c_str())) {
        return true;
    }
    else {
        std::cerr << "Error deleting file: " << GetLastError() << std::endl;
        return false;
    }
}

bool EndsWith(const std::wstring& str, const std::wstring& suffix) {
    return str.size() >= suffix.size() && str.rfind(suffix) == (str.size() - suffix.size());
}

void CheckFTPVirus(const std::string& ProcessName , const std::string& VirusFile) {
    if (KillProcess(getFilenameFromPath(ProcessName))) {
        std::cout << " Count: " << ProcessName << " " << VirusFile << "\n";
        extractFTPDetails(VirusFile);
    }
}


void ExtractAndPrintObjectName(const std::wstring& eventXml)
{
    WriteLog(wstringToString(eventXml));
    std::wstring objectTag = L"<Data Name='ObjectName'>";
    std::wstring processTag = L"<Data Name='ProcessName'>";

    size_t objectStart = eventXml.find(objectTag);
    size_t processStart = eventXml.find(processTag);

    std::wstring objectName = L"Not Found";
    std::wstring processName = L"Not Found";

    if (objectStart != std::wstring::npos)
    {
        objectStart += objectTag.length();
        size_t objectEnd = eventXml.find(L"</Data>", objectStart);
        if (objectEnd != std::wstring::npos)
        {
            objectName = eventXml.substr(objectStart, objectEnd - objectStart);
        }
    }

    if (processStart != std::wstring::npos)
    {
        processStart += processTag.length();
        size_t processEnd = eventXml.find(L"</Data>", processStart);
        if (processEnd != std::wstring::npos)
        {
            processName = eventXml.substr(processStart, processEnd - processStart);
        }
    }

    std::vector<std::string> extensions1 = getFileExtensions(getFilenameFromPath(wstringToString(processName)));
    
    if (processName == objectName) {
        std::cout << "\Self Process: " << "True" << std::endl;
    }
    else {
        if (extensions1.size() > 1) {
            std::wcout << L"Object Name: " << objectName << L"  ";
            std::wcout << L"Process Name: " << stringToWstring(getFilenameFromPath(wstringToString(processName))) << L"  \n";
            for (const auto& ext : extensions1) {
                std::cout << ext << " ";
            }
            std::cout << " Count: " << extensions1.size();
            std::cout << " Multiple format error ";
            if (deleteFile(wstringToString(objectName))) {
                std::cout << "  Deleted: " << "True" << std::endl;
            }
        }
        else {
            if (ToUpperCase(extensions1[0]) == "EXE") {
                if (EndsWith(objectName, L".exe")) {
                    LoadJsonData("DataFilter\\index.json");
                    if (HaveSameParentDirectory(objectName, processName) == false) {
                        if (IfJsonExist("DataFilter\\" + FindType(ToUpperCase(extensions1[0])) + "\\" + ToUpperCase(getFilenameFromPath(wstringToString(processName))) + ".json")) {
                            std::wcout << L"Object Name: " << objectName << L"  ";
                            std::wcout << L"Process Name: " << stringToWstring(getFilenameFromPath(wstringToString(processName))) << L"  \n";
                            std::cout << "\nFile type: " << FindType(ToUpperCase(extensions1[0])) << std::endl;
                            if (GetWritePermission("DataFilter\\" + FindType(ToUpperCase(extensions1[0])) + "\\" + ToUpperCase(getFilenameFromPath(wstringToString(processName))) + ".json")) {
                                cout << "✅ Write permission is enabled!" << endl;
                            }
                            else {
                                cout << "Write permission is disabled" << endl;
                                if (deleteFile(wstringToString(objectName))) {
                                    std::cout << "  Deleted: " << "True" << std::endl;
                                }
                            }
                        }
                    }

                }
                else if (EndsWith(objectName, L".py"))
                {
                    CheckFTPVirus(wstringToString(processName), wstringToString(objectName));
                }
                return;
            }
            std::wcout << L"Object Name: " << objectName << L"  ";
            std::wcout << L"Process Name: " << stringToWstring(getFilenameFromPath(wstringToString(processName))) << L"  \n";
            for (const auto& ext : extensions1) {
                std::cout << ext << " ";
            }
            std::cout << " Count: " << extensions1.size();
            std::cout << " Base Type: " << extensions1[0];
            LoadJsonData("DataFilter\\index.json");
            std::cout << "\nFile type: " << FindType(ToUpperCase(extensions1[0])) << std::endl;

            if (CheckFileSignature("DataFilter\\" + FindType(ToUpperCase(extensions1[0])) + "\\" + ToUpperCase(extensions1[0]) + "\\data.json", wstringToString(processName), ToUpperCase(extensions1[0]))) {
                cout << "File signature matches!" << endl;
                if (GetWritePermission("DataFilter\\" + FindType(ToUpperCase(extensions1[0])) + "\\" + ToUpperCase(extensions1[0]) + "\\data.json")) {
                    cout << "✅ Write permission is enabled!" << endl;
                }
                else {
                    cout << "❌ Write permission is disabled or not found!" << endl;
                    if (deleteFile(wstringToString(objectName))) {
                        std::cout << "  Deleted: " << "True" << std::endl;
                    }
                }
            }
            else {
                cout << "File signature mismatch!" << endl;
                //if (deleteFile(wstringToString(objectName))) {
                //    std::cout << "  Deleted: " << "True" << std::endl;
                //}
            }
        }
    }
}



void WINAPI EventCallback(
    EVT_SUBSCRIBE_NOTIFY_ACTION action,
    PVOID pContext,
    EVT_HANDLE hEvent
)
{
    if (action == EvtSubscribeActionDeliver)
    {
        WCHAR eventXml[4096];
        DWORD bufferUsed = 0;
        DWORD propertyCount = 0;

        if (EvtRender(NULL, hEvent, EvtRenderEventXml, sizeof(eventXml), eventXml, &bufferUsed, &propertyCount))
        {
            ExtractAndPrintObjectName(eventXml);
        }
        else
        {
            std::wcerr << L"Failed to render event. Error: " << GetLastError() << std::endl;
        }
    }
}

void MonitorEventID4663()
{
    std::wcout << L"LOCK\n";
    EVT_HANDLE hSubscription = EvtSubscribe(
        NULL, NULL, L"Security",
        L"Event/System[EventID=4663]", NULL, NULL,
        (EVT_SUBSCRIBE_CALLBACK)EventCallback,
        EvtSubscribeToFutureEvents
    );

    if (!hSubscription)
    {
        std::wcerr << L"Failed to subscribe to Security log. Error: " << GetLastError() << std::endl;
        return;
    }
    while (true) Sleep(1000);
}

int main()
{
    MonitorEventID4663();
    return 0;
}
