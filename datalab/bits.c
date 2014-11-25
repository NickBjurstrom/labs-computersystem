/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* 
 * minusOne - return a value of -1 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 2
 *   Rating: 1
 */
int minusOne(void) {
  // ~0 = 111...111 = -1
  return ~0;
}

/* 
 * fitsShort - return 1 if x can be represented as a 
 *   16-bit, two's complement integer.
 *   Examples: fitsShort(33000) = 0, fitsShort(-32768) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 1
 */
int fitsShort(int x) {
  /* 
   * after left shift 16 and right shift 16, the left 16 of x is 00000..00 or 111...1111
   * so after shift, if x remains the same, then it means that x can be represent as 16-bit
  */
  return !(((x << 16) >> 16) ^ x); 

}
/*
 * isZero - returns 1 if x == 0, and 0 otherwise 
 *   Examples: isZero(5) = 0, isZero(0) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 2
 *   Rating: 1
 */
int isZero(int x) {
  // if x == 0, then !x == 1; if x != 0, then !x = 0;
  return !x;
}
/* 
 * isLess - if x < y  then return 1, else return 0 
 *   Example: isLess(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLess(int x, int y) {
  /*
   *11 ops
   * diff = x+(~y+1) : x-y
   * ~y&x : x<0, y>0 return 1
   * ~y|x : x>0, y<0 return 0
   * diff|(~x+y+1)) : x=y, x-y and y-x both equals 0, return 0

   * another way: 
   * int sx = x >> 31;               //sx is 1111...1 or 0000...0
   * int sy = y >> 31;               //sy is 1111...1 or 0000...0
   * int s_flag = sy ^ sx;           //s_flag = 0 when x's and y's significant bits are the same; s_flag = 1 when they are different
   * return (!(s_flag | (((~x)+ y) >> 31)) + (s_flag  & !(sy)));  //the first part !(s_flag | (((~x)+ y)>>31)) == (!s_flag & !(((~x)+y)>>31)), which is when x and y are '++' or '--'; the second part is when x and y are '+-' or '-+'. 
   *
   *
   */
  int diff = x+(~y+1);
  return (((((diff) | (~y&x)) & (~y|x))) >> 31) & 1;
}
/* 
 * absVal - absolute value of x
 *   Example: absVal(-1) = 1.
 *   You may assume -TMax <= x <= TMax
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 4
 */
int absVal(int x) {
  /*
   * mask: x>>31 = 000000..0000 or 1111...111
   * if x > 0, mask = 00..0000, then mask^x = x
   * if x < 0, mask = 11..111, then mask^x = ~x
  */
  int mask = x >> 31;
  return (mask + x) ^ mask;
}
/* 
 * sm2tc - Convert from sign-magnitude to two's complement
 *   where the MSB is the sign bit
 *   Example: sm2tc(0x80000005) = -5.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 4
 */
int sm2tc(int x) {
  /*
   * ops 6
   * mask: x>>31 = 000000..0000 or 1111...111
   * if x > 0, mask = 00..0000, then ~mask&x = x
   * if x < 0, mask = 11..111, then ~mask&x = 00..000
   * the other two solution
   *return (~mask & x) | ((((x^mask) ^ (1<<31)) + 1) & mask);
   *
   * int sx = x >> 31;     //sx = 11111...1 or 00000...0, depends on the significant bit
   * return ((((1<<31)+(~0))& sx)^x)+(sx & 0x01);
  */
  int mask = x >> 31;
  return (~mask & x) | (((1<<31) + (~x+1)) & mask);

}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  /*
   * n<<3: 8*n
   * 0xff: 8 least significant bit
   */
  return (x >> (n<<3)) & 0xff;
}
/* 
 * byteSwap - swaps the nth byte and the mth byte
 *  Examples: byteSwap(0x12345678, 1, 3) = 0x56341278
 *            byteSwap(0xDEADBEEF, 0, 2) = 0xDEEFBEAD
 *  You may assume that 0 <= n <= 3, 0 <= m <= 3
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 25
 *  Rating: 2
 */
