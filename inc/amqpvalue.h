// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef ANQPVALUE_H
#define ANQPVALUE_H

#include "amqp_types.h"

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#include <cstdint>
#include <cstdbool>
#else
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#endif /* __cplusplus */

	typedef struct AMQP_VALUE_DATA_TAG* AMQP_VALUE;
	typedef unsigned char uuid[16];
	typedef int64_t timestamp;

	typedef struct amqp_binary_TAG
	{
		const void* bytes;
		uint32_t length;
	} amqp_binary;

	/* type handling */
	extern AMQP_VALUE amqpvalue_create_null(void);
	extern AMQP_VALUE amqpvalue_create_boolean(bool value);
	extern int amqpvalue_get_boolean(AMQP_VALUE value, bool* bool_value);
	extern AMQP_VALUE amqpvalue_create_ubyte(unsigned char value);
	extern int amqpvalue_get_ubyte(AMQP_VALUE value, unsigned char* ubyte_value);
	extern AMQP_VALUE amqpvalue_create_ushort(uint16_t value);
	extern int amqpvalue_get_ushort(AMQP_VALUE value, uint16_t* ushort_value);
	extern AMQP_VALUE amqpvalue_create_uint(uint32_t value);
	extern int amqpvalue_get_uint(AMQP_VALUE value, uint32_t* uint_value);
	extern AMQP_VALUE amqpvalue_create_ulong(uint64_t value);
	extern int amqpvalue_get_ulong(AMQP_VALUE value, uint64_t* ulong_value);
	extern AMQP_VALUE amqpvalue_create_byte(char value);
	extern int amqpvalue_get_byte(AMQP_VALUE value, char* byte_value);
	extern AMQP_VALUE amqpvalue_create_short(int16_t value);
	extern int amqpvalue_get_short(AMQP_VALUE value, int16_t* short_value);
	extern AMQP_VALUE amqpvalue_create_int(int32_t value);
	extern int amqpvalue_get_int(AMQP_VALUE value, int32_t* int_value);
	extern AMQP_VALUE amqpvalue_create_long(int64_t value);
	extern int amqpvalue_get_long(AMQP_VALUE value, int64_t* long_value);
	extern AMQP_VALUE amqpvalue_create_float(float value);
	extern int amqpvalue_get_float(AMQP_VALUE value, float* float_value);
	extern AMQP_VALUE amqpvalue_create_double(double value);
	extern int amqpvalue_get_double(AMQP_VALUE value, double* double_value);
	extern AMQP_VALUE amqpvalue_create_char(uint32_t value);
	extern int amqpvalue_get_char(AMQP_VALUE value, uint32_t* char_value);
	extern AMQP_VALUE amqpvalue_create_timestamp(int64_t value);
	extern int amqpvalue_get_timestamp(AMQP_VALUE value, int64_t* timestamp_value);
	extern AMQP_VALUE amqpvalue_create_uuid(uuid value);
	extern int amqpvalue_get_uuid(AMQP_VALUE value, uuid* uuid_value);
	extern AMQP_VALUE amqpvalue_create_binary(amqp_binary value);
	extern int amqpvalue_get_binary(AMQP_VALUE value, amqp_binary* binary_value);
	extern AMQP_VALUE amqpvalue_create_string(const char* value);
	extern int amqpvalue_get_string(AMQP_VALUE value, const char** string_value);
	extern AMQP_VALUE amqpvalue_create_symbol(const char* value);
	extern int amqpvalue_get_symbol(AMQP_VALUE value, const char** symbol_value);
	extern AMQP_VALUE amqpvalue_create_list(void);
	extern int amqpvalue_set_list_item_count(AMQP_VALUE value, uint32_t count);
	extern int amqpvalue_get_list_item_count(AMQP_VALUE value, uint32_t* count);
	extern int amqpvalue_set_list_item(AMQP_VALUE value, uint32_t index, AMQP_VALUE list_item_value);
	extern AMQP_VALUE amqpvalue_get_list_item(AMQP_VALUE value, size_t index);
	extern AMQP_VALUE amqpvalue_create_map(void);
	extern int amqpvalue_set_map_value(AMQP_VALUE map, AMQP_VALUE key, AMQP_VALUE value);
	extern AMQP_VALUE amqpvalue_get_map_value(AMQP_VALUE map, AMQP_VALUE key);
	extern int amqpvalue_get_map_pair_count(AMQP_VALUE map, uint32_t* pair_count);
	extern int amqpvalue_get_map_key_value_pair(AMQP_VALUE map, uint32_t index, AMQP_VALUE* key, AMQP_VALUE* value);
	extern int amqpvalue_get_map(AMQP_VALUE value, AMQP_VALUE* map_value);
	extern AMQP_TYPE amqpvalue_get_type(AMQP_VALUE value);

	extern void amqpvalue_destroy(AMQP_VALUE value);

	extern bool amqpvalue_are_equal(AMQP_VALUE value1, AMQP_VALUE value2);
	extern AMQP_VALUE amqpvalue_clone(AMQP_VALUE value);

	/* encoding */
	typedef int(*AMQPVALUE_ENCODER_OUTPUT)(void* context, const unsigned char* bytes, size_t length);

	extern int amqpvalue_encode(AMQP_VALUE value, AMQPVALUE_ENCODER_OUTPUT encoder_output, void* context);
	extern int amqpvalue_get_encoded_size(AMQP_VALUE value, size_t* encoded_size);

	/* decoding */
	typedef void* AMQPVALUE_DECODER_HANDLE;
	typedef void(*ON_VALUE_DECODED)(void* context, AMQP_VALUE decoded_value);

	extern AMQPVALUE_DECODER_HANDLE amqpvalue_decoder_create(ON_VALUE_DECODED on_value_decoded, void* callback_context);
	extern void amqpvalue_decoder_destroy(AMQPVALUE_DECODER_HANDLE handle);
	extern int amqpvalue_decode_bytes(AMQPVALUE_DECODER_HANDLE handle, const unsigned char* buffer, size_t size);

	/* misc for now */
	extern AMQP_VALUE amqpvalue_create_array(void);
	extern int amqpvalue_get_array_item_count(AMQP_VALUE value, uint32_t* count);
	extern int amqpvalue_add_array_item(AMQP_VALUE value, AMQP_VALUE array_item_value);
	extern AMQP_VALUE amqpvalue_get_array_item(AMQP_VALUE value, uint32_t index);
	extern int amqpvalue_get_array(AMQP_VALUE value, AMQP_VALUE* array_value);

	extern AMQP_VALUE amqpvalue_get_inplace_descriptor(AMQP_VALUE value);
	extern AMQP_VALUE amqpvalue_get_inplace_described_value(AMQP_VALUE value);

	extern AMQP_VALUE amqpvalue_create_composite(AMQP_VALUE descriptor, uint32_t list_size);
	extern int amqpvalue_set_composite_item(AMQP_VALUE value, size_t index, AMQP_VALUE item_value);
	extern AMQP_VALUE amqpvalue_get_composite_item(AMQP_VALUE value, size_t index);
	extern AMQP_VALUE amqpvalue_create_described(AMQP_VALUE descriptor, AMQP_VALUE value);
	extern AMQP_VALUE amqpvalue_create_composite_with_ulong_descriptor(uint64_t descriptor);
	extern AMQP_VALUE amqpvalue_get_list_item_in_place(AMQP_VALUE value, size_t index);
	extern AMQP_VALUE amqpvalue_get_composite_item_in_place(AMQP_VALUE value, size_t index);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ANQPVALUE_H */
