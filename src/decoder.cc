#include <string.h>
#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>
#include <nan.h>

#include <string>
#include <sstream>
#include <vector>

using namespace v8;
using namespace node;

#include "common.h"
#include "opus.h"

class Decoder : public ObjectWrap {
	private:
		OpusDecoder* decoder;

		opus_int32 rate;
		int channels;
		int frame_size;

	protected:
		int EnsureDecoder() {
			if( decoder != NULL ) return 0;
			int error;
			decoder = opus_decoder_create( rate, channels, &error );
			return error;
		}

	public:
		Decoder( opus_int32 rate, int channels, int frame_size ):
			decoder( NULL ),
			rate( rate ), channels( channels ), frame_size( frame_size ) {
		}

		~Decoder() {
			if( decoder != NULL )
				opus_decoder_destroy( decoder );

			decoder = NULL;
		}

		static void Decode( const Nan::FunctionCallbackInfo< v8::Value >& info ) {

			Decoder* self = ObjectWrap::Unwrap<Decoder>( info.This() );
			if( self->EnsureDecoder() != OPUS_OK ) {
				Nan::ThrowError( "Could not create decoder. Check the decoder parameters" );
				return;
			}

			TRACE_CALL();

			REQ_OBJ_ARG( 0, compressedBuffer );

			// Read the compressed data.
			unsigned char* compressedData = ( unsigned char* )Buffer::Data( compressedBuffer );
			size_t compressedDataLength = Buffer::Length( compressedBuffer );

			TRACE_CALL_I( compressedDataLength );

			// Keeping things efficent means we'll only allocate what we need, when we need it
			std::vector<opus_int16> outPCM;
			outPCM.reserve( self->frame_size * INT16_BYTES * self->channels );

			// Encode the samples.
			int decodedSamples = opus_decode(
					self->decoder,
					compressedData,
					compressedDataLength,
					outPCM.data(),
					self->frame_size, /* decode_fex */ 0 );

			if( decodedSamples < 0 ) {
				std::ostringstream error_string;
				error_string << "Failed to decode data: " << opus_strerror( decodedSamples );
				return Nan::ThrowTypeError( error_string.str().c_str() );
			}

			TRACE_CALL_I( decodedSamples );

			// Create a new result buffer.
			int decodedLength = decodedSamples * INT16_BYTES * self->channels;
			Nan::MaybeLocal<Object> actualBuffer = Nan::CopyBuffer( reinterpret_cast<char*>( outPCM.data() ), decodedLength );
			if( !actualBuffer.IsEmpty() )
				info.GetReturnValue().Set( actualBuffer.ToLocalChecked() );

			TRACE_END();
		}

		static void SetGain( const Nan::FunctionCallbackInfo< v8::Value >& info ) {

			REQ_INT_ARG( 0, gain );

			Decoder* self = ObjectWrap::Unwrap<Decoder>( info.This() );
			if( self->EnsureDecoder() != OPUS_OK ) {
				Nan::ThrowError( "Could not create decoder. Check the decoder parameters" );
				return;
			}

			if( opus_decoder_ctl( self->decoder, OPUS_SET_GAIN( gain ) ) != OPUS_OK )
				return Nan::ThrowError( "Invalid gain" );
		}

		static void GetGain( const Nan::FunctionCallbackInfo< v8::Value >& info ) {

			Decoder* self = ObjectWrap::Unwrap<Decoder>( info.This() );
			if( self->EnsureDecoder() != OPUS_OK ) {
				Nan::ThrowError( "Could not create decoder. Check the decoder parameters" );
				return;
			}

			opus_int32 gain;
			opus_decoder_ctl( self->decoder, OPUS_GET_GAIN( &gain ) );

			info.GetReturnValue().Set( Nan::New<v8::Integer>( gain ) );
		}

		static void New( const Nan::FunctionCallbackInfo< v8::Value >& info ) {

			if( !info.IsConstructCall()) {
				return Nan::ThrowTypeError("Use the new operator to construct the Decoder.");
			}

			OPT_INT_ARG( 0, rate, 48000 );
			OPT_INT_ARG( 1, channels, 1 );
			OPT_INT_ARG( 2, frame_size, ( ( 60 / 1000 ) * rate ) );

			Decoder* decoder = new Decoder( rate, channels, frame_size );

			decoder->Wrap( info.This() );
			info.GetReturnValue().Set( info.This() );
		}
};

NAN_MODULE_INIT( InitDecoder )
{
	Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>( Decoder::New );
	tpl->SetClassName( Nan::New( "OpusDecoder" ).ToLocalChecked() );
	tpl->InstanceTemplate()->SetInternalFieldCount( 1 );

	Nan::SetPrototypeMethod( tpl, "decode", Decoder::Decode );

	Nan::SetPrototypeMethod( tpl, "setGain", Decoder::SetGain );
	Nan::SetPrototypeMethod( tpl, "getGain", Decoder::GetGain );

	//v8::Persistent<v8::FunctionTemplate> constructor;
	//Nan::AssignPersistent(constructor, tpl);
	Nan::Set( target, Nan::New<String>( "OpusDecoder" ).ToLocalChecked(), tpl->GetFunction() );
}
