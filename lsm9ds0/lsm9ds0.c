/*
 * lsm9ds0_gyro.c
 *
 * Copyright (C) 2016 Matija Podravec <matija_podravec@fastmail.fm>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * 
 *
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/bitops.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/iio/kfifo_buf.h>

#define LSM9DS0_WHO_AM_I_REG            (0x0F)
#define LSM9DS0_CTRL_REG1_G_REG         (0x20)
#define LSM9DS0_CTRL_REG2_G_REG         (0x21)
#define LSM9DS0_CTRL_REG3_G_REG         (0x22)
#define LSM9DS0_CTRL_REG4_G_REG         (0x23)
#define LSM9DS0_CTRL_REG5_G_REG         (0x24)
#define LSM9DS0_REFERENCE_G_REG         (0x25)
#define LSM9DS0_STATUS_REG_G_REG        (0x27)
#define LSM9DS0_OUT_X_L_G_REG           (0x28)
#define LSM9DS0_OUT_X_H_G_REG           (0x29)
#define LSM9DS0_OUT_Y_L_G_REG           (0x2A)
#define LSM9DS0_OUT_Y_H_G_REG           (0x2B)
#define LSM9DS0_OUT_Z_L_G_REG           (0x2C)
#define LSM9DS0_OUT_Z_H_G_REG           (0x2D)
#define LSM9DS0_FIFO_CTRL_REG_G_REG     (0x2E)
#define LSM9DS0_FIFO_SRC_REG_G_REG      (0x2F)
#define LSM9DS0_INT1_CFG_G_REG          (0x30)
#define LSM9DS0_INT1_SRC_G_REG          (0x31) 
#define LSM9DS0_INT1_TSH_XH_G_REG       (0x32)
#define LSM9DS0_INT1_TSH_XL_G_REG       (0x33)
#define LSM9DS0_INT1_TSH_YH_G_REG       (0x34)
#define LSM9DS0_INT1_TSH_YL_G_REG       (0x35)
#define LSM9DS0_INT1_TSH_ZH_G_REG       (0x36)
#define LSM9DS0_INT1_TSH_ZL_G_REG       (0x37)
#define LSM9DS0_INT1_DURATION_G_REG     (0x38)
#define LSM9DS0_OUT_TEMP_L_XM_REG       (0x05)
#define LSM9DS0_OUT_TEMP_H_XM_REG       (0x06)
#define LSM9DS0_STATUS_REG_M_REG        (0x07)
#define LSM9DS0_OUT_X_L_M_REG           (0x08)
#define LSM9DS0_OUT_X_H_M_REG           (0x09)
#define LSM9DS0_OUT_Y_L_M_REG           (0x0A)
#define LSM9DS0_OUT_Y_H_M_REG           (0x0B)
#define LSM9DS0_OUT_Z_L_M_REG           (0x0C)
#define LSM9DS0_OUT_Z_H_M_REG           (0x0D)
#define LSM9DS0_INT_CTRL_REG_M_REG      (0x12)
#define LSM9DS0_INT_SRC_REG_M_REG       (0x13)
#define LSM9DS0_INT_THS_L_M_REG         (0x14)
#define LSM9DS0_INT_THS_H_M_REG         (0x15)
#define LSM9DS0_OFFSET_X_L_M_REG        (0x16)
#define LSM9DS0_OFFSET_X_H_M_REG        (0x17)
#define LSM9DS0_OFFSET_Y_L_M_REG        (0x18)
#define LSM9DS0_OFFSET_Y_H_M_REG        (0x19)
#define LSM9DS0_OFFSET_Z_L_M_REG        (0x1A)
#define LSM9DS0_OFFSET_Z_H_M_REG        (0x1B)
#define LSM9DS0_REFERENCE_X_REG         (0x1C)
#define LSM9DS0_REFERENCE_Y_REG         (0x1D)
#define LSM9DS0_REFERENCE_Z_REG         (0x1E)
#define LSM9DS0_CTRL_REG0_XM_REG        (0x1F)
#define LSM9DS0_CTRL_REG1_XM_REG        (0x20)
#define LSM9DS0_CTRL_REG2_XM_REG        (0x21)
#define LSM9DS0_CTRL_REG3_XM_REG        (0x22)
#define LSM9DS0_CTRL_REG4_XM_REG        (0x23)
#define LSM9DS0_CTRL_REG5_XM_REG        (0x24)
#define LSM9DS0_CTRL_REG6_XM_REG        (0x25)
#define LSM9DS0_CTRL_REG7_XM_REG        (0x26)
#define LSM9DS0_STATUS_REG_A_REG        (0x27)
#define LSM9DS0_OUT_X_L_A_REG           (0x28)
#define LSM9DS0_OUT_X_H_A_REG           (0x29)
#define LSM9DS0_OUT_Y_L_A_REG           (0x2A)
#define LSM9DS0_OUT_Y_H_A_REG           (0x2B)
#define LSM9DS0_OUT_Z_L_A_REG           (0x2C)
#define LSM9DS0_OUT_Z_H_A_REG           (0x2D)
#define LSM9DS0_FIFO_CTRL_REG_REG       (0x2E)
#define LSM9DS0_FIFO_SRC_REG_REG        (0x2F)
#define LSM9DS0_INT_GEN_1_REG_REG       (0x30)
#define LSM9DS0_INT_GEN_1_SRC_REG       (0x31)
#define LSM9DS0_INT_GEN_1_THS_REG       (0x32)
#define LSM9DS0_INT_GEN_1_DURATION_REG  (0x33)
#define LSM9DS0_INT_GEN_2_REG_REG       (0x34)
#define LSM9DS0_INT_GEN_2_SRC_REG       (0x35)
#define LSM9DS0_INT_GEN_2_THS_REG       (0x36)
#define LSM9DS0_INT_GEN_2_DURATION_REG  (0x37)
#define LSM9DS0_CLICK_CFG_REG           (0x38)
#define LSM9DS0_CLICK_SRC_REG           (0x39)
#define LSM9DS0_CLICK_THS_REG           (0x3A)
#define LSM9DS0_TIME_LIMIT_REG          (0x3B)
#define LSM9DS0_TIME_LATENCY_REG        (0x3C)
#define LSM9DS0_TIME_WINDOW_REG         (0x3D)
#define LSM9DS0_ACT_THS_REG             (0x3E)
#define LSM9DS0_ACT_DUR_REG             (0x3F)

#define LSM9DS0_GYRO_ODR_95HZ_VAL       (0x00 << 6)
#define LSM9DS0_GYRO_ODR_190HZ_VAL      (0x01 << 6)
#define LSM9DS0_GYRO_ODR_380HZ_VAL      (0x02 << 6)
#define LSM9DS0_GYRO_ODR_760HZ_VAL      (0x03 << 6)

#define LSM9DS0_ACCEL_POWER_DOWN        (0x00 << 4)
#define LSM9DS0_ACCEL_ODR_3_125HZ_VAL   (0x01 << 4)
#define LSM9DS0_ACCEL_ODR_6_25HZ_VAL    (0x02 << 4)
#define LSM9DS0_ACCEL_ODR_12_5HZ_VAL    (0x03 << 4)
#define LSM9DS0_ACCEL_ODR_25HZ_VAL      (0x04 << 4)
#define LSM9DS0_ACCEL_ODR_50HZ_VAL      (0x05 << 4)
#define LSM9DS0_ACCEL_ODR_100HZ_VAL     (0x06 << 4)
#define LSM9DS0_ACCEL_ODR_200HZ_VAL     (0x07 << 4)
#define LSM9DS0_ACCEL_ODR_400HZ_VAL     (0x08 << 4)
#define LSM9DS0_ACCEL_ODR_800HZ_VAL     (0x09 << 4)
#define LSM9DS0_ACCEL_ODR_1600HZ_VAL    (0x0A << 4)

#define LSM9DS0_ACCEL_AFS_2G_VAL        (0x00 << 3)
#define LSM9DS0_ACCEL_AFS_4G_VAL        (0x01 << 3)
#define LSM9DS0_ACCEL_AFS_6G_VAL        (0x02 << 3)
#define LSM9DS0_ACCEL_AFS_8G_VAL        (0x03 << 3)
#define LSM9DS0_ACCEL_AFS_16G_VAL       (0x04 << 3)

#define LSM9DS0_MAGN_ODR_3_125HZ_VAL    (0x00 << 2)
#define LSM9DS0_MAGN_ODR_6_25HZ_VAL     (0x01 << 2)
#define LSM9DS0_MAGN_ODR_12_5HZ_VAL     (0x02 << 2)
#define LSM9DS0_MAGN_ODR_25HZ_VAL       (0x03 << 2)
#define LSM9DS0_MAGN_ODR_50HZ_VAL       (0x04 << 2)
#define LSM9DS0_MAGN_ODR_100HZ_VAL      (0x05 << 2)

#define LSM9DS0_MAGN_MFS_2_GAUSS_VAL    (0x00 << 5)
#define LSM9DS0_MAGN_MFS_4_GAUSS_VAL    (0x01 << 5)
#define LSM9DS0_MAGN_MFS_8_GAUSS_VAL    (0x02 << 5)
#define LSM9DS0_MAGN_MFS_12_GAUSS_VAL   (0x03 << 5)

#define LSM9DS0_GYRO_FS_245DPS_VAL      (0x00 << 4)
#define LSM9DS0_GYRO_FS_500DPS_VAL      (0x01 << 4)
#define LSM9DS0_GYRO_FS_2000DPS_VAL     (0x10 << 4)

#define LSM9DS0_GYRO_X_EN               BIT(1)
#define LSM9DS0_GYRO_Y_EN               BIT(0) 
#define LSM9DS0_GYRO_Z_EN               BIT(2) 
#define LSM9DS0_GYRO_POWER_DOWN         (0x00 << 3)
#define LSM9DS0_GYRO_NORMAL_MODE        BIT(3)
#define LSM9DS0_ACCEL_X_EN              BIT(0) 
#define LSM9DS0_ACCEL_Y_EN              BIT(1) 
#define LSM9DS0_ACCEL_Z_EN              BIT(2) 
#define LSM9DS0_TEMP_EN                 BIT(7)
#define LSM9DS0_MAGN_LOW_RES_VAL        (0x00 << 5)
#define LSM9DS0_MAGN_HIGH_RES_VAL       (0x03 << 5)
#define LSM9DS0_MAGN_POWER_DOWN         (0x02)
#define LSM9DS0_MAGN_CONT_CONV_MODE     (0x00)
#define LSM9DS0_MAGN_SINGLE_CONV_MODE   (0x01)

#define LSM9DS0_GYRO_ID                  0xD4
#define LSM9DS0_ACCEL_MAGN_ID            0x49

enum { SCAN_INDEX_X, SCAN_INDEX_Y, SCAN_INDEX_Z };
enum { 
  SCAN_INDEX_ACCEL_X, SCAN_INDEX_ACCEL_Y, SCAN_INDEX_ACCEL_Z, 
  SCAN_INDEX_MAGN_X, SCAN_INDEX_MAGN_Y, SCAN_INDEX_MAGN_Z 
};
enum { GYRO, ACCEL_MAGN };

struct lsm9ds0_data {
  struct i2c_client *client;
  struct mutex lock;
  int sensor_type;
};

struct sensor_odr_avl {
  unsigned int hz;
  u8 value;
};

static const struct sensor_odr_avl lsm9ds0_odr_avl[4] = {
  {95, LSM9DS0_GYRO_ODR_95HZ_VAL},
  {190, LSM9DS0_GYRO_ODR_190HZ_VAL},
  {380, LSM9DS0_GYRO_ODR_380HZ_VAL},
  {760, LSM9DS0_GYRO_ODR_760HZ_VAL},
};

static ssize_t lsm9ds0_show_samp_freq_avail(struct device *dev,
        struct device_attribute *attr, char *buf)
{
  size_t len = 0;
  int n = ARRAY_SIZE(lsm9ds0_odr_avl);

  while (n-- > 0)
    len += scnprintf(buf + len, PAGE_SIZE - len,
      "%d ", lsm9ds0_odr_avl[n].hz);

  /* replace trailing space by newline */
  buf[len - 1] = '\n';

  return len;
}

