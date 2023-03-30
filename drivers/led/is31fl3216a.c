/*
 * Copyright (c) 2023 Endor AG
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT lumissil_is31fl3216a

#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/led.h>
#include <zephyr/logging/log.h>

#define IS31FL3216A_REG_CONFIG		0x00
#define IS31FL3216A_REG_CTL_1		0x01
#define IS31FL3216A_REG_CTL_2		0x02
#define IS31FL3216A_REG_PWM_FIRST	0x10
#define IS31FL3216A_REG_PWM_LAST	0x1F
#define IS31FL3216A_REG_UPDATE		0xB0

#define AMOUNT_OF_LEDS			16

LOG_MODULE_REGISTER(is31fl3216a, CONFIG_LED_LOG_LEVEL);

struct is31fl3216a_cfg {
	struct i2c_dt_spec i2c;
};

static int is31fl3216a_write_reg(const struct i2c_dt_spec *i2c, uint8_t reg,
				 uint8_t val)
{
	uint8_t buffer[2] = {reg, val};
	int status;

	status = i2c_write_dt(i2c, buffer, sizeof(buffer));
	if (status) {
		LOG_ERR("Could not write register 0x%02x: %i", reg, status);
	}

	return status;
}

static int is31fl3216a_led_blink(const struct device *dev, uint32_t led,
				 uint32_t delay_on, uint32_t delay_off)
{
	return -ENOTSUP;
}

static int is31fl3216a_led_set_brightness(const struct device *dev,
					  uint32_t led, uint8_t value)
{
	const struct is31fl3216a_cfg *config = dev->config;
	uint8_t pwm_reg = IS31FL3216A_REG_PWM_LAST - led;
	int status;
	uint8_t pwm_value;

	if (led > AMOUNT_OF_LEDS - 1) {
		return -ENODEV;
	}

	if (value > 100) {
		return -EINVAL;
	}

	pwm_value = (0xFFU * value) / 100;
	status = is31fl3216a_write_reg(&config->i2c, pwm_reg, pwm_value);
	if (status) {
		return status;
	}

	return is31fl3216a_write_reg(&config->i2c, IS31FL3216A_REG_UPDATE, 0);
}

static int is31fl3216a_led_on(const struct device *dev, uint32_t led)
{
	return is31fl3216a_led_set_brightness(dev, led, 100);
}

static int is31fl3216a_led_off(const struct device *dev, uint32_t led)
{
	return is31fl3216a_led_set_brightness(dev, led, 0);
}

static int is31fl3216a_init(const struct device *dev)
{
	const struct is31fl3216a_cfg *config = dev->config;
	int status;

	LOG_INF("Initializing @0x%x...", config->i2c.addr);

	status = is31fl3216a_write_reg(&config->i2c, IS31FL3216A_REG_CTL_1,
				       0xFF);
	if (status) {
		return status;
	}

	status = is31fl3216a_write_reg(&config->i2c, IS31FL3216A_REG_CTL_2,
				       0xFF);
	if (status) {
		return status;
	}

	for (int i = IS31FL3216A_REG_PWM_FIRST;
	     i <= IS31FL3216A_REG_PWM_LAST;
	     i++) {
		status = is31fl3216a_write_reg(&config->i2c, i, 0);
		if (status) {
			return status;
		}
	}
	status = is31fl3216a_write_reg(&config->i2c, IS31FL3216A_REG_UPDATE, 0);
	if (status) {
		return status;
	}

	status = is31fl3216a_write_reg(&config->i2c, IS31FL3216A_REG_CONFIG, 0);
	if (status) {
		return status;
	}

	return status;
}

static const struct led_driver_api is31fl3216a_led_api = {
	.blink = is31fl3216a_led_blink,
	.set_brightness = is31fl3216a_led_set_brightness,
	.on = is31fl3216a_led_on,
	.off = is31fl3216a_led_off,
};

#define IS31FL3216A_INIT(id) \
	static const struct is31fl3216a_cfg is31fl3216a_##id##_cfg = {	\
		.i2c = I2C_DT_SPEC_INST_GET(id),			\
	};								\
	DEVICE_DT_INST_DEFINE(						\
		id, &is31fl3216a_init, NULL, NULL,			\
		&is31fl3216a_##id##_cfg, POST_KERNEL,			\
		CONFIG_LED_INIT_PRIORITY, &is31fl3216a_led_api);

DT_INST_FOREACH_STATUS_OKAY(IS31FL3216A_INIT)
