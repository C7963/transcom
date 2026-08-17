#pragma once

/*******************************************************************************
 *   @file   no_os_alloc.h
 *   @brief  Header file of memory allocator.
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
#ifndef _NO_OS_ALLOC_H_
#define _NO_OS_ALLOC_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#define NO_OS_EOVERRUN	(2000 + 1) /* Circular buffer overrun */
#define NO_OS_IS_ERR_VALUE(x)	((x) < 0)
#define NO_OS_GPIO_OUT	0x01
#define NO_OS_GPIO_IN	0x00
#define	NO_OS_SPI_CPHA	0x01
#define	NO_OS_SPI_CPOL	0x02
#define SPI_MAX_BUS_NUMBER 8
#define NO_OS_BIT(x)	(1 << (x))

#define NO_OS_ARRAY_SIZE(x) \
	(sizeof(x) / sizeof((x)[0]))

#define NO_OS_DIV_ROUND_UP(x,y) \
	(((x) + (y) - 1) / (y))
#define NO_OS_DIV_ROUND_CLOSEST(x, y) \
	(((x) + (y) / 2) / (y))
#define NO_OS_DIV_ROUND_CLOSEST_ULL(x, y) \
	NO_OS_DIV_ROUND_CLOSEST(x, y)

#define no_os_min(x, y) \
	(((x) < (y)) ? (x) : (y))
#define no_os_min_t(type, x, y) \
	(type)no_os_min((type)(x), (type)(y))

#define no_os_max(x, y) \
	(((x) > (y)) ? (x) : (y))
#define no_os_max_t(type, x, y) \
	(type)no_os_max((type)(x), (type)(y))

#define no_os_clamp(val, min_val, max_val) \
	(no_os_max(no_os_min((val), (max_val)), (min_val)))
#define no_os_clamp_t(type, val, min_val, max_val) \
	(type)no_os_clamp((type)(val), (type)(min_val), (type)(max_val))

#define no_os_swap(x, y) \
	{typeof(x) _tmp_ = (x); (x) = (y); (y) = _tmp_;}

#define no_os_round_up(x,y) \
		(((x)+(y)-1)/(y))

#define NO_OS_BITS_PER_LONG 32

//#define NO_OS_GENMASK(h, l) ({ 					\
//		uint32_t t = (uint32_t)(~0UL);			\
//		t = t << (NO_OS_BITS_PER_LONG - (h - l + 1));		\
//		t = t >> (NO_OS_BITS_PER_LONG - (h + 1));		\
//		t;						\
//})

//用于产生一个从l开始到h结束的连续的bitmask：
#define NO_OS_GENMASK(h, l) (((~0UL) - (1UL << (l)) + 1)& (~0UL >> (NO_OS_BITS_PER_LONG - 1 - (h))))

