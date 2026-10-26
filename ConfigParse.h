#ifndef CONFIG_PARSE_H
#define CONFIG_PARSE_H

#include <stdbool.h>
#include <stddef.h>

#include "language.h"

#define CONFIG_PARSE_INPUT_BUFFER_SIZE 160

typedef enum {
    CONFIG_COMMAND_EMPTY,
    CONFIG_COMMAND_HELP,
    CONFIG_COMMAND_CLEAR,
    CONFIG_COMMAND_ENUMERATE,
    CONFIG_COMMAND_QUIT,
    CONFIG_COMMAND_SET,
    CONFIG_COMMAND_WRITE,
    CONFIG_COMMAND_LANGUAGE,
    CONFIG_COMMAND_INVALID
} config_command_t;

bool config_parse_read_line(char *buffer, size_t buffer_size, const language_t *language);
config_command_t config_parse_command(char *line, char **argument);
const language_t *config_parse_language(const char *code);
void config_parse_enumerate_values(const language_t *language);
bool config_parse_set_one_value(const char *name, const language_t *language);
void config_parse_set_all_values(const language_t *language);
void config_parse_clear_storage(const language_t *language);

#endif