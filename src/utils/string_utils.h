#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include "common.h"

// Filename and path utilities
char* string_sanitize_filename(const char *filename);
char* string_trim(const char *str);

// Time formatting
char* string_format_duration(int seconds);
int string_parse_time(const char *time_str);

// Size formatting
char* string_format_size(int64_t bytes);

// URL utilities
gboolean string_is_valid_url(const char *url);
char* string_extract_domain(const char *url);

// Shell utilities
char* string_shell_escape(const char *str);

// String manipulation
char* string_join(char **strings, int count, const char *separator);
char* string_replace_all(const char *str, const char *old_substr, const char *new_substr);

#endif
