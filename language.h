#ifndef LANGUAGE_H
#define LANGUAGE_H

typedef struct {
    const char *code;
    const char *welcome;
    const char *commands;
    const char *values;
    const char *input_prompt;
    const char *confirm_clear;
    const char *cleared;
    const char *cancelled;
    const char *saved;
    const char *goodbye;
    const char *unknown_command;
    const char *unknown_name;
    const char *invalid_value;
    const char *line_too_long;
    const char *language_changed;
    const char *unsupported_language;
    const char *defaults_loaded;
    const char *stored_values_loaded;
} language_t;

static const language_t language_en[] = {{
    "en",
    "Configuration CLI",
    "Commands: ? help, c clear, e enumerate, q quit, s [name] set, w write and quit, l <code> language",
    "Configuration values:",
    "Enter %s (blank keeps current, type \"\" to clear): ",
    "Clear stored configuration? (y/n): ",
    "Configuration storage cleared.",
    "Clear cancelled.",
    "Configuration written.",
    "Goodbye.",
    "Unknown command. Enter ? for help.",
    "Unknown configuration name.",
    "Invalid value or value too long.",
    "Input too long; try again.",
    "Language changed.",
    "Unsupported language. Available: en es",
    "Checksum invalid or no saved configuration; defaults loaded.",
    "Stored configuration loaded."
}};

static const language_t language_es[] = {{
    "es",
    "CLI de configuracion",
    "Comandos: ? ayuda, c borrar, e enumerar, q salir, s [nombre] cambiar, w guardar y salir, l <codigo> idioma",
    "Valores de configuracion:",
    "Introduzca %s (vacio conserva el valor; \"\" lo borra): ",
    "Borrar la configuracion guardada? (s/n): ",
    "Almacenamiento de configuracion borrado.",
    "Borrado cancelado.",
    "Configuracion guardada.",
    "Adios.",
    "Comando desconocido. Introduzca ? para ayuda.",
    "Nombre de configuracion desconocido.",
    "Valor no valido o demasiado largo.",
    "Entrada demasiado larga; intentelo de nuevo.",
    "Idioma cambiado.",
    "Idioma no disponible. Disponibles: en es",
    "Suma de comprobacion no valida o sin datos; se cargaron valores predeterminados.",
    "Configuracion guardada cargada."
}};

static const language_t *const languages[] = {language_en, language_es};

#endif