#include "parser.h"
#include "headerhash.h"
#define FIND_NEXT_LIKELY(x, y) { \
	    __asm__ ( \
	    "movdqu xmm1, [ %[imm] ]"	"\t\n"\
	    "1:"		"\t\n"\
	    "movdqu xmm2, [%0]"		"\t\n"\
	    "pcmpeqb xmm2, xmm1"		"\t\n"\
	    "ptest xmm2, xmm2"		"\t\n"\
	    "jnz 1f"		"\t\n"\
	    "add %0, 16"		"\t\n"\
	    "jmp 1b"	"\t\n"\
	    "1:"		"\t\n"\
	    "pmovmskb rcx, xmm2"	"\t\n"\
	    "bsf rcx, rcx"	"\t\n"\
	    "lea %0, [ %0 + rcx + 1]"	"\t\n"\
	    :"+r"(y)\
	    :[imm]"r"(x x x x x x x x x x x x x x x x)\
	    :"rcx"  \
	    );\
	}
#define FIND_NEXT_UNLIKELY(x, y) { \
	    __asm__ ( \
	    "movdqu xmm1, [ %[imm] ]"	"\t\n"\
	    "jmp 2f"	"\t\n"\
	    "1:"		"\t\n"\
	    "add %0, 16"		"\t\n"\
	    "2:"		"\t\n"\
	    "movdqu xmm2, [%0]"		"\t\n"\
	    "pcmpeqb xmm2, xmm1"		"\t\n"\
	    "ptest xmm2, xmm2"		"\t\n"\
	    "jz 1b"		"\t\n"\
	    "pmovmskb rcx, xmm2"	"\t\n"\
	    "bsf rcx, rcx"	"\t\n"\
	    "lea %0, [ %0 + rcx + 1]"	"\t\n"\
	    :"+r"(y)\
	    :[imm]"r"(x x x x x x x x x x x x x x x x)\
	    :"rcx"  \
	    );\
	}

