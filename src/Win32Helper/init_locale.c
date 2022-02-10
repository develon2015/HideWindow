#include <locale.h>

void init_locale(const char *locale) {
    setlocale(LC_ALL, locale);
}