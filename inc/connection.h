// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef CONNECTION_H
#define CONNECTION_H

#include <stddef.h>
#include <stdint.h>
#include "amqp_frame_codec.h"
#include "amqp_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	typedef struct CONNECTION_INSTANCE_TAG* CONNECTION_HANDLE;
	typedef struct ENDPOINT_INSTANCE_TAG* ENDPOINT_HANDLE;

	typedef enum CONNECTION_STATE_TAG
	{
		/* Codes_SRS_CONNECTION_01_039: [START In this state a connection exists, but nothing has been sent or received. This is the state an implementation would be in immediately after performing a socket connect or socket accept.] */
		CONNECTION_STATE_START,

		/* Codes_SRS_CONNECTION_01_040: [HDR RCVD In this state the connection header has been received from the peer but a connection header has not been sent.] */
		CONNECTION_STATE_HDR_RCVD,

		/* Codes_SRS_CONNECTION_01_041: [HDR SENT In this state the connection header has been sent to the peer but no connection header has been received.] */
		CONNECTION_STATE_HDR_SENT,

		/* Codes_SRS_CONNECTION_01_042: [HDR EXCH In this state the connection header has been sent to the peer and a connection header has been received from the peer.] */
		CONNECTION_STATE_HDR_EXCH,

		/* Codes_SRS_CONNECTION_01_043: [OPEN PIPE In this state both the connection header and the open frame have been sent but nothing has been received.] */
		CONNECTION_STATE_OPEN_PIPE,

		/* Codes_SRS_CONNECTION_01_044: [OC PIPE In this state, the connection header, the open frame, any pipelined connection traffic, and the close frame have been sent but nothing has been received.] */
		CONNECTION_STATE_OC_PIPE,

		/* Codes_SRS_CONNECTION_01_045: [OPEN RCVD In this state the connection headers have been exchanged. An open frame has been received from the peer but an open frame has not been sent.] */
		CONNECTION_STATE_OPEN_RCVD,

		/* Codes_SRS_CONNECTION_01_046: [OPEN SENT In this state the connection headers have been exchanged. An open frame has been sent to the peer but no open frame has yet been received.] */
		CONNECTION_STATE_OPEN_SENT,

		/* Codes_SRS_CONNECTION_01_047: [CLOSE PIPE In this state the connection headers have been exchanged. An open frame, any pipelined connection traffic, and the close frame have been sent but no open frame has yet been received from the peer.] */
		CONNECTION_STATE_CLOSE_PIPE,

		/* Codes_SRS_CONNECTION_01_048: [OPENED In this state the connection header and the open frame have been both sent and received.] */
		CONNECTION_STATE_OPENED,

		/* Codes_SRS_CONNECTION_01_049: [CLOSE RCVD In this state a close frame has been received indicating that the peer has initiated an AMQP close.] */
		CONNECTION_STATE_CLOSE_RCVD,

		/* Codes_SRS_CONNECTION_01_053: [CLOSE SENT In this state a close frame has been sent to the peer. It is illegal to write anything more onto the connection, however there could potentially still be incoming frames.] */
		CONNECTION_STATE_CLOSE_SENT,

		/* Codes_SRS_CONNECTION_01_055: [DISCARDING The DISCARDING state is a variant of the CLOSE SENT state where the close is triggered by an error.] */
		CONNECTION_STATE_DISCARDING,

		/* Codes_SRS_CONNECTION_01_057: [END In this state it is illegal for either endpoint to write anything more onto the connection. The connection can be safely closed and discarded.] */
		CONNECTION_STATE_END
	} CONNECTION_STATE;

	typedef void(*ON_ENDPOINT_FRAME_RECEIVED)(void* context, AMQP_VALUE performative, uint32_t frame_payload_size, const unsigned char* payload_bytes);
	typedef void(*ON_CONNECTION_STATE_CHANGED)(void* context, CONNECTION_STATE new_connection_state, CONNECTION_STATE previous_connection_state);
	typedef bool(*ON_NEW_ENDPOINT)(void* context, ENDPOINT_HANDLE new_endpoint);

	extern CONNECTION_HANDLE connection_create(XIO_HANDLE io, const char* hostname, const char* container_id, ON_NEW_ENDPOINT on_new_endpoint, void* callback_context);
	extern void connection_destroy(CONNECTION_HANDLE connection);
	extern int connection_open(CONNECTION_HANDLE connection);
	extern int connection_listen(CONNECTION_HANDLE connection);
	extern int connection_close(CONNECTION_HANDLE connection, const char* condition_value, const char* description);
	extern int connection_set_max_frame_size(CONNECTION_HANDLE connection, uint32_t max_frame_size);
	extern int connection_get_max_frame_size(CONNECTION_HANDLE connection, uint32_t* max_frame_size);
	extern int connection_set_channel_max(CONNECTION_HANDLE connection, uint16_t channel_max);
	extern int connection_get_channel_max(CONNECTION_HANDLE connection, uint16_t* channel_max);
	extern int connection_set_idle_timeout(CONNECTION_HANDLE connection, milliseconds idle_timeout);
	extern int connection_get_idle_timeout(CONNECTION_HANDLE connection, milliseconds* idle_timeout);
	extern int connection_get_remote_max_frame_size(CONNECTION_HANDLE connection, uint32_t* remote_max_frame_size);
	extern void connection_dowork(CONNECTION_HANDLE connection);
	extern ENDPOINT_HANDLE connection_create_endpoint(CONNECTION_HANDLE connection);
	extern int connection_start_endpoint(ENDPOINT_HANDLE endpoint, ON_ENDPOINT_FRAME_RECEIVED on_frame_received, ON_CONNECTION_STATE_CHANGED on_connection_state_changed, void* context);
	extern int connection_endpoint_get_incoming_channel(ENDPOINT_HANDLE endpoint, uint16_t* incoming_channel);
	extern void connection_destroy_endpoint(ENDPOINT_HANDLE endpoint);
	extern int connection_encode_frame(ENDPOINT_HANDLE endpoint, const AMQP_VALUE performative, PAYLOAD* payloads, size_t payload_count, ON_SEND_COMPLETE on_send_complete, void* callback_context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CONNECTION_H */
