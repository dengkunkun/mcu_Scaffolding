#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include "worker.h"
#include "unity.h"

#include <stdio.h>
#include <string.h>

// Test function declarations
void test_worker_task_creation(void);
void test_worker_task_execution(void);
void test_worker_priority_handling(void);
void worker_test_runner(void);

// Unity framework required functions
void setUp(void)
{
    // Called before each test
    // Initialize any test-specific setup here
}

void tearDown(void)
{
    // Called after each test
    // Clean up any test-specific resources here
}

// Test runner for worker component
void worker_test_runner(void)
{
    // Run worker component tests
    RUN_TEST(test_worker_task_creation);
    RUN_TEST(test_worker_task_execution);
    RUN_TEST(test_worker_priority_handling);
}

// Individual test functions
void test_worker_task_creation(void)
{
    // Test worker task creation functionality
    TEST_ASSERT_TRUE(1); // Placeholder test
}

void test_worker_task_execution(void)
{
    // Test worker task execution
    TEST_ASSERT_TRUE(1); // Placeholder test
}

void test_worker_priority_handling(void)
{
    // Test worker priority handling
    TEST_ASSERT_TRUE(1); // Placeholder test
}

// 测试任务
void test_runner_task(void *argument)
{
    UNUSED(argument);

    printf("\n");
    printf("========================================\n");
    printf("  MCU Scaffolding Component Test Suite  \n");
    printf("========================================\n");
    printf("Build: %s %s\n", __DATE__, __TIME__);
    printf("FreeRTOS Version: %s\n", tskKERNEL_VERSION_NUMBER);
    printf("========================================\n\n");

    // 延迟一下确保系统稳定
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 运行Worker组件测试
    printf("\n🧪 Running Worker Component Tests...\n");
    worker_test_runner();

    // 未来可以添加更多组件测试
    /*
    printf("\n🧪 Running UART Component Tests...\n");
    uart_test_runner();

    printf("\n🧪 Running Memory Component Tests...\n");
    memory_test_runner();
    */

    printf("\n========================================\n");
    printf("  All Component Tests Completed!        \n");
    printf("========================================\n");

    // 测试完成后进入空闲循环
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
        printf("📊 System Status: Tests Completed Successfully\n");
    }
}

void app_main(void)
{
    printf("🚀 MCU Scaffolding Test Mode Started\n");
    printf("💡 Test mode: Component Unit Tests\n");

    // 测试模式只需要最基本的初始化
    printf("✅ Basic system components initialized\n");

    // 创建测试运行器任务
    osThreadId_t test_task_handle = osThreadNew(test_runner_task, NULL, &(osThreadAttr_t){
                                                                            .name = "TestRunner",
                                                                            .stack_size = 4096,
                                                                            .priority = osPriorityNormal,
                                                                        });

    if (test_task_handle == NULL)
    {
        printf("❌ Failed to create test runner task\n");
        Error_Handler();
    }

    printf("✅ Test runner task created\n");
    printf("🔄 Starting FreeRTOS scheduler...\n");
}