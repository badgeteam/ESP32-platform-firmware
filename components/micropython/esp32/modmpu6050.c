#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include "driver_mpu6050.h"

#ifdef CONFIG_DRIVER_MPU6050_ENABLE

static mp_obj_t mpu6050_configure_dlpf(mp_uint_t n_args, const mp_obj_t *args) {
	esp_err_t res = driver_mpu6050_configure_dlpf(mp_obj_get_int(args[0]));
	if (res != ESP_OK) {
		mp_raise_OSError(MP_EIO);
	}
	return mp_const_none;
}

static mp_obj_t mpu6050_configure_accel_range(mp_uint_t n_args, const mp_obj_t *args) {
	esp_err_t res = driver_mpu6050_configure_accel_range(mp_obj_get_int(args[0]));
	if (res != ESP_OK) {
		mp_raise_OSError(MP_EIO);
	}
	return mp_const_none;
}

static mp_obj_t mpu6050_configure_gyro_range(mp_uint_t n_args, const mp_obj_t *args) {
	esp_err_t res = driver_mpu6050_configure_gyro_range(mp_obj_get_int(args[0]));
	if (res != ESP_OK) {
		mp_raise_OSError(MP_EIO);
	}
	return mp_const_none;
}

static mp_obj_t mpu6050_interrupt_configure(mp_uint_t n_args, const mp_obj_t *args) {
	esp_err_t res = driver_mpu6050_interrupt_configure(mp_obj_get_int(args[0]), mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]), mp_obj_get_int(args[4]), mp_obj_get_int(args[5]), mp_obj_get_int(args[6]));
	if (res != ESP_OK) {
		mp_raise_OSError(MP_EIO);
	}
	return mp_const_none;
}

static mp_obj_t mpu6050_interrupt_enable(mp_uint_t n_args, const mp_obj_t *args) {
	esp_err_t res = driver_mpu6050_interrupt_enable(mp_obj_get_int(args[0]), mp_obj_get_int(args[1]), mp_obj_get_int(args[2]));
	if (res != ESP_OK) {
		mp_raise_OSError(MP_EIO);
	}
	return mp_const_none;
}

static mp_obj_t mpu6050_read_interrupt_status( void ) {
	bool fifo_oflow_int, i2c_mst_int, data_rdy_int;
	esp_err_t res = driver_mpu6050_read_interrupt_status(&fifo_oflow_int, &i2c_mst_int, &data_rdy_int);
	if (res != ESP_OK) {
		mp_raise_OSError(MP_EIO);
	}
	mp_obj_t tuple[3];
	tuple[0] = mp_obj_new_int(fifo_oflow_int);
	tuple[1] = mp_obj_new_int(i2c_mst_int);
	tuple[2] = mp_obj_new_int(data_rdy_int);
	return mp_obj_new_tuple(3, tuple);
}

static mp_obj_t mpu6050_read_accel( void ) {
	int16_t x, y, z;
	esp_err_t res = driver_mpu6050_read_accel(&x, &y, &z);
	if (res != ESP_OK) {
		mp_raise_OSError(MP_EIO);
	}
	mp_obj_t tuple[3];
	tuple[0] = mp_obj_new_int(x);
	tuple[1] = mp_obj_new_int(y);
	tuple[2] = mp_obj_new_int(z);
	return mp_obj_new_tuple(3, tuple);
}

static mp_obj_t mpu6050_read_gyro( void ) {
	int16_t x, y, z;
	esp_err_t res = driver_mpu6050_read_gyro(&x, &y, &z);
	if (res != ESP_OK) {
		mp_raise_OSError(MP_EIO);
	}
	mp_obj_t tuple[3];
	tuple[0] = mp_obj_new_int(x);
	tuple[1] = mp_obj_new_int(y);
	tuple[2] = mp_obj_new_int(z);
	return mp_obj_new_tuple(3, tuple);
}

static mp_obj_t mpu6050_read_temp( void ) {
	float t;
	esp_err_t res = driver_mpu6050_read_temp(&t);
	if (res != ESP_OK) {
		mp_raise_OSError(MP_EIO);
	}
	return mp_obj_new_float(t);
}

