#pragma once
#include <stdint.h>
int serial_init(void);
void serial_write(char a);
void serial_write_string(const char* str);
void serial_write_hex64(uint64_t value);