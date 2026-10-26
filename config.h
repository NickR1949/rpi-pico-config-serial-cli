#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stddef.h>

#define CONFIG_SSID_MAX_LENGTH 32
#define CONFIG_PASSWORD_MAX_LENGTH 63

#define CONFIG_VARIABLES(X) \
    X(BOOL, first_run, true, 1) \
    X(STRING, ssid, "", CONFIG_SSID_MAX_LENGTH + 1) \
    X(STRING, password, "", CONFIG_PASSWORD_MAX_LENGTH + 1)

#define CONFIG_DECLARE_BOOL(name, initial, size) bool name;
#define CONFIG_DECLARE_STRING(name, initial, size) char name[size];
#define CONFIG_DECLARE(type, name, initial, size) CONFIG_DECLARE_##type(name, initial, size)

typedef struct {
    CONFIG_VARIABLES(CONFIG_DECLARE)
} config_values_t;

#undef CONFIG_DECLARE
#undef CONFIG_DECLARE_BOOL
#undef CONFIG_DECLARE_STRING

typedef enum {
    CONFIG_TYPE_BOOL,
    CONFIG_TYPE_STRING
} config_type_t;

extern config_values_t config;

bool config_init(void);
size_t config_count(void);
const char *config_name(size_t index);
config_type_t config_type(size_t index);
bool config_get_value(size_t index, char *buffer, size_t buffer_size);
bool config_set_value(const char *name, const char *value);
bool config_save(void);
bool config_clear_storage(void);
bool config_is_dirty(void);

#endif