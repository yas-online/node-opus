#include <string.h>
#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>
#include <nan.h>

using namespace v8;
using namespace node;

#include "common.h"
#include "opus.h"

NAN_MODULE_INIT( InitEncoder );
NAN_MODULE_INIT( InitDecoder );

NAN_MODULE_INIT( InitOpus )
{
	Nan::HandleScope scope;

	CONST_INT( OPUS_AUTO );
	CONST_INT( OPUS_BITRATE_MAX );

	CONST_INT( OPUS_APPLICATION_VOIP );
	CONST_INT( OPUS_APPLICATION_AUDIO );
	CONST_INT( OPUS_APPLICATION_RESTRICTED_LOWDELAY );

	CONST_INT( OPUS_SIGNAL_VOICE );
	CONST_INT( OPUS_SIGNAL_MUSIC );

	CONST_INT( MIN_FRAME_SIZE );
	CONST_INT( MAX_FRAME_SIZE );
	CONST_INT( MAX_PACKET_SIZE );

	CONST_INT( INT16_BYTES );

	InitEncoder( target );
	InitDecoder( target );
}

NODE_MODULE( node_opus, InitOpus )
