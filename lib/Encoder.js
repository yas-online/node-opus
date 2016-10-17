
var util = require( 'util' );
var Transform = require( 'stream' ).Transform;
var opus = require( 'bindings' )( 'node-opus' );
var debug = require( 'debug' )( 'opus:encoder' );

// These are the valid rates for libopus according to
// https://www.opus-codec.org/docs/opus_api-1.1.2/group__opus__encoder.html#gaa89264fd93c9da70362a0c9b96b9ca88
var VALID_RATES = [8000, 12000, 16000, 24000, 48000]

var Encoder = function( rate, channels, frameDuration, application ) {
    Transform.call( this, { readableObjectMode: true } );

    this.rate = rate || 48000;

    // Ensure the range is valid.
    if( VALID_RATES.indexOf( this.rate ) === -1 ) {
        throw new RangeError(
                'Encoder rate (' + this.rate + ') is not valid. ' +
                'Valid rates are: ' + VALID_RATES.join( ', ' ) );
    }

    this.channels = channels || 1;
    this.frameDuration = frameDuration || 60;
    this.frameSize = this.rate * ( this.frameDuration / 1000 );

    this.application = application || opus.OPUS_APPLICATION_AUDIO;

    this.encoder = new opus.OpusEncoder( this.rate, this.channels, this.application );
    this.frameOverflow = new Buffer(0);

    debug( 'created new Encoder instance' );
};
util.inherits( Encoder, Transform );

/**
 * Transform stream callback
 */
Encoder.prototype._transform = function( buf, encoding, done ) {
    debug( '_transform(): %d bytes', buf.length );

    // Transform the buffer
    this._processOutput( buf );

    done();
};

Encoder.prototype._processOutput = function( buf ) {
    // Calculate the total data available and data required for each frame.
    var totalData = buf.length + this.frameOverflow.length;
    var requiredData = this.frameSize * opus.INT16_BYTES * this.channels;

    debug( '_processOutput(): totalData = %d bytes; requiredData = %d bytes', totalData, requiredData );

    // Process output while we got enough for a frame.
    while( totalData >= requiredData ) {

        // If we got overflow, use it up first.
        var buffer;
        if( this.frameOverflow && this.frameOverflow.length > 0 ) {

            buffer = Buffer.concat([
                this.frameOverflow,
                buf.slice( 0, requiredData - this.frameOverflow.length )
            ]);

            // Cut the already used part off the buf.
            buf = buf.slice( requiredData - this.frameOverflow.length );

            // Remove overflow. We'll set it later so it'll never be null
            // outside of this function.
            this.frameOverflow = null;

        } else {

            // We got no overflow.
            // Just cut the required bits from the buffer
            buffer = buf.slice( 0, requiredData );
            buf = buf.slice( requiredData );
        }

        // Flush frame and remove bits from the total data counter before
        // repeating loop.
        this._flushFrame( buffer );
        totalData -= requiredData;
    }

    // Store the remaining buffer in the overflow.
    this.frameOverflow = buf;
    debug( '_processOutput(): frameOverflow = %d bytes', this.frameOverflow.length );
};

Encoder.prototype._flushFrame = function( frame, maxPacketSize ) {
    debug( '_flushFrame(): %d bytes', frame.length );

    this.push( this.encoder.encode( frame, maxPacketSize ) );
};

Encoder.prototype.encode = function( pcm, maxPacketSize ) {
    debug( 'encode(): %d bytes <= %d', pcm.length, maxPacketSize );

    return this.encoder.encode( pcm, maxPacketSize );
};

module.exports = Encoder;
