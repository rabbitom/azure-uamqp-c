// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "platform.h"
#include "message_sender.h"
#include "message.h"
#include "messaging.h"
#include "amqpalloc.h"
#include "saslclientio.h"
#include "sasl_mssbcbs.h"
#include "tlsio.h"
#include "consolelogger.h"
#include "cbs.h"
#include "strings.h"
#include "buffer_.h"
#include "base64.h"
#include "urlencode.h"
#include "sastoken.h"

#if _WIN32
#include "windows.h"
#endif

/* This sample connects to an Event Hub, authenticates using SASL MSSBCBS (SAS token given by a put-token) and sends 1 message to the EH specifying a publisher ID */
/* The SAS token is generated based on the policy name/key */
/* Replace the below settings with your own.*/

#define EH_HOST "<<<Replace with your own EH host (like myeventhub.servicebus.windows.net)>>>"
#define EH_KEY_NAME "<<<Replace with your own key name>>>"
#define EH_KEY "<<<Replace with your own key>>>"
#define EH_NAME "<<<Replace with your own EH name (like ingress_eh)>>>"

#define EH_PUBLISHER "test_publisher"

static const size_t msg_count = 1;
static unsigned int sent_messages = 0;
static bool auth = false;

static void on_cbs_operation_complete(void* context, CBS_OPERATION_RESULT cbs_operation_result, unsigned int status_code, const char* status_description)
{
	(void)context, status_code, status_description;

	if (cbs_operation_result == CBS_OPERATION_RESULT_OK)
	{
		auth = true;
	}
}

static void on_message_send_complete(const void* context, MESSAGE_SEND_RESULT send_result)
{
	(void)send_result;
	(void)context;

	sent_messages++;
}