static IIO_DEV_ATTR_SAMP_FREQ_AVAIL(lsm9ds0_show_samp_freq_avail);

static struct attribute *lsm9ds0_attributes[] = {
  &iio_dev_attr_sampling_frequency_available.dev_attr.attr,
  //&iio_dev_attr_in_accel_scale_available.dev_attr.attr,
  NULL
};

static const struct attribute_group lsm9ds0_group = {
  .attrs = lsm9ds0_attributes,
};


static const struct iio_buffer_setup_ops lsm9ds0_buffer_setup_ops = {
  .postenable = &iio_triggered_buffer_postenable,
  .predisable = &iio_triggered_buffer_predisable,
};

/*static irqreturn_t lsm9ds0_trigger_h(int irq, void *p)
{
  struct iio_poll_func *pf = p;
  struct iio_dev *indio_dev = pf->indio_dev;
  u32 *buf_data;

  buf_data = kmalloc(indio_dev->scan_bytes, GFP_KERNEL);
  if (!buf_data)
    goto done;

  if (!bitmap_empty(indio_dev->active_scan_mask, indio_dev->masklength)) {
    struct lsm9ds0_data *data = iio_priv(indio_dev);

    mutex_lock(&data->lock);

    mutex_unlock(&data->lock);
  }

  iio_push_to_buffers_with_timestamp(indio_dev, buf_data, iio_get_time_ns());
  kfree(buf_data);

done:
  iio_trigger_notify_done(indio_dev->trig);

  return IRQ_HANDLED;
}*/

