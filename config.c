#include "config.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico.h"

#ifndef CONFIG_STORAGE_BYTES
#define CONFIG_STORAGE_BYTES (10u * 1024u)
#endif

#ifndef CONFIG_FLASH_ERASE_BYTES
#define CONFIG_FLASH_ERASE_BYTES (12u * 1024u)
#endif

#define CONFIG_FLASH_REGION_OFFSET (PICO_FLASH_SIZE_BYTES - CONFIG_FLASH_ERASE_BYTES)
#define CONFIG_FLASH_STORAGE_ADDR (XIP_BASE + CONFIG_FLASH_REGION_OFFSET)

#define CONFIG_MAGIC 0x43464731u
#define CONFIG_VERSION 1u

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t values_size;
    uint32_t checksum;
    config_values_t values;
} config_flash_record_t;

typedef struct {
    const char *name;
    config_type_t type;
    size_t offset;
    size_t size;
} config_field_t;

config_values_t config;

static const config_field_t config_fields[] = {
#define CONFIG_FIELD_BOOL(name, initial, size) {#name, CONFIG_TYPE_BOOL, offsetof(config_values_t, name), sizeof(bool)},
#define CONFIG_FIELD_STRING(name, initial, size) {#name, CONFIG_TYPE_STRING, offsetof(config_values_t, name), sizeof(((config_values_t *)0)->name)},
#define CONFIG_FIELD(type, name, initial, size) CONFIG_FIELD_##type(name, initial, size)
    CONFIG_VARIABLES(CONFIG_FIELD)
#undef CONFIG_FIELD
#undef CONFIG_FIELD_BOOL
#undef CONFIG_FIELD_STRING
};

static bool config_dirty;
static uint8_t flash_image[CONFIG_STORAGE_BYTES] __attribute__((aligned(FLASH_PAGE_SIZE)));

_Static_assert(CONFIG_STORAGE_BYTES % FLASH_PAGE_SIZE == 0, "Config storage must be page aligned");
_Static_assert(CONFIG_FLASH_ERASE_BYTES % FLASH_SECTOR_SIZE == 0, "Config erase size must be sector aligned");
_Static_assert(CONFIG_FLASH_ERASE_BYTES >= CONFIG_STORAGE_BYTES, "Flash erase area must cover config storage");
_Static_assert(sizeof(config_flash_record_t) <= CONFIG_STORAGE_BYTES, "Config record exceeds reserved storage");

static void config_set_defaults(void)
{
    memset(&config, 0, sizeof(config));
#define CONFIG_DEFAULT_BOOL(name, initial, size) config.name = initial;
#define CONFIG_DEFAULT_STRING(name, initial, size) memcpy(config.name, initial, sizeof(initial));
#define CONFIG_DEFAULT(type, name, initial, size) CONFIG_DEFAULT_##type(name, initial, size)
    CONFIG_VARIABLES(CONFIG_DEFAULT)
#undef CONFIG_DEFAULT
#undef CONFIG_DEFAULT_BOOL
#undef CONFIG_DEFAULT_STRING
}

static uint32_t config_crc32(const uint8_t *data, size_t length)
{
    uint32_t crc = 0xffffffffu;

    for (size_t index = 0; index < length; ++index) {
        crc ^= data[index];
        for (unsigned int bit = 0; bit < 8; ++bit) {
            uint32_t mask = 0u - (crc & 1u);
            crc = (crc >> 1) ^ (0xedb88320u & mask);
        }
    }

    return ~crc;
}

static bool config_ascii_equal(const char *left, const char *right)
{
    while (*left != '\0' && *right != '\0') {
        char left_char = *left++;
        char right_char = *right++;
        if (left_char >= 'A' && left_char <= 'Z') {
            left_char = (char)(left_char - 'A' + 'a');
        }
        if (right_char >= 'A' && right_char <= 'Z') {
            right_char = (char)(right_char - 'A' + 'a');
        }
        if (left_char != right_char) {
            return false;
        }
    }
    return *left == *right;
}

static const config_field_t *config_find_field(const char *name)
{
    for (size_t index = 0; index < sizeof(config_fields) / sizeof(config_fields[0]); ++index) {
        if (config_ascii_equal(name, config_fields[index].name)) {
            return &config_fields[index];
        }
    }
    return NULL;
}

