#pragma once
#include <stddef.h>
#define MAX_ARGS 32
void readline(char *buf, size_t max, const char *prompt);
void handle_command(int argc, char *argv[]);
int parse_args(char *line, char *argv[MAX_ARGS]);