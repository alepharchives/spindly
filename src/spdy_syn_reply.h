#ifndef SPDY_SYN_REPLY_H_
#define SPDY_SYN_REPLY_H_ 1

#include "spdy_nv_block.h"
#include "spdy_zlib.h"

#include <stdint.h>

// Minimum length of a SYN_REPLY frame.
extern const uint8_t SPDY_SYN_REPLY_MIN_LENGTH;
// Minimum length of a SYN_REPLY frame header.
extern const uint8_t SPDY_SYN_REPLY_HEADER_MIN_LENGTH;

/**
 * Flags for SYN_REPLY frames.
 */
enum SPDY_SYN_REPLY_FLAGS {
	SPDY_SYN_REPLY_FLAG_FIN = 0x01  /*!< FLAG_FIN */
};

/**
 * SYN_REPLY control frame
 */
typedef struct {
	uint32_t stream_id;      /*!< 31 bit stream id */
	spdy_nv_block *nv_block; /*!< Name/Value block */
} spdy_syn_reply;

int spdy_syn_reply_parse_header(spdy_syn_reply *syn_reply, char *data, size_t data_length);
int spdy_syn_reply_parse(spdy_syn_reply *syn_reply, char *data, size_t data_length, spdy_zlib_context *zlib_ctx);

#endif