int main(int argc, char** argv)
{
	int result;

	amqpalloc_set_memory_tracing_enabled(true);

	if (platform_init() != 0)
	{
		result = -1;
	}
	else
	{
		XIO_HANDLE sasl_io;
		CONNECTION_HANDLE connection;
		SESSION_HANDLE session;
		LINK_HANDLE link;
		MESSAGE_SENDER_HANDLE message_sender;
		MESSAGE_HANDLE message;

		size_t last_memory_used = 0;

		/* create SASL PLAIN handler */
		SASL_MECHANISM_HANDLE sasl_mechanism_handle = saslmechanism_create(saslmssbcbs_get_interface(), NULL);
		XIO_HANDLE tls_io;
        STRING_HANDLE sas_key_name;
        STRING_HANDLE sas_key_value;
        STRING_HANDLE resource_uri;
        STRING_HANDLE encoded_resource_uri;
        STRING_HANDLE sas_token;
        BUFFER_HANDLE buffer;

		/* create the TLS IO */
        TLSIO_CONFIG tls_io_config = { EH_HOST, 5671 };
		const IO_INTERFACE_DESCRIPTION* tlsio_interface = platform_get_default_tlsio();
		tls_io = xio_create(tlsio_interface, &tls_io_config, NULL);

		/* create the SASL client IO using the TLS IO */
		SASLCLIENTIO_CONFIG sasl_io_config = { tls_io, sasl_mechanism_handle };
		sasl_io = xio_create(saslclientio_get_interface_description(), &sasl_io_config, NULL);

		/* create the connection, session and link */
		connection = connection_create(sasl_io, EH_HOST, "some", NULL, NULL);
		session = session_create(connection, NULL, NULL);
		session_set_incoming_window(session, 2147483647);
		session_set_outgoing_window(session, 65536);

        /* Construct a SAS token */
        sas_key_name = STRING_construct(EH_KEY_NAME);

        /* unfortunately SASToken wants an encoded key - this should be fixed at a later time */
        buffer = BUFFER_create(EH_KEY, strlen(EH_KEY));
        sas_key_value = Base64_Encode(buffer);
        BUFFER_delete(buffer);
        resource_uri = STRING_construct("sb://" EH_HOST "/" EH_NAME "/publishers/" EH_PUBLISHER);
        encoded_resource_uri = URL_EncodeString(STRING_c_str(resource_uri));

        /* Make a token that expires in one hour */
        time_t currentTime = time(NULL);
        size_t expiry_time = (size_t)(difftime(currentTime, 0) + 3600);

        sas_token = SASToken_Create(sas_key_value, encoded_resource_uri, sas_key_name, expiry_time);

		CBS_HANDLE cbs = cbs_create(session, NULL, NULL);
		if (cbs_open(cbs) == 0)
		{
			(void)cbs_put_token(cbs, "servicebus.windows.net:sastoken", "sb://" EH_HOST "/" EH_NAME "/publishers/" EH_PUBLISHER, STRING_c_str(sas_token), on_cbs_operation_complete, cbs);

			while (!auth)
			{
				size_t current_memory_used;
				size_t maximum_memory_used;
				connection_dowork(connection);

				current_memory_used = amqpalloc_get_current_memory_used();
				maximum_memory_used = amqpalloc_get_maximum_memory_used();

				if (current_memory_used != last_memory_used)
				{
					printf("Current memory usage:%lu (max:%lu)\r\n", (unsigned long)current_memory_used, (unsigned long)maximum_memory_used);
					last_memory_used = current_memory_used;
				}
			}
		}

        STRING_delete(sas_token);
        STRING_delete(sas_key_name);
        STRING_delete(sas_key_value);
        STRING_delete(resource_uri);
        STRING_delete(encoded_resource_uri);

		AMQP_VALUE source = messaging_create_source("ingress");
		AMQP_VALUE target = messaging_create_target("amqps://" EH_HOST "/" EH_NAME);
		link = link_create(session, "sender-link", role_sender, source, target);
		link_set_snd_settle_mode(link, sender_settle_mode_settled);
		(void)link_set_max_message_size(link, 65536);

		amqpvalue_destroy(source);
		amqpvalue_destroy(target);

		message = message_create();
		unsigned char hello[] = { 'H', 'e', 'l', 'l', 'o' };
		BINARY_DATA binary_data = { hello, sizeof(hello) };
		message_add_body_amqp_data(message, binary_data);

		/* create a message sender */
		message_sender = messagesender_create(link, NULL, NULL, NULL);
		if (messagesender_open(message_sender) == 0)
		{
			uint32_t i;

#if _WIN32
			unsigned long startTime = (unsigned long)GetTickCount64();
#endif

			for (i = 0; i < msg_count; i++)
			{
				(void)messagesender_send(message_sender, message, on_message_send_complete, message);
			}

			message_destroy(message);

			while (true)
			{
				size_t current_memory_used;
				size_t maximum_memory_used;
				connection_dowork(connection);

				current_memory_used = amqpalloc_get_current_memory_used();
				maximum_memory_used = amqpalloc_get_maximum_memory_used();

				if (current_memory_used != last_memory_used)
				{
					printf("Current memory usage:%lu (max:%lu)\r\n", (unsigned long)current_memory_used, (unsigned long)maximum_memory_used);
					last_memory_used = current_memory_used;
				}

				if (sent_messages == msg_count)
				{
					break;
				}
			}

#if _WIN32
			unsigned long endTime = (unsigned long)GetTickCount64();

			printf("Send %lu messages in %lu ms: %.02f msgs/sec\r\n", msg_count, (endTime - startTime), (float)msg_count / ((float)(endTime - startTime) / 1000));
#endif
		}

		messagesender_destroy(message_sender);
		link_destroy(link);
		session_destroy(session);
		connection_destroy(connection);
		xio_destroy(sasl_io);
		xio_destroy(tls_io);
		saslmechanism_destroy(sasl_mechanism_handle);
		platform_deinit();

		printf("Max memory usage:%lu\r\n", (unsigned long)amqpalloc_get_maximum_memory_used());
		printf("Current memory usage:%lu\r\n", (unsigned long)amqpalloc_get_current_memory_used());

		result = 0;
	}

#ifdef _CRTDBG_MAP_ALLOC
	_CrtDumpMemoryLeaks();
#endif

	return result;
}
