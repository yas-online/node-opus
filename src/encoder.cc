#include <string.h>
#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>
#include <nan.h>

#include <string>
#include <sstream>
#include <algorithm>
#include <vector>

using namespace v8;
using namespace node;

#include "common.h"
#include "opus.h"

class Encoder : public ObjectWrap {
	private:
		OpusEncoder* encoder;

		opus_int32 rate;
		int channels;
		int application;

	protected:
		int EnsureEncoder() {
			if( encoder != NULL ) return 0;
			int error;
			encoder = opus_encoder_create( rate, channels, application, &error );
			return error;
		}

	public:
		Encoder( opus_int32 rate, int channels, int application ):
			encoder( NULL ),
			rate( rate ), channels( channels ), application( application ) {
		}

		~Encoder() {
			if( encoder != NULL )
				opus_encoder_destroy( encoder );

			encoder = NULL;
		}

		static void Encode( const Nan::FunctionCallbackInfo< v8::Value >& info ) {

			// Unwrap the encoder.
			Encoder* self = ObjectWrap::Unwrap<Encoder>( info.This() );
			if( self->EnsureEncoder() != OPUS_OK ) {
				Nan::ThrowError( "Could not create encoder. Check the encoder parameters" );
				return;
			}

			TRACE_CALL();

			// Read the functiona rguments
			REQ_OBJ_ARG( 0, pcmBuffer );

			// Read the PCM data.
			char* pcmData = Buffer::Data( pcmBuffer );

			// Make sure we're able to contain the entire data
			OPT_INT_ARG( 1, maxPacketSize, static_cast<int>( Buffer::Length( pcmBuffer ) * INT16_BYTES ) );

			TRACE_CALL_I( maxPacketSize );

			size_t frameSize = Buffer::Length( pcmBuffer ) / INT16_BYTES / self->channels;

			TRACE_CALL_I( frameSize );

			// Keeping things efficent means we'll only allocate what we need, when we need it
			std::vector<unsigned char> outOpus;
			outOpus.reserve( maxPacketSize );

			// Encode the samples.
			int compressedLength = opus_encode( self->encoder, reinterpret_cast<opus_int16*>( pcmData ), frameSize, outOpus.data(), maxPacketSize );
			if( compressedLength < 0 ) {
				std::ostringstream error_string;
				error_string << "Failed to encode data: " << opus_strerror( compressedLength );
				return Nan::ThrowTypeError( error_string.str().c_str() );
			}

			TRACE_CALL_I( compressedLength );

			// Create a new result buffer.
			Nan::MaybeLocal<Object> actualBuffer = Nan::CopyBuffer( reinterpret_cast<char*>( outOpus.data() ), compressedLength );
			if( !actualBuffer.IsEmpty() )
				info.GetReturnValue().Set( actualBuffer.ToLocalChecked() );

			TRACE_END();
		}

		static void SetBitrate( const Nan::FunctionCallbackInfo< v8::Value >& info ) {

			REQ_INT_ARG( 0, bitrate );

			Encoder* self = ObjectWrap::Unwrap<Encoder>( info.This() );
			if( self->EnsureEncoder() != OPUS_OK ) {
				Nan::ThrowError( "Could not create encoder. Check the encoder parameters" );
				return;
			}

			if( opus_encoder_ctl( self->encoder, OPUS_SET_BITRATE( bitrate ) ) != OPUS_OK )
				return Nan::ThrowError( "Invalid bitrate" );
		}

		static void GetBitrate( const Nan::FunctionCallbackInfo< v8::Value >& info ) {

			Encoder* self = ObjectWrap::Unwrap<Encoder>( info.This() );
			if( self->EnsureEncoder() != OPUS_OK ) {
				Nan::ThrowError( "Could not create encoder. Check the encoder parameters" );
				return;
			}

			opus_int32 bitrate;
			opus_encoder_ctl( self->encoder, OPUS_GET_BITRATE( &bitrate ) );

			info.GetReturnValue().Set( Nan::New<v8::Integer>( bitrate ) );
		}

