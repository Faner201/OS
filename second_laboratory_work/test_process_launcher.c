   #include <stdio.h>
   #include "process_launcher.h"

   int main() {
       const char *command = "ping -c 1 127.0.0.1";
       ProcessResult result = launch_process(command);

       if (result.success) {
           printf("Process completed successfully with exit code: %d\n", result.exit_code);
       } else {
           printf("Process failed with error code: %d\n", result.exit_code);
       }

       return 0;
   }