"use strict";

const chai = require( "chai" );
const should = chai.should();

const opus = require( __dirname + "/../" );

const stream = require( "stream" );

describe( "Decoder", function()
{
	let iTime = 5;
	let hInput = null;

	before( function()
	{
		// Generate Input PCM
		let iSamples = opus.MAX_FRAME_SIZE * iTime;
		let hBuffer = Buffer.alloc( iSamples * opus.INT16_BYTES );

		for( let i = 0; i < iSamples; ++i ) hBuffer.writeInt16LE( Math.round( Math.sin( Math.PI * i * opus.INT16_BYTES * 123 / 48000 ) ) * Math.pow( 2, 14 ), i * opus.INT16_BYTES );

		let hEncoder = new opus.Encoder();

		// We encode hEncoder.frameSize 16-bit samples. This requires a hEncoder.frameSize * opus.INT16_BYTES buffer of bytes.
		let iBufferSize = hEncoder.frameSize * opus.INT16_BYTES;
		hInput = hEncoder.encode( hBuffer.slice( 0, iBufferSize ) );
	} );

	it( "should decode opus data into pcm data using direct call", function()
	{
		let hDecoder = new opus.Decoder();
		should.exist( hDecoder );
		hDecoder.should.be.a( "object" );

		let iBufferSize = hDecoder.frameSize * opus.INT16_BYTES;
		let hOutput = hDecoder.decode( hInput.slice( 0, iBufferSize ) );
		should.exist( hOutput );
		hOutput.should.be.a.instanceof( Buffer );

		hOutput.length.should.be.above( hInput.length );
		Buffer.byteLength( hOutput ).should.be.above( Buffer.byteLength( hInput ) );
	} );

	it( "should decode opus data into pcm data using piping", function( pDone )
	{
		let hDecoder = new opus.Decoder();
		should.exist( hDecoder );
		hDecoder.should.be.a( "object" );

		let bReadBuffer = false;
		let hInputStream = new stream.Readable( { read( iSize )
		{
			if( !bReadBuffer )
			{
				this.push( hInput );
				bReadBuffer = true;
			}
			else this.push( null );
		} } );
		should.exist( hInputStream );
		hInputStream.should.be.a( "object" );

		let hOutput = null;

		let hOutputStream = new stream.Writable( { write( hChunk, sEncoding, pCallback )
		{
			if( hOutput == null ) hOutput = hChunk;
			else hOutput = Buffer.concat( [hOutput, hChunk] );

			pCallback();
		} } );
		should.exist( hOutputStream );
		hOutputStream.should.be.a( "object" );

		hOutputStream.once( "finish", function()
		{
			should.exist( hOutput );
			hOutput.should.be.a.instanceof( Buffer );

			hOutput.length.should.be.above( hInput.length );
			Buffer.byteLength( hOutput ).should.be.above( Buffer.byteLength( hInput ) );

			pDone();
		} );

		hDecoder.pipe( hOutputStream );
		hInputStream.pipe( hDecoder );
	} );
} );