static const struct iio_chan_spec lsm9ds0_gyro_channels[] = {
  {
    .type = IIO_ANGL_VEL,
    .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | BIT(IIO_CHAN_INFO_SCALE), 
    .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SAMP_FREQ), 
    .modified = 1,
    .channel2 = IIO_MOD_X,
    .scan_index = SCAN_INDEX_X,
    .scan_type = {
      .sign = 's',
      .realbits = 16,
      .storagebits = 16,
      .shift = 0,
      .endianness = IIO_LE,
    },
  }, {
    .type = IIO_ANGL_VEL,
    .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | BIT(IIO_CHAN_INFO_SCALE), 
    .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SAMP_FREQ), 
    .modified = 1,
    .channel2 = IIO_MOD_Y,
    .scan_index = SCAN_INDEX_Y,
    .scan_type = {
      .sign = 's',
      .realbits = 16,
      .storagebits = 16,
      .shift = 0,
      .endianness = IIO_LE,
    },
  }, {
    .type = IIO_ANGL_VEL,
    .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | BIT(IIO_CHAN_INFO_SCALE), 
    .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SAMP_FREQ), 
    .modified = 1,
    .channel2 = IIO_MOD_Z,
    .scan_index = SCAN_INDEX_Z,
    .scan_type = {
      .sign = 's',
      .realbits = 16,
      .storagebits = 16,
      .shift = 0,
      .endianness = IIO_LE,
    },
  },
  IIO_CHAN_SOFT_TIMESTAMP(3),
};