//Configuration functions
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mpu6050_configure_dlpf_obj,        1, 1, mpu6050_configure_dlpf);
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mpu6050_configure_accel_range_obj, 1, 1, mpu6050_configure_accel_range);
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mpu6050_configure_gyro_range_obj,  1, 1, mpu6050_configure_gyro_range);
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mpu6050_interrupt_configure_obj,   7, 7, mpu6050_interrupt_configure);
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mpu6050_interrupt_enable_obj,      3, 3, mpu6050_interrupt_enable);

//Read functions
static MP_DEFINE_CONST_FUN_OBJ_0( mpu6050_read_interrupt_status_obj, mpu6050_read_interrupt_status );
static MP_DEFINE_CONST_FUN_OBJ_0( mpu6050_read_accel_obj,            mpu6050_read_accel            );
static MP_DEFINE_CONST_FUN_OBJ_0( mpu6050_read_gyro_obj,             mpu6050_read_gyro             );
static MP_DEFINE_CONST_FUN_OBJ_0( mpu6050_read_temp_obj,             mpu6050_read_temp             );

static const mp_rom_map_elem_t mpu6050_module_globals_table[] = {
	{MP_ROM_QSTR( MP_QSTR_GYRO_RANGE_250        ), MP_ROM_INT( 0                                  )},
	{MP_ROM_QSTR( MP_QSTR_GYRO_RANGE_500        ), MP_ROM_INT( 1                                  )},
	{MP_ROM_QSTR( MP_QSTR_GYRO_RANGE_1000       ), MP_ROM_INT( 2                                  )},
	{MP_ROM_QSTR( MP_QSTR_GYRO_RANGE_2000       ), MP_ROM_INT( 3                                  )},
	
	{MP_ROM_QSTR( MP_QSTR_ACCEL_RANGE_2G        ), MP_ROM_INT( 0                                  )},
	{MP_ROM_QSTR( MP_QSTR_ACCEL_RANGE_4G        ), MP_ROM_INT( 1                                  )},
	{MP_ROM_QSTR( MP_QSTR_ACCEL_RANGE_8G        ), MP_ROM_INT( 2                                  )},
	{MP_ROM_QSTR( MP_QSTR_ACCEL_RANGE_16G       ), MP_ROM_INT( 3                                  )},
	
	{MP_ROM_QSTR( MP_QSTR_configure_dlpf        ), MP_ROM_PTR( &mpu6050_configure_dlpf_obj        )},
	{MP_ROM_QSTR( MP_QSTR_configure_accel_range ), MP_ROM_PTR( &mpu6050_configure_accel_range_obj )},
	{MP_ROM_QSTR( MP_QSTR_configure_gyro_range  ), MP_ROM_PTR( &mpu6050_configure_gyro_range_obj  )},
	{MP_ROM_QSTR( MP_QSTR_interrupt_configure   ), MP_ROM_PTR( &mpu6050_interrupt_configure_obj   )},
	{MP_ROM_QSTR( MP_QSTR_interrupt_enable      ), MP_ROM_PTR( &mpu6050_interrupt_enable_obj      )},
	{MP_ROM_QSTR( MP_QSTR_interrupt_status      ), MP_ROM_PTR( &mpu6050_read_interrupt_status_obj )},
	{MP_ROM_QSTR( MP_QSTR_acceleration          ), MP_ROM_PTR( &mpu6050_read_accel_obj            )},
	{MP_ROM_QSTR( MP_QSTR_gyroscope             ), MP_ROM_PTR( &mpu6050_read_gyro_obj             )},
	{MP_ROM_QSTR( MP_QSTR_temperature           ), MP_ROM_PTR( &mpu6050_read_temp_obj             )},
};

static MP_DEFINE_CONST_DICT(mpu6050_module_globals, mpu6050_module_globals_table);

const mp_obj_module_t mpu6050_module = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&mpu6050_module_globals,
};

#endif // CONFIG_DRIVER_MPU6050_ENABLE
