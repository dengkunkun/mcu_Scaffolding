#include "log.h"
int log_init(void)
{
    // 初始化日志系统
    elog_init();
    // 设置所有级别的格式，包含文件名和行号
    for (int level = ELOG_LVL_ASSERT; level <= ELOG_LVL_VERBOSE; level++)
    {
        elog_set_fmt(level, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME |
                                ELOG_FMT_DIR | ELOG_FMT_LINE);
    }
    // 启动日志输出
    elog_start();

    return 0; // 成功
}
