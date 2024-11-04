#include <stdio.h>
#include "multi_process_launcher.h"

int main() {
    start_process_launcher();

    while (1) {
        int new_value;
        printf("Enter new counter value: ");
        scanf("%d", &new_value);
        set_counter(new_value);
    }

    return 0;
}