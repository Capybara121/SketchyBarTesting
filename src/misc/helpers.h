#ifndef HELPERS_H
#define HELPERS_H

extern AXError _AXUIElementGetWindow(AXUIElementRef ref, uint32_t *wid);

static const char *bool_str[] = { "off", "on" };

struct signal_args
{
    char name[2][255];
    char value[2][255];
    void *entity;
    void *param1;
};

struct rgba_color
{
    bool is_valid;
    uint32_t p;
    float r;
    float g;
    float b;
    float a;
};

static struct rgba_color
rgba_color_from_hex(uint32_t color)
{
    struct rgba_color result;
    result.is_valid = true;
    result.p = color;
    result.r = ((color >> 16) & 0xff) / 255.0;
    result.g = ((color >> 8) & 0xff) / 255.0;
    result.b = ((color >> 0) & 0xff) / 255.0;
    result.a = ((color >> 24) & 0xff) / 255.0;
    return result;
}

static inline bool is_root(void)
{
    return getuid() == 0 || geteuid() == 0;
}

static inline bool string_equals(const char *a, const char *b)
{
    return a && b && strcmp(a, b) == 0;
}

static inline char *string_escape_quote(char *s)
{
    if (!s) return NULL;

    char *cursor = s;
    int num_quotes = 0;

    while (*cursor) {
        if (*cursor == '"') ++num_quotes;
        ++cursor;
    }

    if (!num_quotes) return NULL;

    int size_in_bytes = (int)(cursor - s) + num_quotes;
    char *result = malloc(sizeof(char) * (size_in_bytes+1));
    result[size_in_bytes] = '\0';

    for (char *dst = result, *cursor = s; *cursor; ++cursor) {
        if (*cursor == '"') *dst++ = '\\';
        *dst++ = *cursor;
    }

    return result;
}

static inline char *cfstring_copy(CFStringRef string)
{
    CFIndex num_bytes = CFStringGetMaximumSizeForEncoding(CFStringGetLength(string), kCFStringEncodingUTF8);
    char *result = malloc(num_bytes + 1);
    if (!result) return NULL;

    if (!CFStringGetCString(string, result, num_bytes + 1, kCFStringEncodingUTF8)) {
        free(result);
        result = NULL;
    }

    return result;
}

static inline char *string_copy(char *s)
{
    int length = strlen(s);
    char *result = malloc(length + 1);
    if (!result) return NULL;

    memcpy(result, s, length);
    result[length] = '\0';
    return result;
}

static inline bool file_exists(char *filename)
{
    struct stat buffer;

    if (stat(filename, &buffer) != 0) {
        return false;
    }

    if (buffer.st_mode & S_IFDIR) {
        return false;
    }

    return true;
}

static inline bool ensure_executable_permission(char *filename)
{
    struct stat buffer;

    if (stat(filename, &buffer) != 0) {
        return false;
    }

    bool is_executable = buffer.st_mode & S_IXUSR;
    if (!is_executable && chmod(filename, S_IXUSR | buffer.st_mode) != 0) {
        return false;
    }

    return true;
}

static bool fork_exec(char *command, struct signal_args *args)
{
    int pid = fork();
    if (pid == -1) return false;
    if (pid !=  0) return true;

    if (args) {
        if (*args->name[0]) setenv(args->name[0], args->value[0], 1);
        if (*args->name[1]) setenv(args->name[1], args->value[1], 1);
    }

    char *exec[] = { "/usr/bin/env", "sh", "-c", command, NULL};
    exit(execvp(exec[0], exec));
}

static bool ax_privilege(void)
{
    const void *keys[] = { kAXTrustedCheckOptionPrompt };
    const void *values[] = { kCFBooleanTrue };
    CFDictionaryRef options = CFDictionaryCreate(NULL, keys, values, array_count(keys), &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    bool result = AXIsProcessTrustedWithOptions(options);
    CFRelease(options);
    return result;
}

static inline uint32_t ax_window_id(AXUIElementRef ref)
{
    uint32_t wid = 0;
    _AXUIElementGetWindow(ref, &wid);
    return wid;
}

static inline pid_t ax_window_pid(AXUIElementRef ref)
{
    return *(pid_t *)((void *) ref + 0x10);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
static inline bool psn_equals(ProcessSerialNumber *a, ProcessSerialNumber *b)
{
    Boolean result;
    SameProcess(a, b, &result);
    return result == 1;
}
#pragma clang diagnostic pop

static inline float clampf_range(float value, float min, float max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

#endif
