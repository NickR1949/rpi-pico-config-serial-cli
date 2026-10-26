#include <stdbool.h>
#include <stdio.h>

#include "config.h"
#include "ConfigParse.h"
#include "language.h"
#include "pico/stdlib.h"

static const language_t *active_language = language_en;

int main(void)
{
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(10);
    }

    bool loaded = config_init();
    printf("%s\r\n%s\r\n", active_language->welcome,
           loaded ? active_language->stored_values_loaded : active_language->defaults_loaded);
    config_parse_enumerate_values(active_language);

    char line[CONFIG_PARSE_INPUT_BUFFER_SIZE];
    for (;;) {
        printf("> ");
        if (!config_parse_read_line(line, sizeof(line), active_language)) {
            continue;
        }

        char *argument;
        config_command_t command = config_parse_command(line, &argument);
        if (command == CONFIG_COMMAND_EMPTY) {
            config_parse_set_all_values(active_language);
        } else if (command == CONFIG_COMMAND_HELP) {
            printf("%s\r\n", active_language->commands);
        } else if (command == CONFIG_COMMAND_CLEAR) {
            config_parse_clear_storage(active_language);
        } else if (command == CONFIG_COMMAND_ENUMERATE) {
            config_parse_enumerate_values(active_language);
        } else if (command == CONFIG_COMMAND_QUIT) {
            printf("%s\r\n", active_language->goodbye);
            return 0;
        } else if (command == CONFIG_COMMAND_SET) {
            if (*argument == '\0') {
                config_parse_set_all_values(active_language);
            } else {
                config_parse_set_one_value(argument, active_language);
            }
        } else if (command == CONFIG_COMMAND_WRITE) {
            if (config_save()) {
                printf("%s\r\n", active_language->saved);
                return 0;
            }
        } else if (command == CONFIG_COMMAND_LANGUAGE) {
            const language_t *language = config_parse_language(argument);
            if (language != NULL) {
                active_language = language;
                printf("%s\r\n", active_language->language_changed);
            } else {
                printf("%s\r\n", active_language->unsupported_language);
            }
        } else {
            printf("%s\r\n", active_language->unknown_command);
        }
    }
}