static const struct iio_chan_spec lsm9ds0_accel_magn_channels[] = {
  {
    .type = IIO_ACCEL,
    .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | BIT(IIO_CHAN_INFO_SCALE), 
    .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SAMP_FREQ), 
    .modified = 1,
    .channel2 = IIO_MOD_X,
    .scan_index = SCAN_INDEX_ACCEL_X,
    .scan_type = {
      .sign = 's',
      .realbits = 16,
      .storagebits = 16,
      .shift = 0,
      .endianness = IIO_LE,
    },
  }, {
    .type = IIO_ACCEL,
    .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | BIT(IIO_CHAN_INFO_SCALE), 
    .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SAMP_FREQ), 
    .modified = 1,
    .channel2 = IIO_MOD_Y,
    .scan_index = SCAN_INDEX_ACCEL_Y,
    .scan_type = {
      .sign = 's',
      .realbits = 16,
      .storagebits = 16,
      .shift = 0,
      .endianness = IIO_LE,
    },
  }, {
    .type = IIO_ACCEL,
    .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | BIT(IIO_CHAN_INFO_SCALE), 
    .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SAMP_FREQ), 
    .modified = 1,
    .channel2 = IIO_MOD_Z,
    .scan_index = SCAN_INDEX_ACCEL_Z,
    .scan_type = {
      .sign = 's',
      .realbits = 16,
      .storagebits = 16,
      .shift = 0,
      .endianness = IIO_LE,
    },
  }, {
    .type = IIO_MAGN,
    .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | BIT(IIO_CHAN_INFO_SCALE), 
    .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SAMP_FREQ), 
    .modified = 1,
    .channel2 = IIO_MOD_X,
    .scan_index = SCAN_INDEX_MAGN_X,
    .scan_type = {
      .sign = 's',
      .realbits = 16,
      .storagebits = 16,
      .shift = 0,
      .endianness = IIO_LE,
    },
  }, {
    .type = IIO_MAGN,
    .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | BIT(IIO_CHAN_INFO_SCALE), 
    .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SAMP_FREQ), 
    .modified = 1,
    .channel2 = IIO_MOD_Y,
    .scan_index = SCAN_INDEX_MAGN_Y,
    .scan_type = {
      .sign = 's',
      .realbits = 16,
      .storagebits = 16,
      .shift = 0,
      .endianness = IIO_LE,
    },
  }, {
    .type = IIO_MAGN,
    .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | BIT(IIO_CHAN_INFO_SCALE), 
    .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SAMP_FREQ), 
    .modified = 1,
    .channel2 = IIO_MOD_Z,
    .scan_index = SCAN_INDEX_MAGN_Z,
    .scan_type = {
      .sign = 's',
      .realbits = 16,
      .storagebits = 16,
      .shift = 0,
      .endianness = IIO_LE,
    },
  },
  IIO_CHAN_SOFT_TIMESTAMP(6),
};

