#include "spdy_syn_reply.h"
#include "spdy_log.h"

#include <netinet/in.h>

/* Minimum length of a SYN_REPLY frame. */
const uint8_t SPDY_SYN_REPLY_MIN_LENGTH = 8;
/* Minimum length of a SYN_STREAM frame header. (The frame without
 * the NV Block.) */
const uint8_t SPDY_SYN_REPLY_HEADER_MIN_LENGTH = 6;

/**
 * Parse the header of a SYN_REPLY control frame.
 * This function can be used to parse the header of a SYN_REPLY frame
 * before the whole NV block has been received.
 * @param syn_reply - Destination frame.
 * @param data - Data to parse.
 * @param data_length - Length of data.
 * @see SPDY_SYN_REPLY_HEADER_MIN_LENGTH
 * @return 0 on success, 01 on failure.
 */
int spdy_syn_reply_parse_header(spdy_syn_reply *syn_reply, char *data, size_t data_length) {
	if(data_length < SPDY_SYN_REPLY_HEADER_MIN_LENGTH) {
		SPDYDEBUG("Not enough data for parsing the header.");
		return -1;
	}

	// Read the Stream-ID.
	syn_reply->stream_id = ntohl(*((uint32_t*)data)) & 0x7FFFFFFF;
	// Skip Stream-ID and 2 bytes of unused space.
	data += 6;

	return 0;
}

/**
 * Parse a SYN_REPLY control frame.
 * Parses the header of a SYN_REPLY control frame and extracts the
 * NV block.
 * @param syn_reply - Destination frame.
 * @param data - Data to parse.
 * @param data_length - Length of data.
 * @param zlib_ctx - The zlib context to use.
 * @see SPDY_SYN_STREAM_MIN_LENGTH
 * @return 0 on success, -1 on failure.
 */
int spdy_syn_reply_parse(spdy_syn_reply *syn_reply, char *data, size_t data_length, spdy_zlib_context *zlib_ctx) {
	if(data_length < SPDY_SYN_REPLY_MIN_LENGTH) {
		SPDYDEBUG("Not enough data for parsing the stream.");
		return -1;
	}

	// Parse the frame header.
	if(spdy_syn_reply_parse_header(syn_reply, data, data_length) < 0) {
		SPDYDEBUG("Failed to parse header.");
		return -1;
	}

	// Skip the (already parsed) header.
	data += SPDY_SYN_REPLY_HEADER_MIN_LENGTH;
	data_length -= SPDY_SYN_REPLY_HEADER_MIN_LENGTH;

	// Inflate NV block.
	char *inflate = NULL;
	size_t inflate_size = 0;
	if(spdy_zlib_inflate(zlib_ctx, data, data_length, &inflate, &inflate_size) < 0) {
		SPDYDEBUG("Failed to infalte data.");
		return -1;
	}

	// Allocate space for NV block.
	syn_reply->nv_block = malloc(sizeof(spdy_nv_block));
	if(!syn_reply->nv_block) {
		// Inflate gets allocated in spdy_zlib_inflate.
		free(inflate);
		SPDYDEBUG("Failed to allocate memory for nv_block.");
		return -1;
	}

	// Parse NV block.
	if(spdy_nv_block_parse(syn_reply->nv_block, inflate, inflate_size) < 0) {
		// Clean up.
		free(inflate);
		free(syn_reply->nv_block);
		SPDYDEBUG("Failed to parse NV block.");
		return -1;
	}

	// Only needed during parsing of NV block.
	free(inflate);

	return 0;
}

