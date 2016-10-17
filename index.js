"use strict";

var bindings = require( 'bindings' )( 'node-opus' );
module.exports = exports = bindings;

exports.Encoder = require( './lib/Encoder' );
exports.Decoder = require( './lib/Decoder' );
