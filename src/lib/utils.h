#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/* Memory management wrappers that exit on failure */
void *xmalloc(size_t size);
void *xcalloc(size_t count, size_t size);
void *xrealloc(void *ptr, size_t size);

/* Error reporting helper */
void set_error(char *error_buf, const char *fmt, ...);

#endif /* UTILS_H */
