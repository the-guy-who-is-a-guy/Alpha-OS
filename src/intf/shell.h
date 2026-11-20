#pragma once
#include <stddef.h>
void readline(char *buf, size_t max, const char *prompt);
void handle_command(const char *cmd);