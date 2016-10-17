
var util = require( 'util' );
var Transform = require( 'stream' ).Transform;
var opus = require( 'bindings' )( 'node-opus' );
var debug = require( 'debug' )( 'opus:decoder' );

// These are the valid rates for libopus according to
// https://www.opus-codec.org/docs/opus_api-1.1.2/group__opus__encoder.html#gaa89264fd93c9da70362a0c9b96b9ca88
var VALID_RATES = [8000, 12000, 16000, 24000, 48000]

var Decoder = function( rate, channels, frameDuration ) {
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

    this.decoder = new opus.OpusDecoder( this.rate, this.channels, this.frameSize );

    debug( 'created new Decoder instance' );
};
util.inherits( Decoder, Transform );

/**
 * Transform stream callback
 */
Decoder.prototype._transform = function( packet, encoding, done ) {
    debug( '_transform(): %d bytes', packet.length );

    this._processInput( packet );

    done();
};

Decoder.prototype._processInput = function( packet ) {
    this.push( this.decode( packet ) );
};

Decoder.prototype.decode = function( packet ) {
    debug( 'decode(): %d bytes', packet.length );

    return this.decoder.decode( packet );
};

module.exports = Decoder;


