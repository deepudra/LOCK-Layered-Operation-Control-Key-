#include <windows.h>
#include <evntcons.h>
#include <iostream>

void StartETWListener() {
    EVENT_TRACE_LOGFILE trace;
    ZeroMemory(&trace, sizeof(trace));

    trace.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
    trace.ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD;
    trace.LoggerName = (LPWSTR)L"NT Kernel Logger"; // Ensure correct session name

    TRACEHANDLE hTrace = OpenTrace(&trace);
    if (hTrace == INVALID_PROCESSTRACE_HANDLE) {
        std::cout << "[ERROR] Failed to start ETW Trace!" << std::endl;
        return;
    }

    std::cout << "[INFO] Monitoring system calls... Press Ctrl+C to stop." << std::endl;
    ProcessTrace(&hTrace, 1, 0, 0);  // Process events
}

int main() {
    std::cout << "[INFO] Starting ETW System Call Monitor..." << std::endl;
    StartETWListener();

    std::cout << "Press Enter to exit...";
    std::cin.get();
    return 0;
}