		static void SetComplexity( const Nan::FunctionCallbackInfo< v8::Value >& info ) {

			REQ_INT_ARG( 0, complexity );

			Encoder* self = ObjectWrap::Unwrap<Encoder>( info.This() );
			if( self->EnsureEncoder() != OPUS_OK )
			{
				Nan::ThrowError( "Could not create encoder. Check the encoder parameters" );
				return;
			}

			if( opus_encoder_ctl( self->encoder, OPUS_SET_COMPLEXITY( complexity ) ) != OPUS_OK )
				return Nan::ThrowError( "Invalid complexity" );
		}

		static void GetComplexity( const Nan::FunctionCallbackInfo< v8::Value >& info ) {

			Encoder* self = ObjectWrap::Unwrap<Encoder>( info.This() );
			if( self->EnsureEncoder() != OPUS_OK )
			{
				Nan::ThrowError( "Could not create encoder. Check the encoder parameters" );
				return;
			}

			opus_int32 complexity;
			opus_encoder_ctl( self->encoder, OPUS_GET_BITRATE( &complexity ) );

			info.GetReturnValue().Set( Nan::New<v8::Integer>( complexity ) );
		}

		static void SetSignal( const Nan::FunctionCallbackInfo< v8::Value >& info ) {

			REQ_INT_ARG( 0, signal_type );

			Encoder* self = ObjectWrap::Unwrap<Encoder>( info.This() );
			if( self->EnsureEncoder() != OPUS_OK )
			{
				Nan::ThrowError( "Could not create encoder. Check the encoder parameters" );
				return;
			}

			if( opus_encoder_ctl( self->encoder, OPUS_SET_SIGNAL( signal_type ) ) != OPUS_OK )
				return Nan::ThrowError( "Invalid signal type" );
		}

		static void GetSignal( const Nan::FunctionCallbackInfo< v8::Value >& info ) {

			Encoder* self = ObjectWrap::Unwrap<Encoder>( info.This() );
			if( self->EnsureEncoder() != OPUS_OK )
			{
				Nan::ThrowError( "Could not create encoder. Check the encoder parameters" );
				return;
			}

			opus_int32 signal_type;
			opus_encoder_ctl( self->encoder, OPUS_GET_SIGNAL( &signal_type ) );

			info.GetReturnValue().Set( Nan::New<v8::Integer>( signal_type ) );
		}

		static void New( const Nan::FunctionCallbackInfo< v8::Value >& info ) {

			if( !info.IsConstructCall()) {
				return Nan::ThrowTypeError("Use the new operator to construct the Encoder.");
			}

			OPT_INT_ARG( 0, rate, 48000 );
			OPT_INT_ARG( 1, channels, 1 );
			OPT_INT_ARG( 2, application, OPUS_APPLICATION_AUDIO );

			Encoder* encoder = new Encoder( rate, channels, application );

			encoder->Wrap( info.This() );
			info.GetReturnValue().Set( info.This() );
		}
};

NAN_MODULE_INIT( InitEncoder )
{
	Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>( Encoder::New );
	tpl->SetClassName( Nan::New( "OpusEncoder" ).ToLocalChecked() );
	tpl->InstanceTemplate()->SetInternalFieldCount( 1 );

	Nan::SetPrototypeMethod( tpl, "encode", Encoder::Encode );

	Nan::SetPrototypeMethod( tpl, "setBitrate", Encoder::SetBitrate );
	Nan::SetPrototypeMethod( tpl, "getBitrate", Encoder::GetBitrate );

	Nan::SetPrototypeMethod( tpl, "setComplexity", Encoder::SetComplexity );
	Nan::SetPrototypeMethod( tpl, "getComplexity", Encoder::GetComplexity );

	Nan::SetPrototypeMethod( tpl, "setSignal", Encoder::SetSignal );
	Nan::SetPrototypeMethod( tpl, "getSignal", Encoder::GetSignal );

	//v8::Persistent<v8::FunctionTemplate> constructor;
	//Nan::AssignPersistent(constructor, tpl);
	Nan::Set( target, Nan::New<String>( "OpusEncoder" ).ToLocalChecked(), tpl->GetFunction() );
}
