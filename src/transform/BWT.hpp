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

#ifndef _BWT_
#define _BWT_

#include "../Transform.hpp"
#include "../concurrent.hpp"
#include "DivSufSort.hpp"

namespace kanzi {
   // The Burrows-Wheeler Transform is a reversible transform based on
   // permutation of the data in the original message to reduce the entropy.

   // The initial text can be found here:
   // Burrows M and Wheeler D, [A block sorting lossless data compression algorithm]
   // Technical Report 124, Digital Equipment Corporation, 1994

   // See also Peter Fenwick, [Block sorting text compression - final report]
   // Technical Report 130, 1996

   // This implementation replaces the 'slow' sorting of permutation strings
   // with the construction of a suffix array (faster but more complex).
   //
   // E.G.    0123456789A
   // Source: mississippi\0
   // Suffixes:    rank  sorted
   // mississippi\0  0  -> 4             i\0
   //  ississippi\0  1  -> 3          ippi\0
   //   ssissippi\0  2  -> 10      issippi\0
   //    sissippi\0  3  -> 8    ississippi\0
   //     issippi\0  4  -> 2   mississippi\0
   //      ssippi\0  5  -> 9            pi\0
   //       sippi\0  6  -> 7           ppi\0
   //        ippi\0  7  -> 1         sippi\0
   //         ppi\0  8  -> 6      sissippi\0
   //          pi\0  9  -> 5        ssippi\0
   //           i\0  10 -> 0     ssissippi\0
   // Suffix array SA : 10 7 4 1 0 9 8 6 3 5 2
   // BWT[i] = input[SA[i]-1] => BWT(input) = pssm[i]pissii (+ primary index 4)
   // The suffix array and permutation vector are equal when the input is 0 terminated
   // The insertion of a guard is done internally and is entirely transparent.
   //
   // See https://code.google.com/p/libdivsufsort/source/browse/wiki/SACA_Benchmarks.wiki
   // for respective performance of different suffix sorting algorithms.
   //
   // This implementation extends the canonical algorithm to use up to MAX_CHUNKS primary
   // indexes (based on input block size). Each primary index corresponds to a data chunk.
   // Chunks may be inverted concurrently.
   template <class T>
   class InverseBigChunkTask : public Task<T> {
   private:
       uint32* _buffer1;
       byte* _buffer2;
       uint32* _buckets;
       int* _primaryIndexes;
       byte* _dst;
       int _pIdx0;
       int _startIdx;
       int _step;
       int _startChunk;
       int _endChunk;

   public:
       InverseBigChunkTask(uint32* buf1, byte* buf2, uint32* buckets, byte* output,
           int* primaryIndexes, int pIdx0, int startIdx, int step, int startChunk, int endChunk);
       ~InverseBigChunkTask() {}

       T call() THROW;
   };

   template <class T>
   class InverseRegularChunkTask : public Task<T> {
   private:
       uint32* _buffer;
       uint32* _buckets;
       int* _primaryIndexes;
       byte* _dst;
       int _pIdx0;
       int _startIdx;
       int _step;
       int _startChunk;
       int _endChunk;

   public:
       InverseRegularChunkTask(uint32* buf, uint32* buckets, byte* output,
           int* primaryIndexes, int pIdx0, int startIdx, int step, int startChunk, int endChunk);
       ~InverseRegularChunkTask() {}

       T call() THROW;
   };

   class BWT : public Transform<byte> {

   private:
       static const int MAX_BLOCK_SIZE = 1024 * 1024 * 1024; // 1 GB (30 bits)
       static const int BWT_MAX_HEADER_SIZE = 4;
       static const int BWT_MAX_CHUNKS = 8;

       uint32* _buffer1;
       byte* _buffer2;
       int* _buffer3;
       int _bufferSize;
       uint32 _buckets[256];
       int _primaryIndexes[8];
       DivSufSort _saAlgo;
       int _jobs;

       bool inverseBigBlock(SliceArray<byte>& input, SliceArray<byte>& output, int count);

       bool inverseRegularBlock(SliceArray<byte>& input, SliceArray<byte>& output, int count);

   public:
       BWT(int jobs = 1);

       ~BWT()
       {
           if (_buffer1 != nullptr)
               delete[] _buffer1;

           if (_buffer2 != nullptr)
               delete[] _buffer2;

           if (_buffer3 != nullptr)
               delete[] _buffer3;
       }

       bool forward(SliceArray<byte>& input, SliceArray<byte>& output, int length);

       bool inverse(SliceArray<byte>& input, SliceArray<byte>& output, int length);

       int getPrimaryIndex(int n) const { return _primaryIndexes[n]; }

       bool setPrimaryIndex(int n, int primaryIndex);

       static int maxBlockSize() { return MAX_BLOCK_SIZE - BWT_MAX_HEADER_SIZE; }

       static int getBWTChunks(int size);
   };
}
#endif