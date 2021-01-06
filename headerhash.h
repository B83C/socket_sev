/* ANSI-C code produced by custom gperf (and it's badly patched). See https://github.com/B83C/gperf for the entire build.*/
/* Command-line: ./gperf -S 1 -R i --output-file=../headerhash.h -N hdr_ind -I -J -f header_fields  */
#ifndef GPERF_PERF_HASH
#define GPERF_PERF_HASH
/* Computed positions: -k'1,8' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#include <stddef.h>

#define TOTAL_KEYWORDS 35
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 30
#define MIN_HASH_VALUE 2
#define MAX_HASH_VALUE 55
/* maximum key range = 54, duplicates = 0 */

#define TE 0
#define Via 1
#define Host 2
#define Range 3
#define Cookie 4
#define Referer 5
#define If_Range 6
#define A_IM 7
#define Connection 8
#define Accept 9
#define Content_Type 10
#define If_None_Match 11
#define Content_Length 12
#define Expect 13
#define Upgrade 14
#define Authorization 15
#define Accept_Charset 16
#define User_Agent 17
#define Pragma 18
#define If_Modified_Since 19
#define Cache_Control 20
#define From 21
#define Accept_Language 22
#define Origin 23
#define Warning 24
#define If_Match 25
#define Forwarded 26
#define Accept_Encoding 27
#define Max_Forwards 28
#define Access_Control_Request_Method 29
#define Access_Control_Request_Headers 30
#define Date 31
#define Proxy_Authorization 32
#define If_Unmodified_Since 33
#define Accept_Datetime 34

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (register const char *str, register size_t len)
{
  static unsigned char asso_values[] =
    {
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56,  0, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56,  5, 56,  0, 35, 10,
      20, 56,  0,  0, 56, 56,  5, 10, 56, 20,
      15, 56,  0, 56,  0, 10,  0, 20, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      30,  0,  5, 56, 20,  0, 56, 56, 56, 56,
      56, 10, 56, 56, 56, 56, 56, 10, 56, 10,
      56, 56,  0, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
      56, 56, 56, 56, 56, 56
    };
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[7]];
#if defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang_major__ && defined __clang_minor__ && __clang_major__ + (__clang_minor__ >= 9) > 3))
      [[fallthrough]];
#elif defined __GNUC__ && __GNUC__ >= 7
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 7:
      case 6:
      case 5:
      case 4:
      case 3:
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

static inline const int
hdr_ind (register const char *str, register size_t len)
{
    register unsigned int key = hash (str, len);

          switch (key - 2)
            {
              case 0:
                return 0;
              case 1:
                return 1;
              case 2:
                return 2;
              case 3:
                return 3;
              case 4:
                return 4;
              case 5:
                return 5;
              case 6:
                return 6;
              case 7:
                return 7;
              case 8:
                return 8;
              case 9:
                return 9;
              case 10:
                return 10;
              case 11:
                return 11;
              case 12:
                return 12;
              case 14:
                return 13;
              case 15:
                return 14;
              case 16:
                return 15;
              case 17:
                return 16;
              case 18:
                return 17;
              case 19:
                return 18;
              case 20:
                return 19;
              case 21:
                return 20;
              case 22:
                return 21;
              case 23:
                return 22;
              case 24:
                return 23;
              case 25:
                return 24;
              case 26:
                return 25;
              case 27:
                return 26;
              case 28:
                return 27;
              case 30:
                return 28;
              case 32:
                return 29;
              case 33:
                return 30;
              case 37:
                return 31;
              case 42:
                return 32;
              case 47:
                return 33;
              case 53:
                return 34;
            }
          return -1;
}
#endif