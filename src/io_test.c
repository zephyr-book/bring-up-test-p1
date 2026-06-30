/**
 * @file io_test.c
 * @author Gabriel Germano (gabriel.germano@edge.ufal.br)
 * @brief
 * @version 0.1
 * @date 29-01-2026
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "io_test.h"

#include "zephyr/devicetree.h"
#include <stdio.h>
#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/ring_buffer.h>
#include <zephyr/shell/shell.h>

#define SLEEP_TIME_MS 10

static const struct gpio_dt_spec led_red = GPIO_DT_SPEC_GET(DT_NODELABEL(red_led), gpios);
static const struct gpio_dt_spec led_green = GPIO_DT_SPEC_GET(DT_NODELABEL(green_led), gpios);
static const struct gpio_dt_spec led_blue = GPIO_DT_SPEC_GET(DT_NODELABEL(blue_led), gpios);
static const struct gpio_dt_spec led_yellow = GPIO_DT_SPEC_GET(DT_NODELABEL(yellow_led), gpios);

static const struct gpio_dt_spec button_left = GPIO_DT_SPEC_GET(DT_NODELABEL(button3), gpios);
static const struct gpio_dt_spec button_right = GPIO_DT_SPEC_GET(DT_NODELABEL(button2), gpios);
static const struct gpio_dt_spec button_up = GPIO_DT_SPEC_GET(DT_NODELABEL(button1), gpios);
static const struct gpio_dt_spec button_down = GPIO_DT_SPEC_GET(DT_NODELABEL(button4), gpios);

static const struct gpio_dt_spec leds[] = {led_red, led_green, led_blue, led_yellow};

static const struct gpio_dt_spec buttons[] = {button_left, button_right, button_up, button_down};

static const char *button_names[] = {"LEFT", "RIGHT", "UP", "DOWN"};
static const char *led_names[] = {"RED", "GREEN", "BLUE", "YELLOW"};

static int init_leds(const struct shell *sh)
{
	int ret;

	for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
		if (!gpio_is_ready_dt(&leds[i])) {
			shell_error(sh, "LED %s device not ready", led_names[i]);
			return -ENODEV;
		}

		ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
		if (ret < 0) {
			shell_error(sh, "Failed to configure LED %s: %d", led_names[i], ret);
			return ret;
		}
	}

	shell_fprintf(sh, SHELL_VT100_COLOR_GREEN, "LEDs initialized\n");
	return 0;
}

static int init_buttons(const struct shell *sh)
{
	int ret;

	for (size_t i = 0; i < ARRAY_SIZE(buttons); i++) {
		if (!gpio_is_ready_dt(&buttons[i])) {
			shell_error(sh, "Button %s device not ready", button_names[i]);
			return -ENODEV;
		}

		ret = gpio_pin_configure_dt(&buttons[i], GPIO_INPUT);
		if (ret < 0) {
			shell_error(sh, "Failed to configure button %s: %d", button_names[i], ret);
			return ret;
		}
	}

	shell_fprintf(sh, SHELL_VT100_COLOR_GREEN, "Buttons initialized\n");
	return 0;
}

int cmd_test_io(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	int ret;
	bool previous_state[ARRAY_SIZE(buttons)] = {0};

	ret = init_leds(sh);
	if (ret < 0) {
		shell_error(sh, "LED initialization failed: %d", ret);
		return ret;
	}

	ret = init_buttons(sh);
	if (ret < 0) {
		shell_error(sh, "Button initialization failed: %d", ret);
		return ret;
	}

	shell_fprintf(sh, SHELL_VT100_COLOR_GREEN, "IO Test started, press any key to stop\n");
	shell_fprintf(sh, SHELL_NORMAL, "Press buttons to toggle corresponding LEDs\n");

	while (1) {
		for (size_t i = 0; i < ARRAY_SIZE(buttons); i++) {
			int current = gpio_pin_get_dt(&buttons[i]);

			/* Detect falling edge (button press if ACTIVE_LOW) */
			if (current == 1 && previous_state[i] == 0) {
				gpio_pin_toggle_dt(&leds[i]);
				shell_fprintf(sh, SHELL_NORMAL,
					      "Button %s pressed -> LED %s toggled\n",
					      button_names[i], led_names[i]);
			}

			previous_state[i] = current;
		}

		k_msleep(SLEEP_TIME_MS);

		/* Stop the test when the button is pressed for 2 seconds btn1 (UP) */
		if (previous_state[2] == 1 && gpio_pin_get_dt(&buttons[2]) == 1) {
			k_sleep(K_SECONDS(2));
			if (gpio_pin_get_dt(&buttons[2]) == 1) {
				shell_fprintf(sh, SHELL_VT100_COLOR_GREEN, "\nIO Test stopped\n");
				break;
			}
		}

	}

	for (size_t i = 0; i < ARRAY_SIZE(leds); i++) {
		gpio_pin_set_dt(&leds[i], 0);
	}

	shell_fprintf(sh, SHELL_VT100_COLOR_GREEN, "\nIO Test completed\n");

	return 0;
}