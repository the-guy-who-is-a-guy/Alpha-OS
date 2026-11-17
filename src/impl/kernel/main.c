#include "print.h"
void kernel_main(){
    print_clear();
    print_set_color(COLOR_YELLOW, COLOR_BLACK);
    print_string("ALPHA OS - BOOTED");
}