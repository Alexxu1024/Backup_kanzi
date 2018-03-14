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

package kanzi.function;

import java.util.Map;
import kanzi.ByteTransform;
import kanzi.transform.BWTS;
import kanzi.transform.MTFT;
import kanzi.transform.SBRT;


public class ByteFunctionFactory
{
   // Up to 15 transforms can be declared (4 bit index)
   public static final short NONE_TYPE    = 0;  // copy
   public static final short BWT_TYPE     = 1;  // Burrows Wheeler
   public static final short BWTS_TYPE    = 2;  // Burrows Wheeler Scott
   public static final short LZ4_TYPE     = 3;  // LZ4
   public static final short SNAPPY_TYPE  = 4;  // Snappy
   public static final short RLT_TYPE     = 5;  // Run Length 
   public static final short ZRLT_TYPE    = 6;  // Zero Run Length
   public static final short MTFT_TYPE    = 7;  // Move To Front
   public static final short RANK_TYPE    = 8;  // Rank
   public static final short X86_TYPE     = 9;  // X86 codec
   public static final short DICT_TYPE    = 10; // Text codec
 

   // The returned type contains 8 (nibble based) transform values
   public int getType(String name)
   {
      if (name.indexOf('+') < 0)
         return this.getTypeToken(name) << 28;
      
      String[] tokens = name.split("\\+");
      
      if (tokens.length == 0)
         throw new IllegalArgumentException("Unknown transform type: " + name);

      if (tokens.length > 8)
         throw new IllegalArgumentException("Only 8 transforms allowed: " + name);

      int res = 0;
      int shift = 28;
      
      for (String token: tokens)
      {
         final int typeTk = this.getTypeToken(token);
         
         // Skip null transform
         if (typeTk != NONE_TYPE)
         {
            res |= (typeTk << shift);
            shift -= 4;
         }
      }
      
      return res;
   }
   
   
   private int getTypeToken(String name)
   {
      // Strings in switch not supported in JDK 6
      name = String.valueOf(name).toUpperCase();
      
      switch (name)
      {
         case "BWT":
            return BWT_TYPE;

         case "BWTS":
            return BWTS_TYPE;

         case "SNAPPY":
            return SNAPPY_TYPE;

         case "LZ4":
            return LZ4_TYPE;

         case "MTFT":
            return MTFT_TYPE;

         case "ZRLT":
            return ZRLT_TYPE;

         case "RLT":
            return RLT_TYPE;

         case "RANK":
            return RANK_TYPE;

         case "X86":
            return X86_TYPE;

         case "TEXT":
            return DICT_TYPE;

         case "NONE":
            return NONE_TYPE;

         default:
            throw new IllegalArgumentException("Unknown transform type: " + name);
      }
   }
   
   
   public ByteTransformSequence newFunction(Map<String, Object> ctx, int functionType)
   {      
      int nbtr = 0;
      
      // Several transforms
      for (int i=0; i<8; i++)
      {
         if (((functionType >>> (28-4*i)) & 0x0F) != NONE_TYPE)
            nbtr++;
      }
    
      // Only null transforms ? Keep first.
      if (nbtr == 0)
         nbtr = 1;
      
      ByteTransform[] transforms = new ByteTransform[nbtr];
      nbtr = 0;
      
      for (int i=0; i<transforms.length; i++)
      {
         int t = (functionType >>> (28-4*i)) & 0x0F;

         if ((t != NONE_TYPE) || (i == 0))
            transforms[nbtr++] = newFunctionToken(ctx, t);
      }
    
      return new ByteTransformSequence(transforms);
   }
   
   
   private static ByteTransform newFunctionToken(Map<String, Object> ctx, int functionType)
   {
      switch (functionType & 0x0F)
      {
         case SNAPPY_TYPE:
            return new SnappyCodec();
            
         case LZ4_TYPE:
            return new LZ4Codec();
            
         case BWT_TYPE:
            return new BWTBlockCodec(); 
            
         case BWTS_TYPE:
            return new BWTS();    
            
         case MTFT_TYPE:
            return new MTFT();

         case ZRLT_TYPE:
            return new ZRLT();
            
         case RLT_TYPE:
            return new RLT();
            
         case RANK_TYPE:
            return new SBRT(SBRT.MODE_RANK);
            
         case DICT_TYPE:
            return new TextCodec(ctx);
            
         case X86_TYPE:
            return new X86Codec();
            
         case NONE_TYPE:
            return new NullFunction();
            
         default:
            throw new IllegalArgumentException("Unknown transform type: " + functionType);
      }
   }

   
   public String getName(int functionType)
   {              
      StringBuilder sb = new StringBuilder();

      for (int i=0; i<8; i++)
      {
         final int t = functionType >>> (28-4*i);

         if ((t & 0x0F) == NONE_TYPE)
            continue;

         String name = getNameToken(t);

         if (sb.length() != 0)
            sb.append('+');

         sb.append(name);
      }

      if (sb.length() == 0)
         sb.append(getNameToken(NONE_TYPE));

      return sb.toString();
   }
   
   
   private static String getNameToken(int functionType)
   {
      switch (functionType & 0x0F)
      {
         case LZ4_TYPE:
            return "LZ4";
            
         case BWT_TYPE:
            return "BWT";

         case BWTS_TYPE:
            return "BWTS";
            
         case SNAPPY_TYPE:
            return "SNAPPY";
            
         case MTFT_TYPE:
            return "MTFT";

         case ZRLT_TYPE:
            return "ZRLT";
            
         case RLT_TYPE:
            return "RLT";
            
         case RANK_TYPE:
            return "RANK";
            
         case X86_TYPE:
            return "X86";
            
         case DICT_TYPE:
            return "TEXT";
            
         case NONE_TYPE:
            return "NONE";
            
         default:
            throw new IllegalArgumentException("Unknown transform type: " + functionType);
      }
   }
}