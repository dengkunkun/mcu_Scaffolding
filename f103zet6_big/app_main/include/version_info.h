#ifndef VERSION_INFO_H
#define VERSION_INFO_H

#include <stdbool.h> // 添加bool类型支持

#ifdef __cplusplus
extern "C"
{
#endif

    // 编译时间相关宏的使用示例
    // 这些宏会在编译时由CMake自动生成：
    // BUILD_DATE - 编译日期 (YYYY-MM-DD)
    // BUILD_TIME - 编译时间 (HH:MM:SS)
    // BUILD_TIMESTAMP - 完整时间戳 (YYYY-MM-DD HH:MM:SS UTC)
    // BUILD_YEAR, BUILD_MONTH, BUILD_DAY - 编译年月日数值

    // Git信息相关宏的使用示例
    // 这些宏会在编译时由CMake从Git仓库获取：
    // GIT_BRANCH - Git分支名
    // GIT_COMMIT_SHORT - Git提交ID短版本
    // GIT_COMMIT_FULL - Git提交ID完整版本
    // GIT_COMMIT_DATE - 最后提交日期
    // GIT_DIRTY - 是否有未提交更改 (true/false)

    /**
     * @brief 打印版本信息
     */
    void print_version_info(void);

    /**
     * @brief 获取编译时间字符串
     * @return 编译时间字符串指针
     */
    const char *get_build_timestamp(void);

    /**
     * @brief 获取Git分支名
     * @return Git分支名字符串指针
     */
    const char *get_git_branch(void);

    /**
     * @brief 获取Git提交ID
     * @return Git提交ID字符串指针
     */
    const char *get_git_commit_id(void);

    /**
     * @brief 检查是否有未提交的更改
     * @return true表示有未提交更改，false表示工作目录干净
     */
    bool is_git_dirty(void);

#ifdef __cplusplus
}
#endif

#endif // VERSION_INFO_H