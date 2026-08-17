/*******************************************************************************
 *   @file   util/no_os_alloc.c
 *   @brief  Implementation of no-OS memory allocation functions.
 *   @author GMois (george.mois@analog.com)
********************************************************************************
 * Copyright 2023(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include <inttypes.h>
#include <string.h>
#include "no_os.h"


using namespace NO_OS;

/**
 * @brief Allocate memory and return a pointer to it.
 * @param size - Size of the memory block, in bytes.
 * @return Pointer to the allocated memory, or NULL if the request fails.
 */
/*__attribute__((weak)*/ void* no_os_alloc::no_os_malloc(size_t size)
{
	return malloc(size);
}

/**
 * @brief Allocate memory and return a pointer to it, set memory to 0.
 * @param nitems - Number of elements to be allocated.
 * @param size - Size of elements.
 * @return Pointer to the allocated memory, or NULL if the request fails.
 */
/*__attribute__((weak)*/ void* no_os_alloc::no_os_calloc(size_t nitems, size_t size)
{
	return calloc(nitems, size);
}

/**
 * @brief Deallocate memory previously allocated by a call to no_os_calloc
 * 		  or no_os_malloc.
 * @param ptr - Pointer to a memory block previously allocated by a call
 * 		  to no_os_calloc or no_os_malloc.
 * @return None.
 */
/*__attribute__((weak)*/  void no_os_alloc::no_os_free(void* ptr)
{
	free(ptr);
}

int32_t no_os_gpio::no_os_gpio_get(struct no_os_gpio_desc** desc,
	const struct no_os_gpio_init_param* param)
{
	int32_t ret;

	if (!param || !param->platform_ops)
		return -EINVAL;

	if (!param->platform_ops->gpio_ops_get)
		return -ENOSYS;

	ret = param->platform_ops->gpio_ops_get(desc, param);
	if (ret)
		return ret;

	(*desc)->platform_ops = param->platform_ops;

	return 0;
}

/**
 * @brief Get the value of an optional GPIO.
 * @param desc - The GPIO descriptor.
 * @param param - GPIO Initialization parameters.
 * @return 0 in case of success, -1 otherwise.
 */
int32_t no_os_gpio::no_os_gpio_get_optional(struct no_os_gpio_desc** desc,
	const struct no_os_gpio_init_param* param)
{
	int32_t ret;

	if (!param || (param->number == -1)) {
		*desc = NULL;
		return 0;
	}

	if (!param->platform_ops)
		return -EINVAL;

	if (!param->platform_ops->gpio_ops_get_optional)
		return -ENOSYS;

	ret = param->platform_ops->gpio_ops_get_optional(desc, param);
	if (ret)
		return ret;

	(*desc)->platform_ops = param->platform_ops;

	return 0;
}
/**
 * @brief Free the resources allocated by no_os_gpio_get().
 * @param desc - The GPIO descriptor.
 * @return 0 in case of success, -1 otherwise.
 */
int32_t no_os_gpio::no_os_gpio_remove(struct no_os_gpio_desc* desc)
{
	if (desc) {
		if (!desc->platform_ops)
			return -EINVAL;

		if (!desc->platform_ops->gpio_ops_remove)
			return -ENOSYS;

		return desc->platform_ops->gpio_ops_remove(desc);
	}

	return 0;
}

/**
 * @brief Enable the input direction of the specified GPIO.
 * @param desc - The GPIO descriptor.
 * @return 0 in case of success, -1 otherwise.
 */
int32_t no_os_gpio::no_os_gpio_direction_input(struct no_os_gpio_desc* desc)
{
	if (desc) {
		if (!desc->platform_ops)
			return -EINVAL;

		if (!desc->platform_ops->gpio_ops_direction_input)
			return -ENOSYS;

		return desc->platform_ops->gpio_ops_direction_input(desc);
	}

	return 0;
}

/**
 * @brief Enable the output direction of the specified GPIO.
 * @param desc - The GPIO descriptor.
 * @param value - The value.
 *                Example: NO_OS_GPIO_HIGH
 *                         NO_OS_GPIO_LOW
 * @return 0 in case of success, -1 otherwise.
 */
