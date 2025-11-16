#include "string_utils.h"
#include <ctype.h>

// Sanitize filename by removing invalid characters
char* string_sanitize_filename(const char *filename) {
    if (!filename) return NULL;

    char *sanitized = g_strdup(filename);
    char invalid_chars[] = {'/', '\\', ':', '*', '?', '"', '<', '>', '|', '\0'};

    for (size_t i = 0; i < strlen(sanitized); i++) {
        for (size_t j = 0; invalid_chars[j] != '\0'; j++) {
            if (sanitized[i] == invalid_chars[j]) {
                sanitized[i] = '_';
                break;
            }
        }
    }

    return sanitized;
}

// Trim whitespace from both ends
char* string_trim(const char *str) {
    if (!str) return NULL;

    while (isspace(*str)) str++;

    if (*str == '\0') return g_strdup("");

    const char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;

    return g_strndup(str, end - str + 1);
}

// Format seconds as HH:MM:SS
char* string_format_duration(int seconds) {
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;

    if (hours > 0) {
        return g_strdup_printf("%02d:%02d:%02d", hours, minutes, secs);
    } else {
        return g_strdup_printf("%02d:%02d", minutes, secs);
    }
}

// Parse time string (HH:MM:SS or MM:SS) to seconds
int string_parse_time(const char *time_str) {
    if (!time_str) return 0;

    int hours = 0, minutes = 0, seconds = 0;
    int parts = sscanf(time_str, "%d:%d:%d", &hours, &minutes, &seconds);

    if (parts == 3) {
        return hours * 3600 + minutes * 60 + seconds;
    } else if (parts == 2) {
        return hours * 60 + minutes;  // hours is actually minutes here
    } else if (parts == 1) {
        return hours;  // Just seconds
    }

    return 0;
}

// Format file size in human-readable format
char* string_format_size(int64_t bytes) {
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size = (double)bytes;

    while (size >= 1024.0 && unit_index < 4) {
        size /= 1024.0;
        unit_index++;
    }

    if (unit_index == 0) {
        return g_strdup_printf("%d %s", (int)size, units[unit_index]);
    } else {
        return g_strdup_printf("%.2f %s", size, units[unit_index]);
    }
}

// Check if string is a valid URL
gboolean string_is_valid_url(const char *url) {
    if (!url || strlen(url) < 7) return FALSE;

    return (g_str_has_prefix(url, "http://") ||
            g_str_has_prefix(url, "https://") ||
            g_str_has_prefix(url, "ftp://"));
}

// Extract domain from URL
char* string_extract_domain(const char *url) {
    if (!string_is_valid_url(url)) return NULL;

    const char *start = strstr(url, "://");
    if (!start) return NULL;

    start += 3;  // Skip "://"

    const char *end = strchr(start, '/');
    if (!end) end = start + strlen(start);

    // Remove www. if present
    if (g_str_has_prefix(start, "www.")) {
        start += 4;
    }

    return g_strndup(start, end - start);
}

// Escape string for shell command
char* string_shell_escape(const char *str) {
    if (!str) return NULL;

    GString *escaped = g_string_new("");

    for (const char *p = str; *p; p++) {
        if (*p == '\'' || *p == '"' || *p == '\\' || *p == '$' || *p == '`') {
            g_string_append_c(escaped, '\\');
        }
        g_string_append_c(escaped, *p);
    }

    return g_string_free(escaped, FALSE);
}

// Join array of strings with separator
char* string_join(char **strings, int count, const char *separator) {
    if (!strings || count <= 0) return g_strdup("");

    GString *result = g_string_new(strings[0]);

    for (int i = 1; i < count; i++) {
        if (separator) {
            g_string_append(result, separator);
        }
        g_string_append(result, strings[i]);
    }

    return g_string_free(result, FALSE);
}

// Replace all occurrences of substring
char* string_replace_all(const char *str, const char *old_substr, const char *new_substr) {
    if (!str || !old_substr || !new_substr) return g_strdup(str);

    GString *result = g_string_new("");
    const char *p = str;
    size_t old_len = strlen(old_substr);

    while (*p) {
        const char *found = strstr(p, old_substr);
        if (found) {
            g_string_append_len(result, p, found - p);
            g_string_append(result, new_substr);
            p = found + old_len;
        } else {
            g_string_append(result, p);
            break;
        }
    }

    return g_string_free(result, FALSE);
}
