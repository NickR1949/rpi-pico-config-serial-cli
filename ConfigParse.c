#include "ConfigParse.h"

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "pico/stdlib.h"

static bool swallow_line_feed;

static char *skip_spaces(char *text)
{
    while (*text == ' ' || *text == '\t') {
        ++text;
    }
    return text;
}

static bool contains_password(const char *name)
{
    for (const char *start = name; *start != '\0'; ++start) {
        static const char password[] = "password";
        size_t index = 0;
        while (password[index] != '\0' && start[index] != '\0') {
            char value = start[index];
            if (value >= 'A' && value <= 'Z') {
                value = (char)(value - 'A' + 'a');
            }
            if (value != password[index]) {
                break;
            }
            ++index;
        }
        if (password[index] == '\0') {
            return true;
        }
    }
    return false;
}

bool config_parse_read_line(char *buffer, size_t buffer_size, const language_t *language)
{
    size_t length = 0;
    bool overflow = false;

    for (;;) {
        int input = getchar_timeout_us(10000);
        if (input == PICO_ERROR_TIMEOUT) {
            tight_loop_contents();
            continue;
        }
        if (swallow_line_feed) {
            swallow_line_feed = false;
            if (input == '\n') {
                continue;
            }
        }
        if (input == '\r' || input == '\n') {
            swallow_line_feed = input == '\r';
            printf("\r\n");
            break;
        }
        if (input == 8 || input == 127) {
            if (length > 0) {
                --length;
                printf("\b \b");
            }
            continue;
        }
        if (input >= 32 && input <= 126) {
            if (length + 1 < buffer_size) {
                buffer[length++] = (char)input;
                putchar((char)input);
            } else {
                overflow = true;
            }
        }
    }

    buffer[length] = '\0';
    if (overflow) {
        printf("%s\r\n", language->line_too_long);
        return false;
    }
    return true;
}

config_command_t config_parse_command(char *line, char **argument)
{
    char *command = skip_spaces(line);
    char *value = command;
    while (*value != '\0' && *value != ' ' && *value != '\t') {
        ++value;
    }
    if (*value != '\0') {
        *value++ = '\0';
        value = skip_spaces(value);
    }
    *argument = value;

    if (*command == '\0') {
        return CONFIG_COMMAND_EMPTY;
    }
    if (strcmp(command, "?") == 0 || strcmp(command, "help") == 0) {
        return CONFIG_COMMAND_HELP;
    }
    if (strcmp(command, "c") == 0) {
        return CONFIG_COMMAND_CLEAR;
    }
    if (strcmp(command, "e") == 0) {
        return CONFIG_COMMAND_ENUMERATE;
    }
    if (strcmp(command, "q") == 0) {
        return CONFIG_COMMAND_QUIT;
    }
    if (strcmp(command, "s") == 0) {
        return CONFIG_COMMAND_SET;
    }
    if (strcmp(command, "w") == 0) {
        return CONFIG_COMMAND_WRITE;
    }
    if (strcmp(command, "l") == 0 && **argument != '\0') {
        return CONFIG_COMMAND_LANGUAGE;
    }
    return CONFIG_COMMAND_INVALID;
}

const language_t *config_parse_language(const char *code)
{
    for (size_t index = 0; index < sizeof(languages) / sizeof(languages[0]); ++index) {
        if (strcmp(code, languages[index]->code) == 0) {
            return languages[index];
        }
    }
    return NULL;
}

void config_parse_enumerate_values(const language_t *language)
{
    char value[CONFIG_PASSWORD_MAX_LENGTH + 1];
    printf("%s\r\n", language->values);
    for (size_t index = 0; index < config_count(); ++index) {
        const char *name = config_name(index);
        if (contains_password(name)) {
            printf("%s = ****\r\n", name);
        } else if (config_get_value(index, value, sizeof(value))) {
            printf("%s = %s\r\n", name, value);
        }
    }
}

bool config_parse_set_one_value(const char *name, const language_t *language)
{
    size_t index;
    for (index = 0; index < config_count(); ++index) {
        if (strcmp(name, config_name(index)) == 0) {
            break;
        }
    }
    if (index == config_count()) {
        printf("%s\r\n", language->unknown_name);
        return false;
    }

    char input[CONFIG_PARSE_INPUT_BUFFER_SIZE];
    printf(language->input_prompt, name);
    if (!config_parse_read_line(input, sizeof(input), language)) {
        return false;
    }
    if (input[0] == '\0') {
        return true;
    }
    if (!config_set_value(name, input)) {
        printf("%s\r\n", language->invalid_value);
        return false;
    }
    return true;
}

void config_parse_set_all_values(const language_t *language)
{
    bool completed = true;
    for (size_t index = 0; index < config_count(); ++index) {
        const char *name = config_name(index);
        if (strcmp(name, "first_run") == 0) {
            continue;
        }
        if (!config_parse_set_one_value(name, language)) {
            completed = false;
        }
    }
    if (completed) {
        config_set_value("first_run", "false");
    }
}

void config_parse_clear_storage(const language_t *language)
{
    char answer[CONFIG_PARSE_INPUT_BUFFER_SIZE];
    printf("%s", language->confirm_clear);
    if (!config_parse_read_line(answer, sizeof(answer), language)) {
        return;
    }
    if ((answer[0] == 'y' || answer[0] == 'Y' || answer[0] == 's' || answer[0] == 'S') &&
        answer[1] == '\0') {
        config_clear_storage();
        printf("%s\r\n", language->cleared);
    } else {
        printf("%s\r\n", language->cancelled);
    }
}