bool config_init(void)
{
    config_set_defaults();
    config_dirty = false;

    const uint8_t *stored = (const uint8_t *)CONFIG_FLASH_STORAGE_ADDR;
    const config_flash_record_t *record = (const config_flash_record_t *)stored;
    const size_t values_offset = offsetof(config_flash_record_t, values);
    uint8_t first_run_value = stored[values_offset + offsetof(config_values_t, first_run)];

    if (record->magic != CONFIG_MAGIC || record->version != CONFIG_VERSION ||
        record->values_size != sizeof(config_values_t) ||
        record->checksum != config_crc32(stored + values_offset, CONFIG_STORAGE_BYTES - values_offset) ||
        first_run_value > 1 ||
        memchr(record->values.ssid, '\0', sizeof(record->values.ssid)) == NULL ||
        memchr(record->values.password, '\0', sizeof(record->values.password)) == NULL) {
        return false;
    }

    memcpy(&config, &record->values, sizeof(config));
    return true;
}

size_t config_count(void)
{
    return sizeof(config_fields) / sizeof(config_fields[0]);
}

const char *config_name(size_t index)
{
    return index < config_count() ? config_fields[index].name : NULL;
}

config_type_t config_type(size_t index)
{
    return index < config_count() ? config_fields[index].type : CONFIG_TYPE_STRING;
}

bool config_get_value(size_t index, char *buffer, size_t buffer_size)
{
    if (index >= config_count() || buffer == NULL || buffer_size == 0) {
        return false;
    }

    const config_field_t *field = &config_fields[index];
    const uint8_t *value = (const uint8_t *)&config + field->offset;

    if (field->type == CONFIG_TYPE_BOOL) {
        int length = snprintf(buffer, buffer_size, "%s", *(const bool *)value ? "true" : "false");
        return length >= 0 && (size_t)length < buffer_size;
    }

    size_t length = strnlen((const char *)value, field->size);
    if (length >= buffer_size || length == field->size) {
        return false;
    }
    memcpy(buffer, value, length + 1);
    return true;
}

bool config_set_value(const char *name, const char *value)
{
    if (name == NULL || value == NULL) {
        return false;
    }

    const config_field_t *field = config_find_field(name);
    if (field == NULL) {
        return false;
    }

    uint8_t *destination = (uint8_t *)&config + field->offset;
    if (field->type == CONFIG_TYPE_BOOL) {
        bool parsed_value;
        if (config_ascii_equal(value, "true") || strcmp(value, "1") == 0) {
            parsed_value = true;
        } else if (config_ascii_equal(value, "false") || strcmp(value, "0") == 0) {
            parsed_value = false;
        } else {
            return false;
        }
        if (*(bool *)destination != parsed_value) {
            *(bool *)destination = parsed_value;
            config_dirty = true;
        }
        return true;
    }

    const char *string_value = value;
    size_t value_length = strlen(value);
    if (value_length == 2 && value[0] == '"' && value[1] == '"') {
        string_value = "";
        value_length = 0;
    }
    if (value_length >= field->size) {
        return false;
    }
    if (strcmp((const char *)destination, string_value) != 0) {
        memset(destination, 0, field->size);
        memcpy(destination, string_value, value_length);
        config_dirty = true;
    }
    return true;
}

bool config_save(void)
{
    memset(flash_image, 0xff, sizeof(flash_image));
    config_flash_record_t *record = (config_flash_record_t *)flash_image;
    record->magic = CONFIG_MAGIC;
    record->version = CONFIG_VERSION;
    record->values_size = sizeof(config_values_t);
    memcpy(&record->values, &config, sizeof(config));
    record->checksum = config_crc32(flash_image + offsetof(config_flash_record_t, values),
                                    CONFIG_STORAGE_BYTES - offsetof(config_flash_record_t, values));

    uint32_t interrupt_state = save_and_disable_interrupts();
    flash_range_erase(CONFIG_FLASH_STORAGE_ADDR - XIP_BASE, CONFIG_FLASH_ERASE_BYTES);
    flash_range_program(CONFIG_FLASH_STORAGE_ADDR - XIP_BASE, flash_image, CONFIG_STORAGE_BYTES);
    restore_interrupts(interrupt_state);

    config_dirty = false;
    return true;
}

bool config_clear_storage(void)
{
    uint32_t interrupt_state = save_and_disable_interrupts();
    flash_range_erase(CONFIG_FLASH_STORAGE_ADDR - XIP_BASE, CONFIG_FLASH_ERASE_BYTES);
    restore_interrupts(interrupt_state);

    config_set_defaults();
    config_dirty = false;
    return true;
}

bool config_is_dirty(void)
{
    return config_dirty;
}