int32_t no_os_gpio::no_os_gpio_direction_output(struct no_os_gpio_desc* desc,
	uint8_t value)
{
	if (desc) {
		if (!desc->platform_ops)
			return -EINVAL;

		if (!desc->platform_ops->gpio_ops_direction_output)
			return -ENOSYS;

		return desc->platform_ops->
			gpio_ops_direction_output(desc, value);
	}

	return 0;
}

/**
 * @brief Get the direction of the specified GPIO.
 * @param desc - The GPIO descriptor.
 * @param direction - The direction.
 *                    Example: NO_OS_GPIO_OUT
 *                             NO_OS_GPIO_IN
 * @return 0 in case of success, -1 otherwise.
 */
int32_t no_os_gpio::no_os_gpio_get_direction(struct no_os_gpio_desc* desc,
	uint8_t* direction)
{
	if (desc) {
		if (!desc->platform_ops)
			return -EINVAL;

		if (!desc->platform_ops->gpio_ops_get_direction)
			return -ENOSYS;

		return desc->platform_ops->
			gpio_ops_get_direction(desc, direction);
	}

	return 0;
}

/**
 * @brief Set the value of the specified GPIO.
 * @param desc - The GPIO descriptor.
 * @param value - The value.
 *                Example: NO_OS_GPIO_HIGH
 *                         NO_OS_GPIO_LOW
 * @return 0 in case of success, -1 otherwise.
 */
int32_t no_os_gpio::no_os_gpio_set_value(struct no_os_gpio_desc* desc,
	uint8_t value)
{
	if (desc) {
		if (!desc->platform_ops)
			return -EINVAL;

		if (!desc->platform_ops->gpio_ops_set_value)
			return -ENOSYS;

		return desc->platform_ops->gpio_ops_set_value(desc, value);
	}

	return 0;
}

/**
 * @brief Get the value of the specified GPIO.
 * @param desc - The GPIO descriptor.
 * @param value - The value.
 *                Example: NO_OS_GPIO_HIGH
 *                         NO_OS_GPIO_LOW
 * @return 0 in case of success, -1 otherwise.
 */
int32_t no_os_gpio::no_os_gpio_get_value(struct no_os_gpio_desc* desc,
	uint8_t* value)
{
	if (desc) {
		if (!desc->platform_ops)
			return -EINVAL;

		if (!desc->platform_ops->gpio_ops_set_value)
			return -ENOSYS;

		return desc->platform_ops->gpio_ops_get_value(desc, value);
	}

	return 0;
}


/*__attribute__((weak)*/  void no_os_mutex::no_os_mutex_init(void** mutex) {}

/**
 * @brief Lock mutex.
 * @param ptr - Pointer toward the mutex.
 * @return None.
 */
/*__attribute__((weak)*/  void no_os_mutex::no_os_mutex_lock(void* mutex) {}

/**
 * @brief Unlock mutex.
 * @param ptr - Pointer toward the mutex.
 * @return None.
 */
/*__attribute((weak))*/  void no_os_mutex::no_os_mutex_unlock(void* mutex) {}

/**
 * @brief Remove mutex.
 * @param ptr - Pointer toward the mutex.
 * @return None.
 */
/*__attribute__((weak)*/  void no_os_mutex::no_os_mutex_remove(void* mutex) {}


static void* spi_table[SPI_MAX_BUS_NUMBER + 1];

/**
 * @brief Initialize the SPI communication peripheral.
 * @param desc - The SPI descriptor.
 * @param param - The structure that contains the SPI parameters.
 * @return 0 in case of success, -1 otherwise.
 */
int32_t no_os_spi::no_os_spi_init(struct no_os_spi_desc** desc,
	const struct no_os_spi_init_param* param)
{
	int32_t ret;

	if (!param || !param->platform_ops)
		return -EINVAL;

	if (!param->platform_ops->init)
		return -ENOSYS;
	if (param->device_id > SPI_MAX_BUS_NUMBER)
		return -EINVAL;
	// Initializing BUS descriptor
	if (!spi_table[param->device_id]) {
		ret = no_os_spi::no_os_spibus_init(param);
		if (ret)
			return ret;
	}
	// Initilize SPI descriptor
	ret = param->platform_ops->init(desc, param);
	if (ret)
		return ret;
	(*desc)->bus = (no_os_spibus_desc*)spi_table[param->device_id];
	(*desc)->bus->slave_number++;
	(*desc)->platform_ops = param->platform_ops;
	(*desc)->parent = param->parent;
	(*desc)->platform_delays = param->platform_delays;

	return 0;
}