#define no_os_bswap_constant_32(x) \
	((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
	 (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#define no_os_bswap_constant_16(x) ((((x) & (uint16_t)0xff00) >> 8) | \
				 (((x) & (uint16_t)0x00ff) << 8))

#define no_os_bit_swap_constant_8(x) \
	((((x) & 0x80) >> 7) | \
	 (((x) & 0x40) >> 5) | \
	 (((x) & 0x20) >> 3) | \
	 (((x) & 0x10) >> 1) | \
	 (((x) & 0x08) << 1) | \
	 (((x) & 0x04) << 3) | \
	 (((x) & 0x02) << 5) | \
	 (((x) & 0x01) << 7))

#define NO_OS_U16_MAX		((uint16_t)~0U)
#define NO_OS_S16_MAX		((int16_t)(NO_OS_U16_MAX>>1))

#define NO_OS_DIV_U64(x, y) (x / y)

#define NO_OS_UNUSED_PARAM(x) ((void)x)

#define no_os_shift_right(x, s) ((x) < 0 ? -(-(x) >> (s)) : (x) >> (s))

#define no_os_align(x, align) (((x) + ((typeof(x))(align) - 1)) & ~((typeof(x))(align) - 1))

#define no_os_bcd2bin(x)	(((x) & 0x0f) + ((x) >> 4) * 10)
#define no_os_bin2bcd(x)	((((x) / 10) << 4) + (x) % 10)

#define NO_OS_CONTAINER_OF(ptr, type, name) ((type *)((char *)(ptr) - offsetof(type, name)))

namespace NO_OS {
	struct no_os_time {
		unsigned int s, us;
	};
	/**
 * @struct no_os_gpio_platform_ops
 * @brief Structure holding gpio function pointers that point to the platform
 * specific function
 */
	struct no_os_gpio_platform_ops;

	/**
	 * @enum no_os_gpio_pull_up
	 * @brief Enum that holds the possible pull up/ pull down resistor configuration.
	 */
	enum no_os_gpio_pull_up {
		NO_OS_PULL_NONE,
		/** Strong pull up */
		NO_OS_PULL_UP,
		/** Strong pull down */
		NO_OS_PULL_DOWN,
		NO_OS_PULL_UP_WEAK,
		NO_OS_PULL_DOWN_WEAK
	};

	/**
	 * @struct no_os_gpio_init_param
	 * @brief Structure holding the parameters for GPIO initialization.
	 */
	struct no_os_gpio_init_param {
		/** Port number */
		int32_t		port;
		/** GPIO number */
		int32_t		number;
		/** Pull up/down resistor configuration */
		enum no_os_gpio_pull_up pull;
		/** GPIO platform specific functions */
		const struct no_os_gpio_platform_ops* platform_ops;
		/** GPIO extra parameters (device specific) */
		void* extra;
	};

	/**
	 * @struct no_os_gpio_desc
	 * @brief Structure holding the GPIO descriptor.
	 */
	struct no_os_gpio_desc {
		/** Port number */
		int32_t		port;
		/** GPIO number */
		int32_t		number;
		/** Pull up/down resistor configuration */
		enum no_os_gpio_pull_up pull;
		/** GPIO platform specific functions */
		const struct no_os_gpio_platform_ops* platform_ops;
		/** GPIO extra parameters (device specific) */
		void* extra;
	};

	/**
	 * @enum no_os_gpio_values
	 * @brief Enum that holds the possible output states of a GPIO.
	 */
	enum no_os_gpio_values {
		/** GPIO logic low */
		NO_OS_GPIO_LOW,
		/** GPIO logic high */
		NO_OS_GPIO_HIGH,
		/** GPIO high impedance */
		NO_OS_GPIO_HIGH_Z
	};

	/**
	 * @struct no_os_gpio_platform_ops
	 * @brief Structure holding gpio function pointers that point to the platform
	 * specific function
	 */
	struct no_os_gpio_platform_ops {
		/** gpio initialization function pointer */
		int32_t(*gpio_ops_get)(struct no_os_gpio_desc**,
			const struct no_os_gpio_init_param*);
		/** gpio optional descriptor function pointer */
		int32_t(*gpio_ops_get_optional)(struct no_os_gpio_desc**,
			const struct no_os_gpio_init_param*);
		/** gpio remove function pointer */
		int32_t(*gpio_ops_remove)(struct no_os_gpio_desc*);
		/** gpio direction input function pointer */
		int32_t(*gpio_ops_direction_input)(struct no_os_gpio_desc*);
		/** gpio direction output function pointer */
		int32_t(*gpio_ops_direction_output)(struct no_os_gpio_desc*, uint8_t);
		/** gpio get direction function pointer */
		int32_t(*gpio_ops_get_direction)(struct no_os_gpio_desc*, uint8_t*);
		/** gpio set value function pointer */
		int32_t(*gpio_ops_set_value)(struct no_os_gpio_desc*, uint8_t);
		/** gpio get value function pointer */
		int32_t(*gpio_ops_get_value)(struct no_os_gpio_desc*, uint8_t*);
	};

	/**
 * @enum no_os_spi_mode
 * @brief SPI configuration for clock phase and polarity.
 */
	enum no_os_spi_mode {
		/** Data on rising, shift out on falling */
		NO_OS_SPI_MODE_0 = (0 | 0),
		/** Data on falling, shift out on rising */
		NO_OS_SPI_MODE_1 = (0 | NO_OS_SPI_CPHA),
		/** Data on rising, shift out on falling */
		NO_OS_SPI_MODE_2 = (NO_OS_SPI_CPOL | 0),
		/** Data on falling, shift out on rising */
		NO_OS_SPI_MODE_3 = (NO_OS_SPI_CPOL | NO_OS_SPI_CPHA)
	};

	/**
	 * @enum no_os_spi_bit_order
	 * @brief SPI configuration for bit order (MSB/LSB).
	 */
	enum no_os_spi_bit_order {
		/** Most-significant bit (MSB) first */
		NO_OS_SPI_BIT_ORDER_MSB_FIRST = 0,
		/** Least-significant bit (LSB) first */
		NO_OS_SPI_BIT_ORDER_LSB_FIRST = 1,
	};

	/**
	 * @struct no_os_spi_msg_list
	 * @brief List item describing a SPI transfer
	 */
	struct no_os_spi_msg {
		/** Buffer with data to send. If NULL, 0x00 will be sent */
		uint8_t* tx_buff;
		/** Buffer where to store data. If NULL, incoming data won't be saved */
		uint8_t* rx_buff;
		/** Length of buffers. Must have equal size. */
		uint32_t		bytes_number;
		/** If set, CS will be deasserted after the transfer */
		uint8_t			cs_change;
		/**
		 * Minimum delay (in us) between the CS de-assert event of the current message
		 * and the assert of the next one.
		 */
		uint32_t		cs_change_delay;
		/** Delay (in us) between the CS assert and the first SCLK edge. */
		uint32_t		cs_delay_first;
		/** Delay (in us) between the last SCLK edge and the CS deassert */
		uint32_t		cs_delay_last;
	};

	/**
	 * @struct no_os_platform_spi_delays
	 * @brief Delays resulted from components in the SPI signal path. The values is ns.
	 */
	struct no_os_platform_spi_delays {
		uint32_t cs_delay_first;
		uint32_t cs_delay_last;
	};

	/**
	 * @struct no_os_spi_platform_ops
	 * @brief Structure holding SPI function pointers that point to the platform
	 * specific function
	 */
	struct no_os_spi_platform_ops;

	/**
	 * @struct no_os_spi_init_param
	 * @brief Structure holding the parameters for SPI initialization
	 */
	struct no_os_spi_init_param {
		/** Device ID */
		uint32_t	device_id;
		/** maximum transfer speed */
		uint32_t	max_speed_hz;
		/** SPI chip select */
		uint8_t		chip_select;
		/** SPI mode */
		enum no_os_spi_mode	mode;
		/** SPI bit order */
		enum no_os_spi_bit_order	bit_order;
		const struct no_os_spi_platform_ops* platform_ops;
		struct no_os_platform_spi_delays platform_delays;
		/**  SPI extra parameters (device specific) */
		void* extra;
		/** Parent of the device */
		struct no_os_spi_desc* parent;
	};

	/**
	 * @struct no_os_spibus_desc
	 * @brief SPI bus descriptor
	*/
	struct no_os_spibus_desc {
		/** SPI bus mutex (lock) */
		void* mutex;
		/** SPI bus slave number*/
		uint8_t         slave_number;
		/** SPI bus device id */
		uint32_t	device_id;
		/** SPI bus max speed */
		uint32_t	max_speed_hz;
		/** SPI bus mode */
		enum no_os_spi_mode	mode;
		/** SPI bus bit order */
		enum no_os_spi_bit_order	bit_order;
		/** SPI bus platform ops */
		const struct no_os_spi_platform_ops* platform_ops;
		/** SPI bus extra */
		void* extra;
	};

	/**
	 * @struct no_os_spi_desc
	 * @brief Structure holding SPI descriptor.
	 */
	struct no_os_spi_desc {
		/** SPI bus address */
		struct no_os_spibus_desc* bus;
		/** SPI bus number (0 for SPI0, 1 for SPI1, ...) */
		uint32_t	device_id;
		/** maximum transfer speed */
		uint32_t	max_speed_hz;
		/** SPI chip select */
		uint8_t		chip_select;
		/** SPI mode */
		enum no_os_spi_mode	mode;
		/** SPI bit order */
		enum no_os_spi_bit_order	bit_order;
		const struct no_os_spi_platform_ops* platform_ops;
		struct no_os_platform_spi_delays platform_delays;
		/**  SPI extra parameters (device specific) */
		void* extra;
		/** Parent of the device */
		struct no_os_spi_desc* parent;
	};

	/**
	 * @struct no_os_spi_platform_ops
	 * @brief Structure holding SPI function pointers that point to the platform
	 * specific function
	 */
	struct no_os_spi_platform_ops {
		/** SPI initialization function pointer */
		int32_t(*init)(struct no_os_spi_desc**, const struct no_os_spi_init_param*);
		/** SPI write/read function pointer */
		int32_t(*write_and_read)(struct no_os_spi_desc*, uint8_t*, uint16_t);
		/** Iterate over the spi_msg array and send all messages at once */
		int32_t(*transfer)(struct no_os_spi_desc*, struct no_os_spi_msg*, uint32_t);
		/** Iterate over the spi_msg array and send all messages using DMA.
		 * Blocks until the transfer is completed.
		 */
		int32_t(*dma_transfer_sync)(struct no_os_spi_desc*, struct no_os_spi_msg*,
			uint32_t);
		/** Iterate over the spi_msg array and send all messages using DMA.
		 * Returns immediately after the transfer is started and invokes a
		 * callback once all the messages have been transfered.
		 */
		int32_t(*dma_transfer_async)(struct no_os_spi_desc*, struct no_os_spi_msg*,
			uint32_t, void (*)(void*), void*);
		/** SPI remove function pointer */
		int32_t(*remove)(struct no_os_spi_desc*);
	};


	class no_os_alloc {
	public:
		/* Allocate memory and return a pointer to it */
		static void* no_os_malloc(size_t size);

		/* Allocate memory and return a pointer to it, set memory to 0 */
		static void* no_os_calloc(size_t nitems, size_t size);

		/* Deallocate memory previously allocated by a call to no_os_calloc or
		 * no_os_malloc */
		static void no_os_free(void* ptr);
	};


	class no_os_delay {
	public:
		/* Generate microseconds delay. */
		static void no_os_udelay(uint32_t usecs);

		/* Generate miliseconds delay. */
		static void no_os_mdelay(uint32_t msecs);

		/* Get current time */
		static struct no_os_time no_os_get_time(void);
	};


	class no_os_gpio {
	public:

		/* Obtain the GPIO decriptor. */
		static int32_t no_os_gpio_get(struct no_os_gpio_desc** desc,
			const struct no_os_gpio_init_param* param);

		/* Obtain optional GPIO descriptor. */
		int32_t no_os_gpio_get_optional(struct no_os_gpio_desc** desc,
			const struct no_os_gpio_init_param* param);

		/* Free the resources allocated by no_os_gpio_get(). */
		static int32_t no_os_gpio_remove(struct no_os_gpio_desc* desc);

		/* Enable the input direction of the specified GPIO. */
		int32_t no_os_gpio_direction_input(struct no_os_gpio_desc* desc);

		/* Enable the output direction of the specified GPIO. */
		static int32_t no_os_gpio_direction_output(struct no_os_gpio_desc* desc,
			uint8_t value);

		/* Get the direction of the specified GPIO. */
		int32_t no_os_gpio_get_direction(struct no_os_gpio_desc* desc,
			uint8_t* direction);

		/* Set the value of the specified GPIO. */
		static int32_t no_os_gpio_set_value(struct no_os_gpio_desc* desc,
			uint8_t value);

		/* Get the value of the specified GPIO. */
		int32_t no_os_gpio_get_value(struct no_os_gpio_desc* desc,
			uint8_t* value);
	};


	class no_os_mutex
	{
	public:
		/**
		* @brief Function for no-os mutex initialization and thread safety.
		* This function is implemented based on different platforms/OS libraries
		* that NO-OS supports. These mutex functions are used for thread safety
		* of peripherals. Since these functions don't return error values it is
		* the developers responsibility to implement the safety checks in case
		* new mutex implementation is being added, like the following:
		*
		* if ((*mutex) == NULL)
		* {
		*      //code to initialize the mutex
		* }
		*
		* Also these check are responsible not to allocate different mutexes
		* for the same peripheral descriptor.
		*/
		static void no_os_mutex_init(void** mutex);

		/**
		 * @brief Function for locking mutex
		*/
		static void no_os_mutex_lock(void* mutex);

		/**
		 * @brief Function for unlocking mutex
		*/
		static void no_os_mutex_unlock(void* mutex);

		/**
		 * @brief Function for removing the initialized mutex.
		 * This function is responsible to remove the allocated mutex. This function is
		 * also used by the peripherals mutex thread safety feature and in case
		 * new mutex implementation is going to be added, it is the developers
		 * responsibility to add extra check inside the function while de-allocating the memory.
		 *
		 * if (mutex != NULL)
		 * {
		 *      //code to de-allocate mutex
		 * }
		*/
		static void no_os_mutex_remove(void* mutex);
	};


	class no_os_spi {
	public:

		/******************************************************************************/
		/************************ Functions Declarations ******************************/
		/******************************************************************************/

		/* Initialize the SPI communication peripheral. */
		static int32_t no_os_spi_init(struct no_os_spi_desc** desc,
			const struct no_os_spi_init_param* param);

		/* Free the resources allocated by no_os_spi_init(). */
		static int32_t no_os_spi_remove(struct no_os_spi_desc* desc);

		/* Write and read data to/from SPI. */
		static int32_t no_os_spi_write_and_read(struct no_os_spi_desc* desc,
			uint8_t* data,
			uint16_t bytes_number);

		/* Iterate over the spi_msg array and send all messages at once */
		int32_t no_os_spi_transfer(struct no_os_spi_desc* desc,
			struct no_os_spi_msg* msgs,
			uint32_t len);

		/* Transfer a list of messages using DMA. Wait until all transfers are done */
		int32_t no_os_spi_transfer_dma_sync(struct no_os_spi_desc* desc,
			struct no_os_spi_msg* msgs,
			uint32_t len);
		/*
		 * Transfer a list of messages using DMA. Return once the first one started and
		 * invoke a callback when they are done.
		 */
		int32_t no_os_spi_transfer_dma_async(struct no_os_spi_desc* desc,
			struct no_os_spi_msg* msgs,
			uint32_t len,
			void (*callback)(void*),
			void* ctx);

		/* Initialize SPI bus descriptor*/
		static int32_t no_os_spibus_init(const struct no_os_spi_init_param* param);

		/* Free the resources allocated for SPI bus desc*/
		static void no_os_spibus_remove(uint32_t bus_number);
	};


	class no_os_util
	{
	public:
		/******************************************************************************/
		/************************ Functions Declarations ******************************/
		/******************************************************************************/
		/* Check if bit set */
		inline int no_os_test_bit(int pos, const volatile void* addr)
		{
			return (((const int*)addr)[pos / 32] >> pos) & 1UL;
		}

		/* Find first set bit in word. */
		uint32_t no_os_find_first_set_bit(uint32_t word);
		/* Find last set bit in word. */
		uint32_t no_os_find_last_set_bit(uint32_t word);
		/* Locate the closest element in an array. */
		uint32_t no_os_find_closest(int32_t val,
			const int32_t* array,
			uint32_t size);
		/* Shift the value and apply the specified mask. */
		uint32_t no_os_field_prep(uint32_t mask, uint32_t val);
		/* Get a field specified by a mask from a word. */
		uint32_t no_os_field_get(uint32_t mask, uint32_t word);
		/* Log base 2 of the given number. */
		int32_t no_os_log_base_2(uint32_t x);
		/* Find greatest common divisor of the given two numbers. */
		uint32_t no_os_greatest_common_divisor(uint32_t a,
			uint32_t b);
		/* Find lowest common multiple of the given two numbers. */
		uint32_t no_os_lowest_common_multiple(uint32_t a, uint32_t b);
		/* Calculate best rational approximation for a given fraction. */
		void no_os_rational_best_approximation(uint32_t given_numerator,
			uint32_t given_denominator,
			uint32_t max_numerator,
			uint32_t max_denominator,
			uint32_t* best_numerator,
			uint32_t* best_denominator);
		/* Calculate the number of set bits (8-bit size). */
		unsigned int no_os_hweight8(uint8_t word);
		/* Calculate the number of set bits (16-bit size). */
		unsigned int no_os_hweight16(uint16_t word);
		/* Calculate the number of set bits (32-bit size). */
		unsigned int no_os_hweight32(uint32_t word);
		/* Calculate the quotient and the remainder of an integer division. */
		static uint64_t no_os_do_div(uint64_t* n,
			uint64_t base);
		/* Unsigned 64bit divide with 64bit divisor and remainder */
		static uint64_t no_os_div64_u64_rem(uint64_t dividend, uint64_t divisor,
			uint64_t* remainder);
		/* Unsigned 64bit divide with 32bit divisor with remainder */
		uint64_t no_os_div_u64_rem(uint64_t dividend, uint32_t divisor,
			uint32_t* remainder);
		int64_t no_os_div_s64_rem(int64_t dividend, int32_t divisor,
			int32_t* remainder);
		/* Unsigned 64bit divide with 32bit divisor */
		uint64_t no_os_div_u64(uint64_t dividend, uint32_t divisor);
		int64_t no_os_div_s64(int64_t dividend, int32_t divisor);
		/* Converts from string to int32_t */
		int32_t no_os_str_to_int32(const char* str);
		/* Converts from string to uint32_t */
		uint32_t no_os_str_to_uint32(const char* str);

		void no_os_put_unaligned_be16(uint16_t val, uint8_t* buf);
		uint16_t no_os_get_unaligned_be16(uint8_t* buf);
		void no_os_put_unaligned_le16(uint16_t val, uint8_t* buf);
		uint16_t no_os_get_unaligned_le16(uint8_t* buf);
		void no_os_put_unaligned_be24(uint32_t val, uint8_t* buf);
		uint32_t no_os_get_unaligned_be24(uint8_t* buf);
		void no_os_put_unaligned_le24(uint32_t val, uint8_t* buf);
		uint32_t no_os_get_unaligned_le24(uint8_t* buf);
		void no_os_put_unaligned_be32(uint32_t val, uint8_t* buf);
		uint32_t no_os_get_unaligned_be32(uint8_t* buf);
		void no_os_put_unaligned_le32(uint32_t val, uint8_t* buf);
		uint32_t no_os_get_unaligned_le32(uint8_t* buf);

		int16_t no_os_sign_extend16(uint16_t value, int index);
		int32_t no_os_sign_extend32(uint32_t value, int index);
		uint64_t no_os_mul_u32_u32(uint32_t a, uint32_t b);
		uint64_t no_os_mul_u64_u32_shr(uint64_t a, uint32_t mul, unsigned int shift);

		bool no_os_is_big_endian(void);
		void no_os_memswap64(void* buf, uint32_t bytes, uint32_t step);
	};

}
#endif // _NO_OS_ALLOC_H_