static int lsm9ds0_gyro_read_measurements(struct i2c_client *client, s16 *x, s16 *y, s16 *z)
{
  int ret;
  u8 buf[6] = {0};

  buf[0] = 0x80 | LSM9DS0_OUT_X_L_G_REG;
  ret = i2c_master_send(client, buf, 1);
  if (ret < 0)
    return ret;

  ret = i2c_master_recv(client, buf, 6);
  if (ret < 0)
    return ret;

  *x = (buf[1] << 8) | buf[0];
  *y = (buf[3] << 8) | buf[2];
  *z = (buf[5] << 8) | buf[4];
  return ret;
}

static int lsm9ds0_read_raw(struct iio_dev *iio_dev,
      struct iio_chan_spec const *channel, 
      int *val, int *val2, long mask)
{
  struct lsm9ds0_data *data = iio_priv(iio_dev);
  int err = 0;
  s16 x = 0, y = 0, z = 0;

  switch (mask) {
  case IIO_CHAN_INFO_RAW:
    switch (channel->type) {
    case IIO_ANGL_VEL:
      err = lsm9ds0_gyro_read_measurements(data->client, &x, &y, &z);
      break;
    case IIO_ACCEL:
      x = y = z = 0;
      break;
    case IIO_MAGN:
      x = y = z = 0;
      break; 
    default:
      return -EINVAL;
      break;
    }
    if (err < 0)
      goto read_error;
    switch (channel->channel2) {
    case IIO_MOD_X:
      *val = x;
      break;
    case IIO_MOD_Y:
      *val = y;
      break;
    case IIO_MOD_Z:
      *val = z;
      break;
    }
    return IIO_VAL_INT;
  case IIO_CHAN_INFO_SCALE:
    return IIO_VAL_INT;
    break;
  case IIO_CHAN_INFO_SAMP_FREQ:
    return IIO_VAL_INT;
    break;
  default:
    return -EINVAL;
  }

read_error:
  return err;
}


static const struct iio_info lsm9ds0_info = {
	.attrs = &lsm9ds0_group,
  .read_raw = lsm9ds0_read_raw,
  .driver_module = THIS_MODULE,
};

static int lsm9ds0_gyro_init(struct i2c_client *client)
{
  int ret;

  ret = i2c_smbus_write_byte_data(client, LSM9DS0_CTRL_REG1_G_REG, 
      LSM9DS0_GYRO_NORMAL_MODE | LSM9DS0_GYRO_X_EN | 
      LSM9DS0_GYRO_Y_EN | LSM9DS0_GYRO_Z_EN);
  if (ret < 0) {
    dev_err(&client->dev, "Failed to write control register 5.\n");
    return ret;
  }
  ret = i2c_smbus_write_byte_data(client, LSM9DS0_CTRL_REG4_G_REG, 
      LSM9DS0_GYRO_FS_245DPS_VAL);
  if (ret < 0) {
    dev_err(&client->dev, "Failed to write control register 4.\n");
    return ret;
  }
  return 0;
}

