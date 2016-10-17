"use strict";

const chai = require( "chai" );
const should = chai.should();

const opus = require( __dirname + "/../" );

const stream = require( "stream" );

describe( "Encoder", function()
{
	let iTime = 5;
	let hInput = null;

	before( function()
	{
		// Generate Input PCM
		let iSamples = opus.MAX_FRAME_SIZE * iTime;
		hInput = Buffer.alloc( iSamples * opus.INT16_BYTES );

		for( let i = 0; i < iSamples; ++i ) hInput.writeInt16LE( Math.round( Math.sin( Math.PI * i * opus.INT16_BYTES * 123 / 48000 ) ) * Math.pow( 2, 14 ), i * opus.INT16_BYTES );
	} );
	
	it( "should encode pcm data into opus data using direct call", function()
	{
		let hEncoder = new opus.Encoder();
		should.exist( hEncoder );
		hEncoder.should.be.a( "object" );
		
		// ToDo: move to chunked test
/*
		let hBuffer = hInput.slice( 0, hInput.length );
		while( hBuffer.length > 0 )
		{
			// We encode hEncoder.frameSize 16-bit samples. This requires a hEncoder.frameSize * opus.INT16_BYTES buffer of bytes.
			let iBufferSize = hEncoder.frameSize * opus.INT16_BYTES;
			let hEncoded = hEncoder.encode( hBuffer.slice( 0, iBufferSize ) );

			for( let aEntry of hEncoded.entries() ) hOutput.writeInt16LE( aEntry[1], aEntry[0] );

			// Move the buffer forward by the buffer size.
			hBuffer = hBuffer.slice( iBufferSize );
		};
*/
		let iBufferSize = hEncoder.frameSize * opus.INT16_BYTES;
		let hOutput = hEncoder.encode( hInput.slice( 0, iBufferSize ) );
		should.exist( hOutput );
		hOutput.should.be.a.instanceof( Buffer );

		hOutput.length.should.be.below( hInput.length );
		Buffer.byteLength( hOutput ).should.be.below( Buffer.byteLength( hInput ) );
	} );

	it( "should encode pcm data into opus data using piping", function( pDone )
	{
		let hEncoder = new opus.Encoder();
		should.exist( hEncoder );
		hEncoder.should.be.a( "object" );

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

			hOutput.length.should.be.below( hInput.length );
			Buffer.byteLength( hOutput ).should.be.below( Buffer.byteLength( hInput ) );

			pDone();
		} );

		hEncoder.pipe( hOutputStream );
		hInputStream.pipe( hEncoder );
	} );
} );