/**
 * @brief Initialize the SPI bus communication peripheral.
 * @param param - The structure that containes the SPI bus parameters
 * @return 0 in case of success, error code otherwise
*/
int32_t no_os_spi::no_os_spibus_init(const struct no_os_spi_init_param* param)
{
	struct no_os_spibus_desc* bus = (struct no_os_spibus_desc*)no_os_alloc::no_os_calloc(1,
		sizeof(struct no_os_spibus_desc));

	if (!bus)
		return -ENOMEM;

	no_os_mutex::no_os_mutex_init(&(bus->mutex));

	bus->slave_number = 0;
	bus->device_id = param->device_id;
	bus->max_speed_hz = param->max_speed_hz;
	bus->mode = param->mode;
	bus->bit_order = param->bit_order;
	bus->platform_ops = param->platform_ops;
	bus->extra = param->extra;

	spi_table[param->device_id] = bus;

	return 0;
}

/**
 * @brief Free the resources allocated by no_os_spi_init().
 * @param desc - The SPI descriptor.
 * @return 0 in case of success, -1 otherwise.
 */
int32_t no_os_spi::no_os_spi_remove(struct no_os_spi_desc* desc)
{
	if (!desc || !desc->platform_ops)
		return -EINVAL;

	if (desc->bus)
		no_os_spi::no_os_spibus_remove(desc->bus->device_id);

	if (!desc->platform_ops->remove)
		return -ENOSYS;
	return desc->platform_ops->remove(desc);
}

/**
 * @brief Removes SPI bus instance
 * @param bus_number - SPI bus number
*/
void no_os_spi::no_os_spibus_remove(uint32_t bus_number)
{
	struct no_os_spibus_desc* bus = (struct no_os_spibus_desc*)
		spi_table[bus_number];

	if (bus->slave_number > 0)
		bus->slave_number--;

	if (bus->slave_number == 0) {
		no_os_mutex::no_os_mutex_remove(bus->mutex);

		if (bus) {
			no_os_alloc::no_os_free(bus);
			bus = NULL;
			spi_table[bus_number] = NULL;
		}
	}
}

/**
 * @brief Write and read data to/from SPI.
 * @param desc - The SPI descriptor.
 * @param data - The buffer with the transmitted/received data.
 * @param bytes_number - Number of bytes to write/read.
 * @return 0 in case of success, -1 otherwise.
 */
int32_t no_os_spi::no_os_spi_write_and_read(struct no_os_spi_desc* desc,
	uint8_t* data,
	uint16_t bytes_number)
{
	int32_t ret;

	if (!desc || !desc->platform_ops)
		return -EINVAL;

	if (!desc->platform_ops->write_and_read)
		return -ENOSYS;

	no_os_mutex::no_os_mutex_lock(desc->bus->mutex);
	ret = desc->platform_ops->write_and_read(desc, data, bytes_number);
	no_os_mutex::no_os_mutex_unlock(desc->bus->mutex);

	return ret;
}

/**
 * @brief  Iterate over head list and send all spi messages
 * @param desc - The SPI descriptor.
 * @param msgs - Array of messages.
 * @param len - Number of messages in the array.
 * @return 0 in case of success, negativ error code otherwise.
 */
int32_t no_os_spi::no_os_spi_transfer(struct no_os_spi_desc* desc,
	struct no_os_spi_msg* msgs,
	uint32_t len)
{
	int32_t  ret = 0;
	uint32_t i;

	if (!desc || !desc->platform_ops)
		return -EINVAL;

	if (desc->platform_ops->transfer)
		return desc->platform_ops->transfer(desc, msgs, len);

	no_os_mutex::no_os_mutex_lock(desc->bus->mutex);

	for (i = 0; i < len; i++) {
		if (msgs[i].rx_buff != msgs[i].tx_buff || !msgs[i].tx_buff) {
			ret = -EINVAL;
			goto out;
		}
		ret = no_os_spi_write_and_read(desc, msgs[i].rx_buff,
			msgs[i].bytes_number);
		if (NO_OS_IS_ERR_VALUE(ret)) {
			goto out;
		}
	}

out:
	no_os_mutex::no_os_mutex_unlock(desc->bus->mutex);
	return ret;
}