int byteSwap(int x, int n, int m) {
    //111111110000...0000
    int mask = ~(255 << (n << 3)); 
    //get the n byte
    int byteN = (x >> (n<<3)) & 0xff;
    //get the m byte
    int byteM = (x >> (m<<3)) & 0xff;
    //change the n byte and n byte
    int byteCombine = (byteM << (n<<3)) | (byteN << (m<<3));
    //change the n and m byte to 0
    x = mask & x;
    mask = ~(255 << (m << 3)); 
    x = mask & x;
    return x | byteCombine;
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  //create 010101..0101, mask | x should be 1111...1111 if true
  /*
   * the other way, first create 1010101.....10, then ^
   *  int mask =  int mask = (0x66<<8) | 0x66;
   *  mask = mask | (mask<<16);
   *  return !((x | mask) ^ x)
   */

  int mask = (0x55<<8) | 0x55;
  mask = mask | (mask<<16);
  return !(~(mask | x));
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  /*
   *if x = 0 , then mask = 000..0000, else mask = 111...1111
   *
   *the other two solution
   *int t = !x + ~0;
   *int q = !!x + ~0;
   *return (y & t) | (z & q);
   *
   * int untrue_x = !x;    //when x is not true, untrue_x = 1
   * int minus1 = ~0;
    return ((((!untrue_x) + minus1) & z) + ((untrue_x + minus1) & y));
   *
  */
  int mask = ~(!!x)+1;
  return (z&~mask) | (mask&y) ;
}
/* 
 * greatestBitPos - return a mask that marks the position of the
 *               most significant 1 bit. If x == 0, return 0
 *   Example: greatestBitPos(96) = 0x40
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 70
 *   Rating: 4 
 */
int greatestBitPos(int x) {
  /* 
    x < 0, x = 11.. after shift
    x = 0, x = 00.. after shift
    x > 0, x=  0001111..1 after shift
    x & ((~x >> 1) ^ (1 << 31))): 
    x for x = 0, 
    ^(~x >> 1) use for separate condition for negative and positive

    the other way:
    //if x > 0 , g = 000000..0100000..00
    // if x < 0, f = oxffff, g = 0xffff
    //if x = 0, f = 0x0000, g = 0x0000
    int b = x | (x>>1);
    int c = b | (b>>2);
    int d = c | (c>>4);
    int e = d | (d>>8);
    int f = e | (e>>16);
    int g = f ^ (f>>1);
    int mask = ~0;
    return (((!g + mask) & g) | ((!!g + mask) & (1<<31))) & f;
  */
    x = x | x >> 1;
    x = x | x >> 2;
    x = x | x >> 4;
    x = x | x >> 8;
    x = x | x >> 16;
    x = x & ((~x >> 1) ^ (1 << 31));
    return x;
 
 
}
/* 
 * float_abs - Return bit-level equivalent of absolute value of f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument..
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_abs(unsigned uf) {
  unsigned mask = 0x7FFFFFFF; 
  //use to check NaN  
  unsigned nan = 0x7F800001; 
  // set sign bit to 0
  unsigned result = uf & mask;   

  // check if NaN
  if (result >= nan)
    return uf;
  return result;
}
/* 
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
  unsigned s = uf >> 31;
  unsigned exp = (uf >> 23) & 0xFF;
  unsigned frac =(uf & 0x7FFFFF);
  //if normalize, e = exp -bias
  int e = exp -127;
  int result;


  //check if NaN or ifinity
  if(exp == 0x7F800000)
    return 0x80000000u;

  //-1 ~ 1, return 0
   if(!exp || (e < 0))
      return 0;

  //if overflow
  if( e > 31)
    return 0x80000000u;

  //if normalized, append a 1 to the left of the frac
  frac = frac | 0x800000; 
  if (e > 22) 
    result = frac << (e - 23);
  else 
    result = frac >> (23 - e);

  //if negate, change the sign
  if(s) 
    return -result;

  return result;
}
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
  unsigned s = uf & 0x80000000;
  unsigned exp = (uf >> 23) & 0xFF;
  unsigned frac =(uf & 0x7FFFFF);

  // if NaN or 0
  if((exp == 0 && frac == 0) || (exp == 255))
    return uf;

  if (exp) { // if normalized
      exp++;
  } else if (frac == 0x7FFFFF) { // if frac is on the edge value
      frac--;
      exp++;
  } else { // if denormalized
      frac <<= 1;
  }
  return (s) | (exp << 23) | (frac);
}
