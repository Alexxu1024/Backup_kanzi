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

#ifndef _TPAQPredictor_
#define _TPAQPredictor_

#include "AdaptiveProbMap.hpp"
#include "../Global.hpp"
#include "Predictor.hpp"


namespace kanzi
{

   // TPAQ predictor
   // Derived from a heavily modified version of Tangelo 2.4 (by Jan Ondrus).
   // PAQ8 is written by Matt Mahoney.
   // See http://encode.ru/threads/1738-TANGELO-new-compressor-(derived-from-PAQ8-FP8)

   // Mixer combines models using neural networks with 8 inputs.
   class TPAQMixer
   {
       friend class TPAQPredictor;

   public:
      TPAQMixer(); 

      ~TPAQMixer() { }

       void update(int bit);

       int get(int32 p0, int32 p1, int32 p2, int32 p3, int32 p4, int32 p5, int32 p6, int32 p7);

   private:
       int32 _w0, _w1, _w2, _w3, _w4, _w5, _w6, _w7;
       int32 _p0, _p1, _p2, _p3, _p4, _p5, _p6, _p7;
       int _pr;
       int32 _skew;
   };


   class TPAQPredictor : public Predictor
   {
   public:
       TPAQPredictor(int logStates=28);

       ~TPAQPredictor();

       void update(int bit);

       // Return the split value representing the probability of 1 in the [0..4095] range.
       int get() { return _pr; }

   private:
       static const int MAX_LENGTH = 88;
       static const int MIXER_SIZE = 16 * 1024;
       static const int BUFFER_SIZE = 64 * 1024 * 1024;
       static const int HASH_SIZE = 16 * 1024 * 1024;
       static const int MASK_BUFFER = BUFFER_SIZE - 1;
       static const int MASK_MIXER = MIXER_SIZE - 1;
       static const int MASK_HASH = HASH_SIZE - 1;
       static const int MASK1 = 0x80808080;
       static const int MASK2 = 0xF0F0F0F0;
       static const int C1 = 0xcc9e2d51;
       static const int C2 = 0x1b873593;
       static const int C3 = 0xe6546b64;
       static const int C4 = 0x85ebca6b;
       static const int C5 = 0xc2b2ae35;
       static const int HASH1 = 200002979;
       static const int HASH2 = 30005491;
       static const int HASH3 = 50004239;

       int _pr; // next predicted value (0-4095)
       int32 _c0; // bitwise context: last 0-7 bits with a leading 1 (1-255)
       int32 _c4; // last 4 whole bytes, last is in low 8 bits
       int32 _c8; // last 8 to 4 whole bytes, last is in low 8 bits
       int _bpos; // number of bits in c0 (0-7)
       int32 _pos;
       int32 _matchLen;
       int32 _matchPos;
       int32 _hash;
       LogisticAdaptiveProbMap<7> _apm;
       TPAQMixer* _mixers;
       TPAQMixer* _mixer; // current mixer
       byte* _buffer;
       int32* _hashes; // hash table(context, buffer position)
       uint8* _states; // hash table(context, prediction)
       const int32 _statesMask;
       uint8* _cp0; // context pointers
       uint8* _cp1; 
       uint8* _cp2; 
       uint8* _cp3; 
       uint8* _cp4; 
       uint8* _cp5; 
       uint8* _cp6; 
       int32 _ctx0; // contexts
       int32 _ctx1; 
       int32 _ctx2; 
       int32 _ctx3; 
       int32 _ctx4; 
       int32 _ctx5; 
       int32 _ctx6;  

       static int32 hash(int32 x, int32 y);

       int32 addContext(uint32 ctxId, uint32 cx);

       int addMatchContextPred();

       void findMatch();
  };

}
#endif