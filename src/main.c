/**
 * @file main.c
 * @author Gabriel Germano (gabriel.germano@edge.ufal.br)
 * @brief
 * @version 0.1
 * @date 29-01-2026
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "ldr_test.h"
#include "io_test.h"
#include "pwm_test.h"
#include "addr_led_test.h"
#include "ir_io_test.h"
#include "sd_test.h"
#include "display_test.h"

#include <stdlib.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_types.h>

static int cmd_test_all(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	int ret;

	ret = cmd_test_ldr(sh, argc, argv);
	if (ret < 0) {
		shell_fprintf(sh, SHELL_VT100_COLOR_RED, "LDR test failed");
		return ret;
	}

	ret = cmd_test_io(sh, argc, argv);
	if (ret < 0) {
		shell_fprintf(sh, SHELL_VT100_COLOR_RED, "IO test failed");
		return ret;
	}

	ret = cmd_test_pwm(sh, argc, argv);
	if (ret < 0) {
		shell_fprintf(sh, SHELL_VT100_COLOR_RED, "PWM test failed");
		return ret;
	}

	ret = cmd_test_addr_led(sh, argc, argv);
	if (ret < 0) {
		shell_fprintf(sh, SHELL_VT100_COLOR_RED, "Addressable LED test failed");
		return ret;
	}

	ret = cmd_test_ir_io(sh, argc, argv);
	if (ret < 0) {
		shell_fprintf(sh, SHELL_VT100_COLOR_RED, "IR IO test failed");
		return ret;
	}

	shell_fprintf(sh, SHELL_VT100_COLOR_GREEN, "All tests passed successfully.");
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	commands,
	SHELL_CMD_ARG(ldr, NULL, "Initialize the bringup test for LDR Module.", cmd_test_ldr, 1, 0),
	SHELL_CMD_ARG(io, NULL, "Initialize the bringup test for IO Module.", cmd_test_io, 1, 0),
	SHELL_CMD_ARG(pwm, NULL, "Initialize the bringup test for PWM Module.", cmd_test_pwm, 1, 0),
	SHELL_CMD_ARG(addr_led, NULL, "Initialize the bringup test for Addressable LED Module.",
		      cmd_test_addr_led, 1, 0),
	SHELL_CMD_ARG(display, NULL, "Initialize the bringup test for Display Module.",
		      cmd_test_display, 1, 0),
	SHELL_CMD_ARG(ir_io, NULL, "Initialize the bringup test for IR IO Module.", cmd_test_ir_io,
		      1, 0),
	SHELL_CMD_ARG(all, NULL, "Initialize the bringup test for all Modules.", cmd_test_all, 1,
		      0),
	SHELL_CMD_ARG(sd, NULL, "Initialize the bringup test for SD Card Module.", cmd_test_sd, 1,
		      0),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);

SHELL_CMD_REGISTER(test, &commands, "Test bringup of Zbook", NULL);

static void list_available_tests(void)
{
	printk("Available tests:\n");
	printk("  test ldr\n");
	printk("  test io\n");
	printk("  test pwm\n");
	printk("  test addr_led\n");
	printk("  test display\n");
	printk("  test ir_io\n");
	printk("  test sd\n");
	printk("  test all\n");
}

int main(void)
{
	const struct device *dev;
	uint32_t dtr = 0;

	dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));
	if (!device_is_ready(dev)) {
		return 0;
	}

	list_available_tests();

	while (!dtr) {
		uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
		k_sleep(K_MSEC(100));
	}
	return 0;
}