/**
 * @brief Transfer a list of messages using DMA and busy wait for the completion
 * @param desc - The SPI descriptor.
 * @param msgs - Array of messages.
 * @param len - Number of messages in the array.
 * @return 0 in case of success, negativ error code otherwise.
 */
int32_t no_os_spi::no_os_spi_transfer_dma_sync(struct no_os_spi_desc* desc,
	struct no_os_spi_msg* msgs,
	uint32_t len)
{
	if (!desc || !desc->platform_ops || !msgs || !len)
		return -EINVAL;

	if (desc->platform_ops->dma_transfer_sync)
		return desc->platform_ops->dma_transfer_sync(desc, msgs, len);

	return -ENOSYS;
}

/**
 * @brief Transfer a list of messages using DMA. The function will return after the
 * 	  first transfer is started. Once all the transfers are complete, a callback
 * 	  will be called.
 * @param desc - The SPI descriptor.
 * @param msgs - Array of messages.
 * @param len - Number of messages in the array.
 * @param callback - A function which will be called after all the transfers are done.
 * @param ctx - User specific data which should be passed to the callback function.
 * @return 0 in case of success, negativ error code otherwise.
 */
int32_t no_os_spi::no_os_spi_transfer_dma_async(struct no_os_spi_desc* desc,
	struct no_os_spi_msg* msgs,
	uint32_t len,
	void (*callback)(void*),
	void* ctx)
{
	if (!desc || !desc->platform_ops || !msgs || !len)
		return -EINVAL;

	if (desc->platform_ops->dma_transfer_async)
		return desc->platform_ops->dma_transfer_async(desc, msgs, len,
			callback, ctx);

	return -ENOSYS;
}

extern int no_os_test_bit(int pos, const volatile void* addr);

/**
 * Find first set bit in word.
 */
uint32_t no_os_util::no_os_find_first_set_bit(uint32_t word)
{
	uint32_t first_set_bit = 0;

	while (word) {
		if (word & 0x1)
			return first_set_bit;
		word >>= 1;
		first_set_bit++;
	}

	return 32;
}

/**
 * Find last set bit in word.
 */
uint32_t no_os_util::no_os_find_last_set_bit(uint32_t word)
{
	uint32_t bit = 0;
	uint32_t last_set_bit = 32;

	while (word) {
		if (word & 0x1)
			last_set_bit = bit;
		word >>= 1;
		bit++;
	}

	return last_set_bit;
}

/**
 * Locate the closest element in an array.
 */
uint32_t no_os_util::no_os_find_closest(int32_t val,
	const int32_t* array,
	uint32_t size)
{
	int32_t diff = abs(array[0] - val);
	uint32_t ret = 0;
	uint32_t i;

	for (i = 1; i < size; i++) {
		if (abs(array[i] - val) < diff) {
			diff = abs(array[i] - val);
			ret = i;
		}
	}

	return ret;
}

/**
 * Shift the value and apply the specified mask.
 */
uint32_t no_os_util::no_os_field_prep(uint32_t mask, uint32_t val)
{
	return (val << no_os_find_first_set_bit(mask)) & mask;
}

/**
 * Get a field specified by a mask from a word.
 */
uint32_t no_os_util::no_os_field_get(uint32_t mask, uint32_t word)
{
	return (word & mask) >> no_os_find_first_set_bit(mask);
}

/**
 * Log base 2 of the given number.
 */
int32_t no_os_util::no_os_log_base_2(uint32_t x)
{
	return no_os_find_last_set_bit(x);
}

/**
 * Find greatest common divisor of the given two numbers.
 */
uint32_t no_os_util::no_os_greatest_common_divisor(uint32_t a,
	uint32_t b)
{
	uint32_t div;

	if ((a == 0) || (b == 0))
		return no_os_max(a, b);

	while (b != 0) {
		div = a % b;
		a = b;
		b = div;
	}

	return a;
}
/**
 * Find lowest common multiple of the given two numbers.
 */
uint32_t no_os_util::no_os_lowest_common_multiple(uint32_t a, uint32_t b)
{
	if (a && b)
		return (a / no_os_greatest_common_divisor(a, b)) * b;
	else
		return 0;
}

