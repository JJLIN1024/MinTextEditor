#ifndef __dbg_h__
#define __dbg_h__

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err(M, ...)                                                       \
  ({                                                                          \
    write(STDOUT_FILENO, "\x1b[2J", 4);                                       \
    write(STDOUT_FILENO, "\x1b[H", 3);                                        \
    fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, \
            clean_errno(), ##__VA_ARGS__);                                    \
    exit(1);                                                                  \
  })

#define log_info(M, ...)                                                     \
  ({                                                                         \
    write(STDOUT_FILENO, "\x1b[2J", 4);                                      \
    write(STDOUT_FILENO, "\x1b[H", 3);                                       \
    fprintf(stderr, "[INFO] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, \
            clean_errno(), ##__VA_ARGS__);                                   \
    exit(1);                                                                 \
  })

#define log_warn(M, ...)                                                     \
  ({                                                                         \
    write(STDOUT_FILENO, "\x1b[2J", 4);                                      \
    write(STDOUT_FILENO, "\x1b[H", 3);                                       \
    fprintf(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, \
            clean_errno(), ##__VA_ARGS__);                                   \
    exit(1);                                                                 \
  })

#define check(A, M, ...)       \
  if ((A)) {                   \
    log_err(M, ##__VA_ARGS__); \
  }

#endif