static int lsm9ds0_accel_magn_init(struct i2c_client *client)
{
  int ret;

  ret = i2c_smbus_write_byte_data(client, LSM9DS0_CTRL_REG1_XM_REG, 
      LSM9DS0_ACCEL_ODR_100HZ_VAL | LSM9DS0_ACCEL_X_EN | 
      LSM9DS0_ACCEL_Y_EN | LSM9DS0_ACCEL_Z_EN);
  if (ret < 0) {
    dev_err(&client->dev, "Failed to write control register 1.\n");
    return ret;
  }
  ret = i2c_smbus_write_byte_data(client, LSM9DS0_CTRL_REG5_XM_REG, 
      LSM9DS0_TEMP_EN | LSM9DS0_MAGN_HIGH_RES_VAL | LSM9DS0_MAGN_ODR_50HZ_VAL);
  if (ret < 0) {
    dev_err(&client->dev, "Failed to write control register 5.\n");
    return ret;
  }
  ret = i2c_smbus_write_byte_data(client, LSM9DS0_CTRL_REG7_XM_REG, 
      LSM9DS0_MAGN_CONT_CONV_MODE);
  if (ret < 0) {
    dev_err(&client->dev, "Failed to write control register 7.\n");
    return ret;
  }
  ret = i2c_smbus_write_byte_data(client, LSM9DS0_CTRL_REG2_XM_REG, 
      LSM9DS0_ACCEL_AFS_2G_VAL);
  if (ret < 0) {
    dev_err(&client->dev, "Failed to write control register 2.\n");
    return ret;
  }
  ret = i2c_smbus_write_byte_data(client, LSM9DS0_CTRL_REG6_XM_REG, 
      LSM9DS0_MAGN_MFS_2_GAUSS_VAL);
  if (ret < 0) {
    dev_err(&client->dev, "Failed to write control register 6.\n");
    return ret;
  }
  return 0;
} 

static int lsm9ds0_probe(struct i2c_client *client,
    const struct i2c_device_id *id)
{
  struct iio_dev *indio_dev;
  struct lsm9ds0_data *data;
  //struct iio_buffer *buffer;
  int sensor_type;
  int ret;

  
  dev_info(&client->dev, "Entering LSM9DS0 probe...\n");
  if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_WORD_DATA)) {
    ret = -ENODEV;
    goto error_ret;
  }

  ret = i2c_smbus_read_byte_data(client, LSM9DS0_WHO_AM_I_REG);
  if (ret < 0) {
    ret = -EINVAL;
    goto error_ret;
  }
  dev_info(&client->dev, "Chip id found: 0x%x\n", ret);
  if (ret == LSM9DS0_GYRO_ID) {
    dev_info(&client->dev, "Gyroscope found.\n");
    sensor_type = GYRO;
  } else if (ret == LSM9DS0_ACCEL_MAGN_ID) {
    dev_info(&client->dev, "Accelerometer and magnetometer found.\n");
    sensor_type = ACCEL_MAGN;
  } else {
    dev_err(&client->dev, "No LSM9DS0 sensor found.\n");
    ret = -ENODEV;
    goto error_ret;
  }

  indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
  if (!indio_dev) {
    ret = -ENOMEM;
    goto error_ret;
  }
  
  data = iio_priv(indio_dev);
  mutex_init(&data->lock);
  i2c_set_clientdata(client, indio_dev);
  data->client = client;
  data->sensor_type = sensor_type;

  indio_dev->dev.parent = &client->dev;
  indio_dev->name = dev_name(&client->dev);
  indio_dev->info = &lsm9ds0_info;
  indio_dev->modes = INDIO_DIRECT_MODE;

  if (sensor_type == GYRO) {
    ret = lsm9ds0_gyro_init(client);
    indio_dev->channels = lsm9ds0_gyro_channels;
    indio_dev->num_channels = ARRAY_SIZE(lsm9ds0_gyro_channels);
  } else {
    ret = lsm9ds0_accel_magn_init(client);
    indio_dev->channels = lsm9ds0_accel_magn_channels;
    indio_dev->num_channels = ARRAY_SIZE(lsm9ds0_accel_magn_channels);
  }

  if (ret < 0)
    goto error_free_device;

  ret = iio_device_register(indio_dev);
  if (ret < 0)
    goto error_free_device;

  dev_info(&client->dev, "LSM9DS0 registered.\n");
  return 0;

error_free_device:
  iio_device_free(indio_dev);
error_ret:
  return ret;
}

static int lsm9ds0_remove(struct i2c_client *client)
{
  return 0;
}

static const struct i2c_device_id lsm9ds0_id[] = {
  { "lsm9ds0_gyro", 0 },
  { }
};
MODULE_DEVICE_TABLE(i2c, lsm9ds0_id);

static struct i2c_driver lsm9ds0_driver = {
  .driver = {
    .name = "lsm9ds0_gyro",
    .owner = THIS_MODULE,
  },
  .probe = lsm9ds0_probe,
  .remove = lsm9ds0_remove,
  .id_table = lsm9ds0_id,
};
module_i2c_driver(lsm9ds0_driver);

MODULE_AUTHOR("Matija Podravec <matija_podravec@fastmail.fm>");
MODULE_DESCRIPTION("LSM9DS0 gyroscope sensor");
MODULE_LICENSE("GPL");