//Issue: TOO MANY COMPARISONS IN THE MAIN LOOP!!!
//TODO: Extract more performance out of it, and my hand-written piece of asm is a bit unoptimized
//
//FULL SPEED AHEAD!!! currently I have only optimized for CPUs having SSE instructions
//Arm neon SIMD is to be added asap
int ParseHeader( char* header_, struct http_header* hh)
{
    register char** table = hh->h_val; //Goddamn double pointer! It causes a bug when passed to GNU inline asm. ( Note: never pass double pointer to an inline asm statement or you will be getting weird errors! To clarify why it is fatal, passing it to the input section of an inline asm causes gnu asm to think that it is accessed only once, so it will advance the pointer by some values , which is error prone to loop execution
    register char* header asm("r11")= header_;
    register int64_t dummy asm("r9") = 0;
    register int64_t dummy1 asm("r10") = 0;
    register volatile int64_t ret = 0;//asm("rax");
    register int64_t rcx asm("rcx"); //for use of bsf instruction, if I can somehow make it not dependent on rcx it would be great
    do
    {
	hh->method += *header;
    }while(*header++ != ' ');
    hh->path = ++header;
    FIND_NEXT_LIKELY(" ", header);
    *(header - 1) = 0;
    /*
       register int i = 0;
       for(i = 0; i < len; i++){
       if(*(header + i) == '\n')
       break;
       }
       i++;
       dummy = header + i;
       for( ; i < len; i++){
       if(*(header + i) == '\r')
       {
     *(header + i) = 0;
     table[pos] = dummy;
     dummy = header + i + 2;
     }
     else if (*(header + i) == ':'){
     pos = in_word_set(dummy, header + i - dummy );
     *(header + i) = 0;
     dummy = header + i + 2;
     } 
     }
     */
    __asm__ volatile(
	    "movdqu xmm4, [ %[imm] ]"	"\t\n"
	    "movdqu xmm3, [%[imm1]]"		"\t\n"
	    "HEAD:		    "		"\t\n"
	  //  "movzx ecx, BYTE PTR [%0]"		"\t\n"
	  //  "cmp ecx, 0x0d"		"\t\n"
	  //  "jbe END"		"\t\n"
	    "movdqu xmm1, [ %0 ]"		"\t\n"
	    "movdqa xmm2, xmm4"		"\t\n"
	    "pcmpeqb xmm2, xmm1"		"\t\n"
	    "ptest xmm2, xmm2"		"\t\n"
	    "jnz SKIPADD"		"\t\n"
	    "add %0, 16"		"\t\n"
	    "jmp HEAD"		"\t\n"
	    "SKIPADD:"		"\t\n"
	    "pmovmskb %k[dum1], xmm2"	"\t\n"
	    "pandn xmm2, xmm1"		"\t\n"
	    "movdqu [ %0 ], xmm2"		"\t\n"
	    "bsf %k[dum1], %k[dum1]"		"\t\n"
	    "lea %0, [%0 + %[dum1] + 2]"		"\t\n"
	    "cmp BYTE PTR [%0], 0x0d"		"\t\n"
	    "jbe END"		"\t\n"
	    "xor %k[dum], %k[dum]"		"\t\n"
	    "TOKENIZE:"		"\t\n"
	    "movdqu xmm1, [ %0 + %[dum] ]"		"\t\n"
	    "pcmpeqb xmm1, xmm3"		"\t\n"
	    "ptest xmm1, xmm1"		"\t\n"
	    "jnz FOUND "		"\t\n"
	    "add %[dum], 16"		"\t\n"
	    "jmp TOKENIZE"		"\t\n"
	    "FOUND:"		"\t\n"
	    "pmovmskb %k[dum1], xmm1"	"\t\n"
	    "bsf %k[dum1], %k[dum1]"		"\t\n"
	    "add %[dum],  %[dum1]"		"\t\n"
	    :"+r"(header),[dum1]"+r"(dummy1), [dum]"=r"(dummy)
	    : "0"(header),[imm]"m"("\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r"), [imm1]"m"("::::::::::::::::")
		   );
    ret = hdr_ind(header, dummy);
    __asm__  (
	    "lea %[header], [%[header] + %[dum] + 2]"		"\t\n"
	    "test %[ret], -1"		"\t\n"
	    "js HEAD"		"\t\n"
	    "mov [%[TABLE] + %[ret]*8], %[header]"		"\t\n"
	    "jmp HEAD"		"\t\n"
	    "END:"		"\t\n"
	    :[header]"+r"(header), [ret]"+r"(ret), [dum]"=r"(dummy)
	    :"0"(header),[TABLE]"g"(table) 
	    ); 
   //
    //    if(len <= 0) return 401;
    //    int Val = 1;
    //    for(int c = 0, i = 0, f = 0, l = 0;;)
    //    {
    //	if((*header == '\r' && *(header + 1) == '\n'))
    //	{
    //	    c++;
    //	    Val = 1;
    //	    f = 0;
    //	    i = 0;
    //	    header++;
    //	    l++;
    //	    goto SKIP;
    //	}
    //	if(*header == ' ')
    //	{
    //	    f += Val;
    //	    i = !Val;
    //	    goto SKIP;
    //	}
    //	if(*header == ':' && *(header + 1) == ' ')
    //	{
    //	    f++;
    //	    i = 0;
    //	    header++;
    //	    Val = 0;
    //	    l++;
    //	    goto SKIP;
    //	}
    //	if(c == 0)
    //	{
    //	    switch(f)
    //	    {
    //		case 0:
    //		    hh->method += *header;
    //		    break;
    //		case 1:
    //		    if(i == 0 && *header == '/') goto SKIP;
    //		    hh->path[i*(i <= MAX_PATH)] = *header;
    //		    i++;
    //		    break;
    //		case 2:
    //		    //assuming that the version number starts at 6th character
    //		    //if(*header == '/') { b = 1; goto SKIP;}
    //		    //if(b == 0) goto SKIP;
    //		    //hh->protocol_ver[i*(i < MAX_PROTOCOL_LEN)] = *header;
    //		    //i++;
    //		    hh->protocol_ver = *(header + 5)*10 + *(header + 7) - 528;
    //		    c++;
    //		    Val = 1;
    //		    f = 0;
    //		    i = 0;
    //		    header++;
    //		    l++;
    //		    goto SKIP;
    //	    }
    //	}
    //	else
    //	{
    //	    switch(f)
    //	    {
    //		case 0:
    //		    hh->h_names[c - 1][i *(i<= MAX_ENTRIES_VAL)]  = *header;
    //		    i++;
    //		    break;
    //		case 1:
    //		    hh->h_val[c - 1][i * (i <= MAX_ENTRIES_VAL)] = *header;
    //		    i++;
    //		    break;
    //	    }
    //	}
    //SKIP:
    //	header++;
    //	l++;
    //	if(l >= len)
    //	{
    //
    //	    return 0;
    //	}
    //    }
}