/**
 * Calculate best rational approximation for a given fraction.
 */
void no_os_util::no_os_rational_best_approximation(uint32_t given_numerator,
	uint32_t given_denominator,
	uint32_t max_numerator,
	uint32_t max_denominator,
	uint32_t* best_numerator,
	uint32_t* best_denominator)
{
	uint32_t gcd;

	gcd = no_os_greatest_common_divisor(given_numerator, given_denominator);

	*best_numerator = given_numerator / gcd;
	*best_denominator = given_denominator / gcd;

	if ((*best_numerator > max_numerator) ||
		(*best_denominator > max_denominator)) {
		*best_numerator = 0;
		*best_denominator = 0;
	}
}

/**
 * Calculate the number of set bits (8-bit size).
 */
unsigned int no_os_util::no_os_hweight8(uint8_t word)
{
	uint32_t count = 0;

	while (word) {
		if (word & 0x1)
			count++;
		word >>= 1;
	}

	return count;
}

/**
 * Calculate the number of set bits (16-bit size).
 */
unsigned int no_os_util::no_os_hweight16(uint16_t word)
{
	return no_os_hweight8(word >> 8) +
		no_os_hweight8(word);
}

/**
 * Calculate the number of set bits (32-bit size).
 */
unsigned int no_os_util::no_os_hweight32(uint32_t word)
{
	return no_os_hweight16(word >> 16) +
		no_os_hweight16(word);
}

/**
 * Calculate the quotient and the remainder of an integer division.
 */
uint64_t no_os_util::no_os_do_div(uint64_t* n,
	uint64_t base)
{
	uint64_t mod = 0;

	mod = *n % base;
	*n = *n / base;

	return mod;
}

/**
 * Unsigned 64bit divide with 64bit divisor and remainder
 */
uint64_t no_os_util::no_os_div64_u64_rem(uint64_t dividend, uint64_t divisor,
	uint64_t* remainder)
{
	*remainder = dividend % divisor;

	return dividend / divisor;
}

/**
 * Unsigned 64bit divide with 32bit divisor with remainder
 */
uint64_t no_os_util::no_os_div_u64_rem(uint64_t dividend, uint32_t divisor,
	uint32_t* remainder)
{
	*remainder = no_os_do_div(&dividend, divisor);

	return dividend;
}

/**
 * Signed 64bit divide with 32bit divisor with remainder
 */
int64_t no_os_util::no_os_div_s64_rem(int64_t dividend, int32_t divisor, int32_t* remainder)
{
	*remainder = dividend % divisor;
	return dividend / divisor;
}

/**
 * Unsigned 64bit divide with 32bit divisor
 */
uint64_t no_os_util::no_os_div_u64(uint64_t dividend, uint32_t divisor)
{
	uint32_t remainder;

	return no_os_div_u64_rem(dividend, divisor, &remainder);
}

/**
 * Signed 64bit divide with 32bit divisor
 */
int64_t no_os_util::no_os_div_s64(int64_t dividend, int32_t divisor)
{
	int32_t remainder;
	return no_os_div_s64_rem(dividend, divisor, &remainder);
}

/**
 * Converts from string to int32_t
 * @param *str
 * @return int32_t
 */
int32_t no_os_util::no_os_str_to_int32(const char* str)
{
	char* end;
	int32_t value = strtol(str, &end, 0);

	if (end == str)
		return -EINVAL;
	else
		return value;
}

/**
 * Converts from string to uint32_t
 * @param *str
 * @return uint32_t
 */
uint32_t no_os_util::no_os_str_to_uint32(const char* str)
{
	char* end;
	uint32_t value = strtoul(str, &end, 0);

	if (end == str)
		return -EINVAL;
	else
		return value;
}

void no_os_util::no_os_put_unaligned_be16(uint16_t val, uint8_t* buf)
{
	buf[1] = val & 0xFF;
	buf[0] = val >> 8;
}

uint16_t no_os_util::no_os_get_unaligned_be16(uint8_t* buf)
{
	return buf[1] | ((uint16_t)buf[0] << 8);
}

void no_os_util::no_os_put_unaligned_le16(uint16_t val, uint8_t* buf)
{
	buf[0] = val & 0xFF;
	buf[1] = val >> 8;
}

