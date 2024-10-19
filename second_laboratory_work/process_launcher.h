   #ifndef PROCESS_LAUNCHER_H
   #define PROCESS_LAUNCHER_H

   #ifdef _WIN32
   #include <windows.h>
   #else
   #include <unistd.h>
   #include <sys/types.h>
   #include <sys/wait.h>
   #endif

   #include <stdio.h>

   typedef struct {
       int exit_code;
       int success;
   } ProcessResult;

   ProcessResult launch_process(const char *command);

   #endif