   #include "process_launcher.h"

   ProcessResult launch_process(const char *command) {
       ProcessResult result;
       result.success = 0;
       
   #ifdef _WIN32
       STARTUPINFO si;
       PROCESS_INFORMATION pi;

       ZeroMemory(&si, sizeof(si));
       si.cb = sizeof(si);
       ZeroMemory(&pi, sizeof(pi));

       if (CreateProcess(NULL,  
                         (LPSTR)command,      
                         NULL,       
                         NULL,      
                         FALSE,      
                         0,           
                         NULL,        
                         NULL,       
                         &si,        
                         &pi)         
          ) {
           WaitForSingleObject(pi.hProcess, INFINITE);
           GetExitCodeProcess(pi.hProcess, &result.exit_code);
           result.success = 1;
           CloseHandle(pi.hProcess);
           CloseHandle(pi.hThread);
       } else {
           result.exit_code = GetLastError();
       }
   #else
       pid_t pid = fork();
       if (pid == 0) {
           execl("/bin/sh", "sh", "-c", command, (char *)NULL);
           _exit(1);
       } else if (pid > 0) {
           int status;
           waitpid(pid, &status, 0);
           if (WIFEXITED(status)) {
               result.exit_code = WEXITSTATUS(status);
               result.success = 1;
           } else {
               result.exit_code = -1;
           }
       } else {
           result.exit_code = -1; 
       }
   #endif
       return result;
   }