# Pico Configuration CLI

A Raspberry Pi Pico 2 W USB-serial CLI and C configuration library. Configuration values are held in RAM while the program runs and are saved to flash with a CRC-32 checksum when requested.

## AI Assistance

This project was developed with assistance from GitHub Copilot using the MAI-Code-1.1-Flash model. The AI helped with design, code generation, debugging, refactoring, and documentation review, while the final implementation and project decisions were validated by the author.

## Build and Run

This project uses the Raspberry Pi Pico SDK 2.3.0 and targets `pico2_w` by default. Open the project in VS Code with the Pico extension and run the **Compile Project** task. Put the board in BOOTSEL mode and run **Run Project** to load the resulting firmware.

Connect to the Pico USB CDC serial port at 115200 baud. The CLI waits for a USB serial connection before printing the welcome banner.

## CLI Commands

| Command | Behavior |
| --- | --- |
| `?` or `help` | Print the command list. |
| `c` | Ask for confirmation, erase saved configuration, and restore defaults in RAM. Accept `y` or `s` to confirm. |
| `e` | Enumerate current values. Names containing `password` are shown as `****`. |
| `q` | Quit without saving pending changes. |
| `s` | Prompt for each non-`first_run` value in declaration order. A blank response keeps its current value. |
| `s <name>` | Prompt for one named value. |
| `w` | Save the current values to flash and quit. |
| `l <code>` | Change the CLI language. Currently supported: `en` and `es`. |

Pressing Enter at the main prompt starts the same sequence as `s`. After a successful full sequence, `first_run` is set to `false`. For string prompts, enter `""` to clear a value. Input is locally echoed, including password input; password masking applies only to enumeration.

## Configuration Schema

The test application declares its variables in `config.h` using `CONFIG_VARIABLES(X)`. Each entry provides a type, name, default, and storage size:

```c
#define CONFIG_VARIABLES(X) \
    X(BOOL, first_run, true, 1) \
    X(STRING, ssid, "", CONFIG_SSID_MAX_LENGTH + 1) \
    X(STRING, password, "", CONFIG_PASSWORD_MAX_LENGTH + 1)
```

The current string limits are 32 characters for `ssid` and 63 for `password`, excluding the terminating NUL. Add entries to this macro to add variables; supported entry types are `BOOL` and `STRING`.

## Configuration API (`config.h`)

The live values are exposed through the global `config` object:

```c
typedef struct {
    bool first_run;
    char ssid[CONFIG_SSID_MAX_LENGTH + 1];
    char password[CONFIG_PASSWORD_MAX_LENGTH + 1];
} config_values_t;

extern config_values_t config;
```

The struct members are generated from `CONFIG_VARIABLES(X)`; the declaration above shows the current test configuration.

| Function | Description |
| --- | --- |
| `bool config_init(void)` | Load and validate the saved record. Returns `true` if valid saved values were loaded; otherwise restores defaults and returns `false`. |
| `size_t config_count(void)` | Return the number of declared variables. |
| `const char *config_name(size_t index)` | Return a variable name, or `NULL` if the index is out of range. |
| `config_type_t config_type(size_t index)` | Return `CONFIG_TYPE_BOOL` or `CONFIG_TYPE_STRING`. Out-of-range indices return `CONFIG_TYPE_STRING`. |
| `bool config_get_value(size_t index, char *buffer, size_t buffer_size)` | Format a value into the caller’s buffer (`true`/`false` for booleans); returns `false` for invalid indices, invalid buffers, or insufficient capacity. |
| `bool config_set_value(const char *name, const char *value)` | Set a value by name. Boolean text accepts `true`, `false`, `1`, or `0`; names are ASCII case-insensitive. For strings, `""` clears the value. Returns `false` for unknown names, invalid booleans, or strings that exceed capacity. |
| `bool config_save(void)` | Erase and program the reserved flash area with the current values and CRC-32. |
| `bool config_clear_storage(void)` | Erase the saved record and reset live values to defaults; it does not save those defaults as a record. |
| `bool config_is_dirty(void)` | Report whether a value changed since load or the last save/clear. |

Call `config_init()` before using the current values. Configuration is not saved automatically; call `config_save()` to persist changes.

## CLI Parsing API (`ConfigParse.h`)

These helpers implement the USB CLI behavior. `config_parse_command()` tokenizes its input buffer in place and returns the remaining argument through `argument`.

| Function | Description |
| --- | --- |
| `bool config_parse_read_line(char *buffer, size_t buffer_size, const language_t *language)` | Read one line from Pico stdio, echo printable characters, support backspace, and report overlong input. |
| `config_command_t config_parse_command(char *line, char **argument)` | Parse a command into a `CONFIG_COMMAND_*` value. |
| `const language_t *config_parse_language(const char *code)` | Find a supported language by its two-letter code, or return `NULL`. |
| `void config_parse_enumerate_values(const language_t *language)` | Print all values, masking password-named variables. |
| `bool config_parse_set_one_value(const char *name, const language_t *language)` | Prompt for and update one variable. Blank input leaves it unchanged. |
| `void config_parse_set_all_values(const language_t *language)` | Prompt for all values except `first_run`; sets `first_run` false if all prompts complete successfully. |
| `void config_parse_clear_storage(const language_t *language)` | Prompt for confirmation, then clear storage and restore defaults. |

`CONFIG_PARSE_INPUT_BUFFER_SIZE` is the CLI input-line capacity, including the terminating NUL.

## Languages (`language.h`)

`language_t` groups the CLI text used for welcome/help, prompts, errors, and status messages. The header currently defines the static `language_en` and `language_es` arrays and the `languages` lookup table. Add a `language_t` record and include it in `languages` to add another CLI language.

## Flash Storage

The current CMake configuration targets a 4 MB Pico 2 W flash device. It reduces the linker-visible flash size by 12 KB, reserving the final three 4 KB sectors for configuration. A 10 KB image is programmed; the extra 2 KB provides sector-aligned erase space. The record contains a format marker, version, value size, values, and CRC-32. Invalid or uninitialized records cause defaults to be used.

If the target board or flash capacity changes, update the `pico_override_flash_size()` value and `CONFIG_FLASH_ERASE_BYTES` together. The config area must remain outside the firmware image and erase operations must remain sector-aligned.