uint16_t no_os_util::no_os_get_unaligned_le16(uint8_t* buf)
{
	return buf[0] | ((uint16_t)buf[1] << 8);
}

void no_os_util::no_os_put_unaligned_be24(uint32_t val, uint8_t* buf)
{
	buf[2] = val & 0xFF;
	buf[1] = (val >> 8) & 0xFF;
	buf[0] = val >> 16;
}

uint32_t no_os_util::no_os_get_unaligned_be24(uint8_t* buf)
{
	return buf[2] | ((uint16_t)buf[1] << 8) | ((uint32_t)buf[0] << 16);
}

void no_os_util::no_os_put_unaligned_le24(uint32_t val, uint8_t* buf)
{
	buf[0] = val & 0xFF;
	buf[1] = (val >> 8) & 0xFF;
	buf[2] = val >> 16;
}

uint32_t no_os_util::no_os_get_unaligned_le24(uint8_t* buf)
{
	return buf[0] | ((uint16_t)buf[1] << 8) | ((uint32_t)buf[2] << 16);
}

void no_os_util::no_os_put_unaligned_be32(uint32_t val, uint8_t* buf)
{
	buf[3] = val & 0xFF;
	buf[2] = (val >> 8) & 0xFF;
	buf[1] = (val >> 16) & 0xFF;
	buf[0] = val >> 24;
}

uint32_t no_os_util::no_os_get_unaligned_be32(uint8_t* buf)
{
	return buf[3] | ((uint16_t)buf[2] << 8) | ((uint32_t)buf[1] << 16)
		| ((uint32_t)buf[0] << 24);
}

void no_os_util::no_os_put_unaligned_le32(uint32_t val, uint8_t* buf)
{
	buf[0] = val & 0xFF;
	buf[1] = (val >> 8) & 0xFF;
	buf[2] = (val >> 16) & 0xFF;
	buf[3] = val >> 24;
}

uint32_t no_os_util::no_os_get_unaligned_le32(uint8_t* buf)
{
	return buf[0] | ((uint16_t)buf[1] << 8) | ((uint32_t)buf[2] << 16)
		| ((uint32_t)buf[3] << 24);
}

int16_t no_os_util::no_os_sign_extend16(uint16_t value, int index)
{
	uint8_t shift = 15 - index;
	return (int16_t)(value << shift) >> shift;
}

int32_t no_os_util::no_os_sign_extend32(uint32_t value, int index)
{
	uint8_t shift = 31 - index;
	return (int32_t)(value << shift) >> shift;
}

uint64_t no_os_util::no_os_mul_u32_u32(uint32_t a, uint32_t b)
{
	return (uint64_t)a * b;
}

uint64_t no_os_util::no_os_mul_u64_u32_shr(uint64_t a, uint32_t mul, unsigned int shift)
{
	uint32_t ah, al;
	uint64_t ret;

	al = a;
	ah = a >> 32;

	ret = no_os_mul_u32_u32(al, mul) >> shift;
	if (ah)
		ret += no_os_mul_u32_u32(ah, mul) << (32 - shift);

	return ret;
}

/**
 * @brief Check big endianess of the host processor.
 * @return Big endianess status (true/false)
 */
bool no_os_util::no_os_is_big_endian(void)
{
	uint16_t a = 0x0100;
	return (bool)*(uint8_t*)&a;
}

/* @brief Swap bytes in a buffer with a given step
 *        Swap with step of 2:
 *        AA BB CC DD EE FF 00 11 becomes
 *        BB AA DD CC FF EE 11 00
 *        Swap with step of 3:
 *        AA BB CC DD EE FF 00 11 22 becomes
 *        CC BB AA FF EE DD 22 11 00
 *        etc.
 * @param buf - Input buffer to be swapped.
 * @param bytes - Number of bytes.
 * @param step - Number of steps.
 * @return None
 */
void no_os_util::no_os_memswap64(void* buf, uint32_t bytes, uint32_t step)
{
	uint8_t* p = (uint8_t*)buf;
	uint32_t i, j;
	uint8_t temp[8];

	if (step < 2 || step > 8 || bytes < step || bytes % step != 0)
		return;

	for (i = 0; i < bytes; i += step) {
		memcpy(temp, p, step);
		for (j = step; j > 0; j--) {
			*p++ = temp[j - 1];
		}
	}
}