char* base64_encode(char* str_, size_t len)
{
    const char magicval[16] __attribute__((aligned(16)))={
        0x01,0x00,0x02,0x01,
        0x04,0x03,0x05,0x04,
        0x07,0x06,0x08,0x07,
        0x0a,0x09,0x0b,0x0a,
    } ; 
    
    const int8_t lookup[16] __attribute__((aligned(16)))={
	71, -4, -4, -4,
	-4, -4, -4, -4,
	-4, -4, -4, '+'- 62,
	'\\' - 63, 65, 0, 0,
    } ; 

    __uint128_t const1 = *(__uint128_t*)"\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33\x33";
    __uint128_t const2 = *(__uint128_t*)"\x19\x19\x19\x19\x19\x19\x19\x19\x19\x19\x19\x19\x19\x19\x19\x19";
    __uint128_t const3 = *(__uint128_t*)"\x0D\x0D\x0D\x0D\x0D\x0D\x0D\x0D\x0D\x0D\x0D\x0D\x0D\x0D\x0D\x0D";

    register int32_t dummy= 0xfc0fc00;
    register int32_t dummy1= 0x4000040;
    register int64_t len = strlen(str_);
    register char* str = str_ + len;

    const int sz = (len/3)*4;
    char* encoded = aligned_alloc(16, sz);
    __asm__(
	    "neg %[size]" "\n"
	    "movd xmm0, %[dum]" "\n"
	    "pshufd xmm3, xmm0, 0" "\n"
	    "movdqa xmm4, xmm3" "\n"
	    "psrld xmm4, 6" "\n"
	    "movd xmm0, %[dum1]" "\n"
	    "pshufd xmm5, xmm0, 0" "\n"
	    "movdqa xmm6, xmm5" "\n"
	    "psrld xmm6, 2" "\n"
	    "movdqa xmm7, [%[lookup]]" "\n"
	    "1:" "\n"
	    "movdqu xmm0, [%[str] + %[size]]" "\n"
	    "pshufb xmm0, [%[magicval]]" "\n"
	    "movdqa xmm1, xmm0" "\n"
	    "pand xmm0, xmm3" "\n"
	    "pand xmm1, xmm4" "\n"
	    "pmulhw xmm0, xmm5" "\n"
	    "pmullw xmm1, xmm6" "\n"
	    "por xmm0, xmm1" "\n"
	    "movdqa xmm1, xmm0" "\n"
	    "movdqa xmm2, xmm0" "\n"
	    "psubusb xmm1, [%[c1]]" "\n"
	    "pcmpgtb xmm2, [%[c2]]" "\n"
	    "pandn xmm2, [%[c3]]" "\n"
	    "por xmm1, xmm2" "\n"
	    "movdqa xmm2, xmm7" "\n"
	    "pshufb xmm2, xmm1" "\n"
	    "paddb xmm0, xmm2" "\n"
	    "movdqa [%[encoded]], xmm0" "\n"
	    "add %[encoded], 0x10" "\n"
	    "add %[size], 12" "\n"
	    "js 1b" "\n"
	    :[size]"+r"(size)
	    :[magicval]"m"(magicval), [str]"r"(str), [dum]"r"(dummy), [dum1]"r"(dummy1), [encoded]"r"(encoded), [lookup]"r"(lookup), [c1]"m"(const1), [c2]"m"(const2), [c3]"m"(const3)
	      );
    return encoded;
}
