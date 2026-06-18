/*
 * Bosch BMA400 3-axis accelerometer driver
 * SPDX-FileCopyrightText: Copyright 2026 bytesatwork AG
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT bosch_bma400

#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

#include "bma400.h"

#if DT_ANY_INST_ON_BUS_STATUS_OKAY(i2c)

LOG_MODULE_DECLARE(bma400, CONFIG_SENSOR_LOG_LEVEL);

static int bma400_i2c_read_reg(const struct device *dev, uint8_t reg_addr,
				uint8_t *value)
{
	const struct bma400_config *cfg = dev->config;

	return i2c_reg_read_byte_dt(&cfg->bus_cfg.i2c, reg_addr, value);
}

static int bma400_i2c_write_reg(const struct device *dev, uint8_t reg_addr,
				uint8_t value)
{
	const struct bma400_config *cfg = dev->config;

	return i2c_reg_write_byte_dt(&cfg->bus_cfg.i2c, reg_addr, value);
}

static const struct bma400_hw_operations i2c_ops = {
	.read_reg  = bma400_i2c_read_reg,
	.write_reg  = bma400_i2c_write_reg,
};

int bma400_i2c_init(const struct device *dev)
{
	struct bma400_data *data = dev->data;
	const struct bma400_config *cfg = dev->config;

	if (!device_is_ready(cfg->bus_cfg.spi.bus)) {
		LOG_ERR("SPI bus device is not ready");
		return -ENODEV;
	}

	data->hw_ops = &i2c_ops;

	return 0;
}
#endif /* DT_ANY_INST_ON_BUS_STATUS_OKAY(spi) */
