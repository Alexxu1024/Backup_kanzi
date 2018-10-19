/*
Copyright 2011-2017 Frederic Langlet
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
you may obtain a copy of the License at

                http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "ExpGolombDecoder.hpp"

using namespace kanzi;

ExpGolombDecoder::ExpGolombDecoder(InputBitStream& bitstream, bool sgn)
    : _bitstream(bitstream), _signed(sgn)
{
}


int ExpGolombDecoder::decode(byte block[], uint blkptr, uint len)
{
    byte* buf = &block[blkptr];
    const uint len8 = len & uint(-8);

    for (uint i = 0; i < len8; i+=8) {
        buf[i]   = decodeByte();
        buf[i+1] = decodeByte();
        buf[i+2] = decodeByte();
        buf[i+3] = decodeByte();
        buf[i+4] = decodeByte();
        buf[i+5] = decodeByte();
        buf[i+6] = decodeByte();
        buf[i+7] = decodeByte();
	}

    for (uint i = len8; i < len; i++)
        buf[i] = decodeByte();

    return len;
}

inline byte ExpGolombDecoder::decodeByte()
{
    if (_bitstream.readBit() == 1)
        return 0;

    int log2 = 1;

    while (_bitstream.readBit() == 0)
        log2++;

    if (_signed == true) {
        // Decode signed: read value + sign
        byte res = byte(_bitstream.readBits(log2 + 1));
        const byte sgn = res & 1;
        res = (res >> 1) + (1 << log2) - 1;
        return byte((res - sgn) ^ -sgn); // res or -res
    }

    // Decode unsigned
    return byte((1 << log2) - 1 + _bitstream.readBits(log2));
}