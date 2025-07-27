#include "version_info.h"
#include <stdio.h>
#include <stdbool.h>

// 宏定义检查，提供默认值以防CMake宏未定义
#ifndef BUILD_TIMESTAMP
#define BUILD_TIMESTAMP "unknown"
#endif

#ifndef GIT_BRANCH
#define GIT_BRANCH "unknown"
#endif

#ifndef GIT_COMMIT_SHORT
#define GIT_COMMIT_SHORT "unknown"
#endif

#ifndef GIT_COMMIT_FULL
#define GIT_COMMIT_FULL "unknown"
#endif

#ifndef GIT_COMMIT_DATE
#define GIT_COMMIT_DATE "unknown"
#endif

#ifndef GIT_DIRTY
#define GIT_DIRTY false
#endif

void print_version_info(void)
{
    printf("=== Firmware Version Information ===\n");
    printf("Build Time:    %s\n", BUILD_TIMESTAMP);
    printf("Git Branch:    %s\n", GIT_BRANCH);
    printf("Git Commit:    %s\n", GIT_COMMIT_SHORT);
    printf("Commit Date:   %s\n", GIT_COMMIT_DATE);
    printf("Working Dir:   %s\n", GIT_DIRTY ? "dirty" : "clean");
    printf("===================================\n");
}

const char *get_build_timestamp(void)
{
    return BUILD_TIMESTAMP;
}

const char *get_git_branch(void)
{
    return GIT_BRANCH;
}

const char *get_git_commit_id(void)
{
    return GIT_COMMIT_SHORT;
}

bool is_git_dirty(void)
{
    return GIT_DIRTY;
}