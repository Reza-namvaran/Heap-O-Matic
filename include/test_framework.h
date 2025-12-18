#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>

#define TEST_ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        return 1; \
    } \
} while (0)

#define TEST_RUN(test_func) do { \
    printf("Running %s ... ", #test_func); \
    int res = test_func(); \
    if (res == 0) printf("PASS\n") \
}

#endif