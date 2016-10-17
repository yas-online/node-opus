#pragma once

#include <stdio.h>

#if defined(_DEBUG)

#define TRACE(msg) printf("   TRACE: %s\n", msg)
#define TRACE_S(msg, s) printf("   TRACE: %s : %s\n", msg, s)
#define TRACE_I(msg, i) printf("   TRACE: %s : %d\n", msg, i)
#define TRACE_CALL() printf("-> TRACE: Call::%s\n", __FUNCTION__)
#define TRACE_CALL_S(p1) printf("-> TRACE: Call::%s(%s)\n", __FUNCTION__, p1)
#define TRACE_CALL_I(p1) printf("-> TRACE: Call::%s(%d)\n", __FUNCTION__, p1)
#define TRACE_END() printf("<- Call::%s\n", __FUNCTION__)

#else

#define TRACE(msg)
#define TRACE_CALL()
#define TRACE_CALL_I(p1)
#define TRACE_END()

#endif

#define THROW_TYPE_ERROR( MSG ) \
	return Nan::ThrowTypeError( MSG );

#define CHECK_ARG(I, CHECK, DO_TRUE, DO_FALSE) \
	if ( info.Length() <= (I) || !info[I]->CHECK ) { DO_FALSE; } else { DO_TRUE; }

#define REQUIRE_ARG(I, CHECK) \
	CHECK_ARG( I, CHECK, , THROW_TYPE_ERROR("Argument " #I " must be an object") )

#define REQ_OBJ_ARG(I, VAR) \
	REQUIRE_ARG( I, IsObject() ) \
	Local<Object> VAR = Local<Object>::Cast( info[I] )

#define REQ_INT_ARG(I, VAR) \
	REQUIRE_ARG( I, IsNumber() ) \
	int VAR = info[I]->Int32Value()

#define OPT_INT_ARG(I, VAR, DEFAULT) \
	int VAR; \
	CHECK_ARG( I, IsNumber(), VAR = info[I]->Int32Value(), VAR = DEFAULT )

#define REQ_FUNC_ARG(I, VAR) \
	CHECK_ARG( I, IsFunction(), , THROW_TYPE_ERROR("Argument " #I " must be a function") ) \
	Local<Function> VAR = Local<Function>::Cast( info[I] );

#define CONST_INT(VALUE) \
	Nan::ForceSet( target, Nan::New<String>( #VALUE ).ToLocalChecked(), Nan::New<Integer>( VALUE ), \
		static_cast<PropertyAttribute>( ReadOnly | DontDelete ) );

// FrameSize begins at 20 ( ( 2.5ms / 1000 ) * 8000 ) on 8KHz (which is the min. sample rate, and 2,5ms is the min duration) per channel
#define MIN_FRAME_SIZE ( 20 )

// FrameSize can be up to 2880 ( ( 60ms / 1000 ) * 48000 ) on 48 KHz (which is the max. sample rate, and 60ms is the max duration) per channel
#define MAX_FRAME_SIZE ( 2880 )

// ( MAX_FRAME_SIZE * channels(2) ) = 5760
#define MAX_PACKET_SIZE ( MAX_FRAME_SIZE * 2 )

// JS's Buffer calculates in bytes, but since opus uses int16 we need to multiply the buffer size with 2
#define INT16_BYTES ( 2 )
