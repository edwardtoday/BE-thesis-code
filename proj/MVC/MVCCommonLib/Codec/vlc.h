/************************************************************************
 *
 * MVC Codec
 * 
 * History
 * 2008-11-29		Weidong Hu		Tsinghua University		Reorganize
 * 2006-2008 		Weidong Hu		Tsinghua University		Modified from T264
 * 
 ************************************************************************/

#ifndef assert
#define assert ASSERT
#endif // assert

typedef struct
{
    int i_bits;
    int i_size;
} vlc_t;

/* XXX: don't forget to change it if you change vlc_t */
#define MKVLC( a, b ) { a, b }
static const vlc_t x264_coeff_token[5][17*4] =
{
    /* table 0 */
    {
        MKVLC( 0x1, 1 ), /* str=1 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x5, 6 ), /* str=000101 */
        MKVLC( 0x1, 2 ), /* str=01 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x7, 8 ), /* str=00000111 */
        MKVLC( 0x4, 6 ), /* str=000100 */
        MKVLC( 0x1, 3 ), /* str=001 */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x7, 9 ), /* str=000000111 */
        MKVLC( 0x6, 8 ), /* str=00000110 */
        MKVLC( 0x5, 7 ), /* str=0000101 */
        MKVLC( 0x3, 5 ), /* str=00011 */

        MKVLC( 0x7, 10 ), /* str=0000000111 */
        MKVLC( 0x6, 9 ), /* str=000000110 */
        MKVLC( 0x5, 8 ), /* str=00000101 */
        MKVLC( 0x3, 6 ), /* str=000011 */

        MKVLC( 0x7, 11 ), /* str=00000000111 */
        MKVLC( 0x6, 10 ), /* str=0000000110 */
        MKVLC( 0x5, 9 ), /* str=000000101 */
        MKVLC( 0x4, 7 ), /* str=0000100 */

        MKVLC( 0xf, 13 ), /* str=0000000001111 */
        MKVLC( 0x6, 11 ), /* str=00000000110 */
        MKVLC( 0x5, 10 ), /* str=0000000101 */
        MKVLC( 0x4, 8 ), /* str=00000100 */

        MKVLC( 0xb, 13 ), /* str=0000000001011 */
        MKVLC( 0xe, 13 ), /* str=0000000001110 */
        MKVLC( 0x5, 11 ), /* str=00000000101 */
        MKVLC( 0x4, 9 ), /* str=000000100 */

        MKVLC( 0x8, 13 ), /* str=0000000001000 */
        MKVLC( 0xa, 13 ), /* str=0000000001010 */
        MKVLC( 0xd, 13 ), /* str=0000000001101 */
        MKVLC( 0x4, 10 ), /* str=0000000100 */

        MKVLC( 0xf, 14 ), /* str=00000000001111 */
        MKVLC( 0xe, 14 ), /* str=00000000001110 */
        MKVLC( 0x9, 13 ), /* str=0000000001001 */
        MKVLC( 0x4, 11 ), /* str=00000000100 */

        MKVLC( 0xb, 14 ), /* str=00000000001011 */
        MKVLC( 0xa, 14 ), /* str=00000000001010 */
        MKVLC( 0xd, 14 ), /* str=00000000001101 */
        MKVLC( 0xc, 13 ), /* str=0000000001100 */

        MKVLC( 0xf, 15 ), /* str=000000000001111 */
        MKVLC( 0xe, 15 ), /* str=000000000001110 */
        MKVLC( 0x9, 14 ), /* str=00000000001001 */
        MKVLC( 0xc, 14 ), /* str=00000000001100 */

        MKVLC( 0xb, 15 ), /* str=000000000001011 */
        MKVLC( 0xa, 15 ), /* str=000000000001010 */
        MKVLC( 0xd, 15 ), /* str=000000000001101 */
        MKVLC( 0x8, 14 ), /* str=00000000001000 */

        MKVLC( 0xf, 16 ), /* str=0000000000001111 */
        MKVLC( 0x1, 15 ), /* str=000000000000001 */
        MKVLC( 0x9, 15 ), /* str=000000000001001 */
        MKVLC( 0xc, 15 ), /* str=000000000001100 */

        MKVLC( 0xb, 16 ), /* str=0000000000001011 */
        MKVLC( 0xe, 16 ), /* str=0000000000001110 */
        MKVLC( 0xd, 16 ), /* str=0000000000001101 */
        MKVLC( 0x8, 15 ), /* str=000000000001000 */

        MKVLC( 0x7, 16 ), /* str=0000000000000111 */
        MKVLC( 0xa, 16 ), /* str=0000000000001010 */
        MKVLC( 0x9, 16 ), /* str=0000000000001001 */
        MKVLC( 0xc, 16 ), /* str=0000000000001100 */

        MKVLC( 0x4, 16 ), /* str=0000000000000100 */
        MKVLC( 0x6, 16 ), /* str=0000000000000110 */
        MKVLC( 0x5, 16 ), /* str=0000000000000101 */
        MKVLC( 0x8, 16 ), /* str=0000000000001000 */
    },

    /* table 1 */
    {
        MKVLC( 0x3, 2 ), /* str=11 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0xb, 6 ), /* str=001011 */
        MKVLC( 0x2, 2 ), /* str=10 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x7, 6 ), /* str=000111 */
        MKVLC( 0x7, 5 ), /* str=00111 */
        MKVLC( 0x3, 3 ), /* str=011 */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x7, 7 ), /* str=0000111 */
        MKVLC( 0xa, 6 ), /* str=001010 */
        MKVLC( 0x9, 6 ), /* str=001001 */
        MKVLC( 0x5, 4 ), /* str=0101 */

        MKVLC( 0x7, 8 ), /* str=00000111 */
        MKVLC( 0x6, 6 ), /* str=000110 */
        MKVLC( 0x5, 6 ), /* str=000101 */
        MKVLC( 0x4, 4 ), /* str=0100 */

        MKVLC( 0x4, 8 ), /* str=00000100 */
        MKVLC( 0x6, 7 ), /* str=0000110 */
        MKVLC( 0x5, 7 ), /* str=0000101 */
        MKVLC( 0x6, 5 ), /* str=00110 */

        MKVLC( 0x7, 9 ), /* str=000000111 */
        MKVLC( 0x6, 8 ), /* str=00000110 */
        MKVLC( 0x5, 8 ), /* str=00000101 */
        MKVLC( 0x8, 6 ), /* str=001000 */

        MKVLC( 0xf, 11 ), /* str=00000001111 */
        MKVLC( 0x6, 9 ), /* str=000000110 */
        MKVLC( 0x5, 9 ), /* str=000000101 */
        MKVLC( 0x4, 6 ), /* str=000100 */

        MKVLC( 0xb, 11 ), /* str=00000001011 */
        MKVLC( 0xe, 11 ), /* str=00000001110 */
        MKVLC( 0xd, 11 ), /* str=00000001101 */
        MKVLC( 0x4, 7 ), /* str=0000100 */

        MKVLC( 0xf, 12 ), /* str=000000001111 */
        MKVLC( 0xa, 11 ), /* str=00000001010 */
        MKVLC( 0x9, 11 ), /* str=00000001001 */
        MKVLC( 0x4, 9 ), /* str=000000100 */

        MKVLC( 0xb, 12 ), /* str=000000001011 */
        MKVLC( 0xe, 12 ), /* str=000000001110 */
        MKVLC( 0xd, 12 ), /* str=000000001101 */
        MKVLC( 0xc, 11 ), /* str=00000001100 */

        MKVLC( 0x8, 12 ), /* str=000000001000 */
        MKVLC( 0xa, 12 ), /* str=000000001010 */
        MKVLC( 0x9, 12 ), /* str=000000001001 */
        MKVLC( 0x8, 11 ), /* str=00000001000 */

        MKVLC( 0xf, 13 ), /* str=0000000001111 */
        MKVLC( 0xe, 13 ), /* str=0000000001110 */
        MKVLC( 0xd, 13 ), /* str=0000000001101 */
        MKVLC( 0xc, 12 ), /* str=000000001100 */

        MKVLC( 0xb, 13 ), /* str=0000000001011 */
        MKVLC( 0xa, 13 ), /* str=0000000001010 */
        MKVLC( 0x9, 13 ), /* str=0000000001001 */
        MKVLC( 0xc, 13 ), /* str=0000000001100 */

        MKVLC( 0x7, 13 ), /* str=0000000000111 */
        MKVLC( 0xb, 14 ), /* str=00000000001011 */
        MKVLC( 0x6, 13 ), /* str=0000000000110 */
        MKVLC( 0x8, 13 ), /* str=0000000001000 */

        MKVLC( 0x9, 14 ), /* str=00000000001001 */
        MKVLC( 0x8, 14 ), /* str=00000000001000 */
        MKVLC( 0xa, 14 ), /* str=00000000001010 */
        MKVLC( 0x1, 13 ), /* str=0000000000001 */

        MKVLC( 0x7, 14 ), /* str=00000000000111 */
        MKVLC( 0x6, 14 ), /* str=00000000000110 */
        MKVLC( 0x5, 14 ), /* str=00000000000101 */
        MKVLC( 0x4, 14 ), /* str=00000000000100 */
    },
    /* table 2 */
    {
        MKVLC( 0xf, 4 ), /* str=1111 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0xf, 6 ), /* str=001111 */
        MKVLC( 0xe, 4 ), /* str=1110 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0xb, 6 ), /* str=001011 */
        MKVLC( 0xf, 5 ), /* str=01111 */
        MKVLC( 0xd, 4 ), /* str=1101 */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x8, 6 ), /* str=001000 */
        MKVLC( 0xc, 5 ), /* str=01100 */
        MKVLC( 0xe, 5 ), /* str=01110 */
        MKVLC( 0xc, 4 ), /* str=1100 */

        MKVLC( 0xf, 7 ), /* str=0001111 */
        MKVLC( 0xa, 5 ), /* str=01010 */
        MKVLC( 0xb, 5 ), /* str=01011 */
        MKVLC( 0xb, 4 ), /* str=1011 */

        MKVLC( 0xb, 7 ), /* str=0001011 */
        MKVLC( 0x8, 5 ), /* str=01000 */
        MKVLC( 0x9, 5 ), /* str=01001 */
        MKVLC( 0xa, 4 ), /* str=1010 */

        MKVLC( 0x9, 7 ), /* str=0001001 */
        MKVLC( 0xe, 6 ), /* str=001110 */
        MKVLC( 0xd, 6 ), /* str=001101 */
        MKVLC( 0x9, 4 ), /* str=1001 */

        MKVLC( 0x8, 7 ), /* str=0001000 */
        MKVLC( 0xa, 6 ), /* str=001010 */
        MKVLC( 0x9, 6 ), /* str=001001 */
        MKVLC( 0x8, 4 ), /* str=1000 */

        MKVLC( 0xf, 8 ), /* str=00001111 */
        MKVLC( 0xe, 7 ), /* str=0001110 */
        MKVLC( 0xd, 7 ), /* str=0001101 */
        MKVLC( 0xd, 5 ), /* str=01101 */

        MKVLC( 0xb, 8 ), /* str=00001011 */
        MKVLC( 0xe, 8 ), /* str=00001110 */
        MKVLC( 0xa, 7 ), /* str=0001010 */
        MKVLC( 0xc, 6 ), /* str=001100 */

        MKVLC( 0xf, 9 ), /* str=000001111 */
        MKVLC( 0xa, 8 ), /* str=00001010 */
        MKVLC( 0xd, 8 ), /* str=00001101 */
        MKVLC( 0xc, 7 ), /* str=0001100 */

        MKVLC( 0xb, 9 ), /* str=000001011 */
        MKVLC( 0xe, 9 ), /* str=000001110 */
        MKVLC( 0x9, 8 ), /* str=00001001 */
        MKVLC( 0xc, 8 ), /* str=00001100 */

        MKVLC( 0x8, 9 ), /* str=000001000 */
        MKVLC( 0xa, 9 ), /* str=000001010 */
        MKVLC( 0xd, 9 ), /* str=000001101 */
        MKVLC( 0x8, 8 ), /* str=00001000 */

        MKVLC( 0xd, 10 ), /* str=0000001101 */
        MKVLC( 0x7, 9 ), /* str=000000111 */
        MKVLC( 0x9, 9 ), /* str=000001001 */
        MKVLC( 0xc, 9 ), /* str=000001100 */

        MKVLC( 0x9, 10 ), /* str=0000001001 */
        MKVLC( 0xc, 10 ), /* str=0000001100 */
        MKVLC( 0xb, 10 ), /* str=0000001011 */
        MKVLC( 0xa, 10 ), /* str=0000001010 */

        MKVLC( 0x5, 10 ), /* str=0000000101 */
        MKVLC( 0x8, 10 ), /* str=0000001000 */
        MKVLC( 0x7, 10 ), /* str=0000000111 */
        MKVLC( 0x6, 10 ), /* str=0000000110 */

        MKVLC( 0x1, 10 ), /* str=0000000001 */
        MKVLC( 0x4, 10 ), /* str=0000000100 */
        MKVLC( 0x3, 10 ), /* str=0000000011 */
        MKVLC( 0x2, 10 ), /* str=0000000010 */
    },

    /* table 3 */
    {
        MKVLC( 0x3, 6 ), /* str=000011 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x0, 6 ), /* str=000000 */
        MKVLC( 0x1, 6 ), /* str=000001 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x4, 6 ), /* str=000100 */
        MKVLC( 0x5, 6 ), /* str=000101 */
        MKVLC( 0x6, 6 ), /* str=000110 */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x8, 6 ), /* str=001000 */
        MKVLC( 0x9, 6 ), /* str=001001 */
        MKVLC( 0xa, 6 ), /* str=001010 */
        MKVLC( 0xb, 6 ), /* str=001011 */

        MKVLC( 0xc, 6 ), /* str=001100 */
        MKVLC( 0xd, 6 ), /* str=001101 */
        MKVLC( 0xe, 6 ), /* str=001110 */
        MKVLC( 0xf, 6 ), /* str=001111 */

        MKVLC( 0x10, 6 ), /* str=010000 */
        MKVLC( 0x11, 6 ), /* str=010001 */
        MKVLC( 0x12, 6 ), /* str=010010 */
        MKVLC( 0x13, 6 ), /* str=010011 */

        MKVLC( 0x14, 6 ), /* str=010100 */
        MKVLC( 0x15, 6 ), /* str=010101 */
        MKVLC( 0x16, 6 ), /* str=010110 */
        MKVLC( 0x17, 6 ), /* str=010111 */

        MKVLC( 0x18, 6 ), /* str=011000 */
        MKVLC( 0x19, 6 ), /* str=011001 */
        MKVLC( 0x1a, 6 ), /* str=011010 */
        MKVLC( 0x1b, 6 ), /* str=011011 */

        MKVLC( 0x1c, 6 ), /* str=011100 */
        MKVLC( 0x1d, 6 ), /* str=011101 */
        MKVLC( 0x1e, 6 ), /* str=011110 */
        MKVLC( 0x1f, 6 ), /* str=011111 */

        MKVLC( 0x20, 6 ), /* str=100000 */
        MKVLC( 0x21, 6 ), /* str=100001 */
        MKVLC( 0x22, 6 ), /* str=100010 */
        MKVLC( 0x23, 6 ), /* str=100011 */

        MKVLC( 0x24, 6 ), /* str=100100 */
        MKVLC( 0x25, 6 ), /* str=100101 */
        MKVLC( 0x26, 6 ), /* str=100110 */
        MKVLC( 0x27, 6 ), /* str=100111 */

        MKVLC( 0x28, 6 ), /* str=101000 */
        MKVLC( 0x29, 6 ), /* str=101001 */
        MKVLC( 0x2a, 6 ), /* str=101010 */
        MKVLC( 0x2b, 6 ), /* str=101011 */

        MKVLC( 0x2c, 6 ), /* str=101100 */
        MKVLC( 0x2d, 6 ), /* str=101101 */
        MKVLC( 0x2e, 6 ), /* str=101110 */
        MKVLC( 0x2f, 6 ), /* str=101111 */

        MKVLC( 0x30, 6 ), /* str=110000 */
        MKVLC( 0x31, 6 ), /* str=110001 */
        MKVLC( 0x32, 6 ), /* str=110010 */
        MKVLC( 0x33, 6 ), /* str=110011 */

        MKVLC( 0x34, 6 ), /* str=110100 */
        MKVLC( 0x35, 6 ), /* str=110101 */
        MKVLC( 0x36, 6 ), /* str=110110 */
        MKVLC( 0x37, 6 ), /* str=110111 */

        MKVLC( 0x38, 6 ), /* str=111000 */
        MKVLC( 0x39, 6 ), /* str=111001 */
        MKVLC( 0x3a, 6 ), /* str=111010 */
        MKVLC( 0x3b, 6 ), /* str=111011 */

        MKVLC( 0x3c, 6 ), /* str=111100 */
        MKVLC( 0x3d, 6 ), /* str=111101 */
        MKVLC( 0x3e, 6 ), /* str=111110 */
        MKVLC( 0x3f, 6 ), /* str=111111 */
    },

    /* table 4 */
    {
        MKVLC( 0x1, 2 ), /* str=01 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x7, 6 ), /* str=000111 */
        MKVLC( 0x1, 1 ), /* str=1 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x4, 6 ), /* str=000100 */
        MKVLC( 0x6, 6 ), /* str=000110 */
        MKVLC( 0x1, 3 ), /* str=001 */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x3, 6 ), /* str=000011 */
        MKVLC( 0x3, 7 ), /* str=0000011 */
        MKVLC( 0x2, 7 ), /* str=0000010 */
        MKVLC( 0x5, 6 ), /* str=000101 */

        MKVLC( 0x2, 6 ), /* str=000010 */
        MKVLC( 0x3, 8 ), /* str=00000011 */
        MKVLC( 0x2, 8 ), /* str=00000010 */
        MKVLC( 0x0, 7 ), /* str=0000000 */

        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */

        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    }
};

static const vlc_t x264_level_prefix[16] =
{
    MKVLC( 0x01,  1 ),
    MKVLC( 0x01,  2 ),
    MKVLC( 0x01,  3 ),
    MKVLC( 0x01,  4 ),
    MKVLC( 0x01,  5 ),
    MKVLC( 0x01,  6 ),
    MKVLC( 0x01,  7 ),
    MKVLC( 0x01,  8 ),
    MKVLC( 0x01,  9 ),
    MKVLC( 0x01, 10 ),
    MKVLC( 0x01, 11 ),
    MKVLC( 0x01, 12 ),
    MKVLC( 0x01, 13 ),
    MKVLC( 0x01, 14 ),
    MKVLC( 0x01, 15 ),
    MKVLC( 0x01, 16 )
};

/* [i_total_coeff-1][i_total_zeros] */
static const vlc_t x264_total_zeros[15][16] =
{
    { /* i_total 1 */
        MKVLC( 0x1, 1 ), /* str=1 */
        MKVLC( 0x3, 3 ), /* str=011 */
        MKVLC( 0x2, 3 ), /* str=010 */
        MKVLC( 0x3, 4 ), /* str=0011 */
        MKVLC( 0x2, 4 ), /* str=0010 */
        MKVLC( 0x3, 5 ), /* str=00011 */
        MKVLC( 0x2, 5 ), /* str=00010 */
        MKVLC( 0x3, 6 ), /* str=000011 */
        MKVLC( 0x2, 6 ), /* str=000010 */
        MKVLC( 0x3, 7 ), /* str=0000011 */
        MKVLC( 0x2, 7 ), /* str=0000010 */
        MKVLC( 0x3, 8 ), /* str=00000011 */
        MKVLC( 0x2, 8 ), /* str=00000010 */
        MKVLC( 0x3, 9 ), /* str=000000011 */
        MKVLC( 0x2, 9 ), /* str=000000010 */
        MKVLC( 0x1, 9 ), /* str=000000001 */
    },
    { /* i_total 2 */
        MKVLC( 0x7, 3 ), /* str=111 */
        MKVLC( 0x6, 3 ), /* str=110 */
        MKVLC( 0x5, 3 ), /* str=101 */
        MKVLC( 0x4, 3 ), /* str=100 */
        MKVLC( 0x3, 3 ), /* str=011 */
        MKVLC( 0x5, 4 ), /* str=0101 */
        MKVLC( 0x4, 4 ), /* str=0100 */
        MKVLC( 0x3, 4 ), /* str=0011 */
        MKVLC( 0x2, 4 ), /* str=0010 */
        MKVLC( 0x3, 5 ), /* str=00011 */
        MKVLC( 0x2, 5 ), /* str=00010 */
        MKVLC( 0x3, 6 ), /* str=000011 */
        MKVLC( 0x2, 6 ), /* str=000010 */
        MKVLC( 0x1, 6 ), /* str=000001 */
        MKVLC( 0x0, 6 ), /* str=000000 */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_total 3 */
        MKVLC( 0x5, 4 ), /* str=0101 */
        MKVLC( 0x7, 3 ), /* str=111 */
        MKVLC( 0x6, 3 ), /* str=110 */
        MKVLC( 0x5, 3 ), /* str=101 */
        MKVLC( 0x4, 4 ), /* str=0100 */
        MKVLC( 0x3, 4 ), /* str=0011 */
        MKVLC( 0x4, 3 ), /* str=100 */
        MKVLC( 0x3, 3 ), /* str=011 */
        MKVLC( 0x2, 4 ), /* str=0010 */
        MKVLC( 0x3, 5 ), /* str=00011 */
        MKVLC( 0x2, 5 ), /* str=00010 */
        MKVLC( 0x1, 6 ), /* str=000001 */
        MKVLC( 0x1, 5 ), /* str=00001 */
        MKVLC( 0x0, 6 ), /* str=000000 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_total 4 */
        MKVLC( 0x3, 5 ), /* str=00011 */
        MKVLC( 0x7, 3 ), /* str=111 */
        MKVLC( 0x5, 4 ), /* str=0101 */
        MKVLC( 0x4, 4 ), /* str=0100 */
        MKVLC( 0x6, 3 ), /* str=110 */
        MKVLC( 0x5, 3 ), /* str=101 */
        MKVLC( 0x4, 3 ), /* str=100 */
        MKVLC( 0x3, 4 ), /* str=0011 */
        MKVLC( 0x3, 3 ), /* str=011 */
        MKVLC( 0x2, 4 ), /* str=0010 */
        MKVLC( 0x2, 5 ), /* str=00010 */
        MKVLC( 0x1, 5 ), /* str=00001 */
        MKVLC( 0x0, 5 ), /* str=00000 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_total 5 */
        MKVLC( 0x5, 4 ), /* str=0101 */
        MKVLC( 0x4, 4 ), /* str=0100 */
        MKVLC( 0x3, 4 ), /* str=0011 */
        MKVLC( 0x7, 3 ), /* str=111 */
        MKVLC( 0x6, 3 ), /* str=110 */
        MKVLC( 0x5, 3 ), /* str=101 */
        MKVLC( 0x4, 3 ), /* str=100 */
        MKVLC( 0x3, 3 ), /* str=011 */
        MKVLC( 0x2, 4 ), /* str=0010 */
        MKVLC( 0x1, 5 ), /* str=00001 */
        MKVLC( 0x1, 4 ), /* str=0001 */
        MKVLC( 0x0, 5 ), /* str=00000 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_total 6 */
        MKVLC( 0x1, 6 ), /* str=000001 */
        MKVLC( 0x1, 5 ), /* str=00001 */
        MKVLC( 0x7, 3 ), /* str=111 */
        MKVLC( 0x6, 3 ), /* str=110 */
        MKVLC( 0x5, 3 ), /* str=101 */
        MKVLC( 0x4, 3 ), /* str=100 */
        MKVLC( 0x3, 3 ), /* str=011 */
        MKVLC( 0x2, 3 ), /* str=010 */
        MKVLC( 0x1, 4 ), /* str=0001 */
        MKVLC( 0x1, 3 ), /* str=001 */
        MKVLC( 0x0, 6 ), /* str=000000 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_total 7 */
        MKVLC( 0x1, 6 ), /* str=000001 */
        MKVLC( 0x1, 5 ), /* str=00001 */
        MKVLC( 0x5, 3 ), /* str=101 */
        MKVLC( 0x4, 3 ), /* str=100 */
        MKVLC( 0x3, 3 ), /* str=011 */
        MKVLC( 0x3, 2 ), /* str=11 */
        MKVLC( 0x2, 3 ), /* str=010 */
        MKVLC( 0x1, 4 ), /* str=0001 */
        MKVLC( 0x1, 3 ), /* str=001 */
        MKVLC( 0x0, 6 ), /* str=000000 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_total 8 */
        MKVLC( 0x1, 6 ), /* str=000001 */
        MKVLC( 0x1, 4 ), /* str=0001 */
        MKVLC( 0x1, 5 ), /* str=00001 */
        MKVLC( 0x3, 3 ), /* str=011 */
        MKVLC( 0x3, 2 ), /* str=11 */
        MKVLC( 0x2, 2 ), /* str=10 */
        MKVLC( 0x2, 3 ), /* str=010 */
        MKVLC( 0x1, 3 ), /* str=001 */
        MKVLC( 0x0, 6 ), /* str=000000 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_total 9 */
        MKVLC( 0x1, 6 ), /* str=000001 */
        MKVLC( 0x0, 6 ), /* str=000000 */
        MKVLC( 0x1, 4 ), /* str=0001 */
        MKVLC( 0x3, 2 ), /* str=11 */
        MKVLC( 0x2, 2 ), /* str=10 */
        MKVLC( 0x1, 3 ), /* str=001 */
        MKVLC( 0x1, 2 ), /* str=01 */
        MKVLC( 0x1, 5 ), /* str=00001 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_total 10 */
        MKVLC( 0x1, 5 ), /* str=00001 */
        MKVLC( 0x0, 5 ), /* str=00000 */
        MKVLC( 0x1, 3 ), /* str=001 */
        MKVLC( 0x3, 2 ), /* str=11 */
        MKVLC( 0x2, 2 ), /* str=10 */
        MKVLC( 0x1, 2 ), /* str=01 */
        MKVLC( 0x1, 4 ), /* str=0001 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_total 11 */
        MKVLC( 0x0, 4 ), /* str=0000 */
        MKVLC( 0x1, 4 ), /* str=0001 */
        MKVLC( 0x1, 3 ), /* str=001 */
        MKVLC( 0x2, 3 ), /* str=010 */
        MKVLC( 0x1, 1 ), /* str=1 */
        MKVLC( 0x3, 3 ), /* str=011 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_total 12 */
        MKVLC( 0x0, 4 ), /* str=0000 */
        MKVLC( 0x1, 4 ), /* str=0001 */
        MKVLC( 0x1, 2 ), /* str=01 */
        MKVLC( 0x1, 1 ), /* str=1 */
        MKVLC( 0x1, 3 ), /* str=001 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_total 13 */
        MKVLC( 0x0, 3 ), /* str=000 */
        MKVLC( 0x1, 3 ), /* str=001 */
        MKVLC( 0x1, 1 ), /* str=1 */
        MKVLC( 0x1, 2 ), /* str=01 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_total 14 */
        MKVLC( 0x0, 2 ), /* str=00 */
        MKVLC( 0x1, 2 ), /* str=01 */
        MKVLC( 0x1, 1 ), /* str=1 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_total 15 */
        MKVLC( 0x0, 1 ), /* str=0 */
        MKVLC( 0x1, 1 ), /* str=1 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
};

/* [i_total_coeff-1][i_total_zeros] */
static const vlc_t x264_total_zeros_dc[3][4] =
{
    {
        MKVLC( 0x01, 1 ), /* 1  */
        MKVLC( 0x01, 2 ), /* 01 */
        MKVLC( 0x01, 3 ), /* 001*/
        MKVLC( 0x00, 3 )  /* 000*/
    },
    {
        MKVLC( 0x01, 1 ), /* 1  */
        MKVLC( 0x01, 2 ), /* 01 */
        MKVLC( 0x00, 2 ), /* 00 */
        MKVLC( 0x00, 0 )  /*    */
    },
    {
        MKVLC( 0x01, 1 ), /* 1  */
        MKVLC( 0x00, 1 ), /* 0  */
        MKVLC( 0x00, 0 ), /*    */
        MKVLC( 0x00, 0 )  /*    */
    }
};

/* x264_run_before[__MIN( i_zero_left -1, 6 )][run_before] */
static const vlc_t x264_run_before[7][15] =
{
    { /* i_zero_left 1 */
        MKVLC( 0x1, 1 ), /* str=1 */
        MKVLC( 0x0, 1 ), /* str=0 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_zero_left 2 */
        MKVLC( 0x1, 1 ), /* str=1 */
        MKVLC( 0x1, 2 ), /* str=01 */
        MKVLC( 0x0, 2 ), /* str=00 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_zero_left 3 */
        MKVLC( 0x3, 2 ), /* str=11 */
        MKVLC( 0x2, 2 ), /* str=10 */
        MKVLC( 0x1, 2 ), /* str=01 */
        MKVLC( 0x0, 2 ), /* str=00 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_zero_left 4 */
        MKVLC( 0x3, 2 ), /* str=11 */
        MKVLC( 0x2, 2 ), /* str=10 */
        MKVLC( 0x1, 2 ), /* str=01 */
        MKVLC( 0x1, 3 ), /* str=001 */
        MKVLC( 0x0, 3 ), /* str=000 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_zero_left 5 */
        MKVLC( 0x3, 2 ), /* str=11 */
        MKVLC( 0x2, 2 ), /* str=10 */
        MKVLC( 0x3, 3 ), /* str=011 */
        MKVLC( 0x2, 3 ), /* str=010 */
        MKVLC( 0x1, 3 ), /* str=001 */
        MKVLC( 0x0, 3 ), /* str=000 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_zero_left 6 */
        MKVLC( 0x3, 2 ), /* str=11 */
        MKVLC( 0x0, 3 ), /* str=000 */
        MKVLC( 0x1, 3 ), /* str=001 */
        MKVLC( 0x3, 3 ), /* str=011 */
        MKVLC( 0x2, 3 ), /* str=010 */
        MKVLC( 0x5, 3 ), /* str=101 */
        MKVLC( 0x4, 3 ), /* str=100 */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
        MKVLC( 0x0, 0 ), /* str= */
    },
    { /* i_zero_left 7 */
        MKVLC( 0x7, 3 ), /* str=111 */
        MKVLC( 0x6, 3 ), /* str=110 */
        MKVLC( 0x5, 3 ), /* str=101 */
        MKVLC( 0x4, 3 ), /* str=100 */
        MKVLC( 0x3, 3 ), /* str=011 */
        MKVLC( 0x2, 3 ), /* str=010 */
        MKVLC( 0x1, 3 ), /* str=001 */
        MKVLC( 0x1, 4 ), /* str=0001 */
        MKVLC( 0x1, 5 ), /* str=00001 */
        MKVLC( 0x1, 6 ), /* str=000001 */
        MKVLC( 0x1, 7 ), /* str=0000001 */
        MKVLC( 0x1, 8 ), /* str=00000001 */
        MKVLC( 0x1, 9 ), /* str=000000001 */
        MKVLC( 0x1, 10 ), /* str=0000000001 */
        MKVLC( 0x1, 11 ), /* str=00000000001 */
    },
};


#define BLOCK_INDEX_CHROMA_DC   (-1)
#define BLOCK_INDEX_LUMA_DC     (-2)
#define INITINVALIDVEC(vec) vec.refno = -1; vec.x = vec.y = 0;

typedef struct  
{
	uint8_t len;
	uint8_t trailing_ones;
	uint8_t total_coeff;
} vlc_coeff_token_t;
#define VLC(a, b, c) {a, b, c}
#define VLC2(a, b, c) VLC(a, b, c), VLC(a, b, c)
#define VLC4(a, b, c) VLC2(a, b, c), VLC2(a, b, c)

static const uint8_t i16x16_eg_to_cbp[26][3] = 
{
	{0, 0, 0}, {0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0},
	{0, 1, 0}, {1, 1, 0}, {2, 1, 0}, {3, 1, 0}, {0, 2, 0}, 
	{1, 2, 0}, {2, 2, 0}, {3, 2, 0}, {0, 0,15}, {1, 0,15}, 
	{2, 0,15}, {3, 0,15}, {0, 1,15}, {1, 1,15}, {2, 1,15}, 
	{3, 1,15}, {0, 2,15}, {1, 2,15}, {2, 2,15}, {3, 2,15}
};

static const uint8_t i4x4_eg_to_cbp[48] =
{
	47, 31, 15, 0, 23, 27, 29, 30, 7, 11, 13, 14, 39, 43, 45,
	46, 16, 3, 5, 10, 12, 19, 21, 26, 28, 35, 37, 42, 44, 1,
	2, 4, 8, 17, 18, 20, 24, 6, 9, 22, 25, 32, 33, 34, 36, 40,
	38, 41
};

static const uint8_t inter_eg_to_cbp[48] = 
{
	0, 16, 1, 2, 4, 8, 32, 3, 5, 10, 12, 15, 47, 7, 11, 13, 14,
	6, 9, 31, 35, 37, 42, 44, 33, 34, 36, 40, 39, 43, 45, 46, 17,
	18, 20, 24, 19, 21, 26, 28, 23, 27, 29, 30, 22, 25, 38, 41
};

/* ++ cavlc tables ++ */
static const vlc_coeff_token_t coeff4_0[] = 
{
	VLC(6, 0, 2),   /* 0001 00 */
	VLC(6, 3, 3),   /* 0001 01 */
	VLC(6, 1, 2),   /* 0001 10 */
	VLC(6, 0, 1),   /* 0001 11 */
};

static const vlc_coeff_token_t coeff4_1[] = 
{
	VLC2(7, 3, 4),   /* 0000 000(0) */
	VLC(8, 2, 4),   /* 0000 0010 */
	VLC(8, 1, 4),   /* 0000 0011 */
	VLC2(7, 2, 3),   /* 0000 010(0) */
	VLC2(7, 1, 3),   /* 0000 011(0) */
	VLC4(6, 0, 4),   /* 0000 10(00) */
	VLC4(6, 0, 3),   /* 0000 11(00) */
};

static const vlc_coeff_token_t coeff3_0[] =
{
	VLC(6, 0, 1),    /* 0000 00 */ 
	VLC(6, 1, 1),    /* 0000 01 */ 
	VLC((uint8_t)(-1), (uint8_t)(-1), (uint8_t)(-1)), /* 0000 10 */ 
	VLC(6, 0, 0),    /* 0000 11 */
	VLC(6, 0, 2),    /* 0001 00 */
	VLC(6, 1, 2),    /* 0001 01 */
	VLC(6, 2, 2),    /* 0001 10 */
	VLC((uint8_t)(-1), (uint8_t)(-1), (uint8_t)(-1)), /* 0001 11 */
	VLC(6, 0, 3),    /* 0010 00 */
	VLC(6, 1, 3),    /* 0010 01 */
	VLC(6, 2, 3),    /* 0010 10 */
	VLC(6, 3, 3),    /* 0010 11 */
	VLC(6, 0, 4),    /* 0011 00 */
	VLC(6, 1, 4),    /* 0011 01 */
	VLC(6, 2, 4),    /* 0011 10 */
	VLC(6, 3, 4),    /* 0011 11 */
	VLC(6, 0, 5),    /* 0100 00 */
	VLC(6, 1, 5),    /* 0100 01 */
	VLC(6, 2, 5),    /* 0100 10 */
	VLC(6, 3, 5),    /* 0100 11 */
	VLC(6, 0, 6),    /* 0101 00 */
	VLC(6, 1, 6),    /* 0101 01 */
	VLC(6, 2, 6),    /* 0101 10 */
	VLC(6, 3, 6),    /* 0101 11 */
	VLC(6, 0, 7),    /* 0110 00 */
	VLC(6, 1, 7),    /* 0110 01 */
	VLC(6, 2, 7),    /* 0110 10 */
	VLC(6, 3, 7),    /* 0110 11 */
	VLC(6, 0, 8),
	VLC(6, 1, 8),
	VLC(6, 2, 8),
	VLC(6, 3, 8),
	VLC(6, 0, 9),
	VLC(6, 1, 9),
	VLC(6, 2, 9),
	VLC(6, 3, 9),
	VLC(6, 0, 10),
	VLC(6, 1, 10),
	VLC(6, 2, 10),
	VLC(6, 3, 10),
	VLC(6, 0, 11),
	VLC(6, 1, 11),
	VLC(6, 2, 11),
	VLC(6, 3, 11),
	VLC(6, 0, 12),
	VLC(6, 1, 12),
	VLC(6, 2, 12),
	VLC(6, 3, 12),
	VLC(6, 0, 13),
	VLC(6, 1, 13),
	VLC(6, 2, 13),
	VLC(6, 3, 13),
	VLC(6, 0, 14),
	VLC(6, 1, 14),
	VLC(6, 2, 14),
	VLC(6, 3, 14),
	VLC(6, 0, 15),
	VLC(6, 1, 15),
	VLC(6, 2, 15),
	VLC(6, 3, 15),
	VLC(6, 0, 16),
	VLC(6, 1, 16),
	VLC(6, 2, 16),
	VLC(6, 3, 16)
};

static const vlc_coeff_token_t coeff2_0[] = 
{
	VLC(4, 3, 7),   /* 1000 */
	VLC(4, 3, 6),   /* 1001 */
	VLC(4, 3, 5),   /* 1010 */
	VLC(4, 3, 4),   /* 1011 */
	VLC(4, 3, 3),   /* 1100 */
	VLC(4, 2, 2),   /* 1101 */
	VLC(4, 1, 1),   /* 1110 */
	VLC(4, 0, 0),   /* 1111 */
};

static const vlc_coeff_token_t coeff2_1[] = 
{
	VLC(5, 1, 5),   /* 0100 0 */
	VLC(5, 2, 5),
	VLC(5, 1, 4),
	VLC(5, 2, 4),
	VLC(5, 1, 3),
	VLC(5, 3, 8),
	VLC(5, 2, 3),
	VLC(5, 1, 2),
};

static const vlc_coeff_token_t coeff2_2[] = 
{
	VLC(6, 0, 3),   /* 0010 00 */
	VLC(6, 2, 7),
	VLC(6, 1, 7),
	VLC(6, 0, 2),
	VLC(6, 3, 9),
	VLC(6, 2, 6),
	VLC(6, 1, 6),
	VLC(6, 0, 1),
};

static const vlc_coeff_token_t coeff2_3[] = 
{
	VLC(7, 0, 7),   /* 0001 000 */
	VLC(7, 0, 6),
	VLC(7, 2, 9),
	VLC(7, 0, 5),
	VLC(7, 3, 10),
	VLC(7, 2, 8),
	VLC(7, 1, 8),
	VLC(7, 0, 4),
};

static const vlc_coeff_token_t coeff2_4[] = 
{
	VLC(8, 3, 12),   /* 0000 1000 */
	VLC(8, 2, 11),
	VLC(8, 1, 10),
	VLC(8, 0, 9),
	VLC(8, 3, 11),
	VLC(8, 2, 10),
	VLC(8, 1, 9),
	VLC(8, 0, 8),
};

static const vlc_coeff_token_t coeff2_5[] = 
{
	VLC(9, 0, 12),   /* 0000 0100 0 */
	VLC(9, 2, 13),
	VLC(9, 1, 12),
	VLC(9, 0, 11),
	VLC(9, 3, 13),
	VLC(9, 2, 12),
	VLC(9, 1, 11),
	VLC(9, 0, 10),
};

static const vlc_coeff_token_t coeff2_6[] = 
{
	VLC((uint8_t)(-1), (uint8_t)(-1), (uint8_t)(-1)),   /* 0000 0000 00 */
	VLC(10, 0, 16),    /* 0000 0000 01 */
	VLC(10, 3, 16),    /* 0000 0000 10 */
	VLC(10, 2, 16),    /* 0000 0000 11 */
	VLC(10, 1, 16),    /* 0000 0001 00 */
	VLC(10, 0, 15),    /* 0000 0001 01 */
	VLC(10, 3, 15),    /* 0000 0001 10 */
	VLC(10, 2, 15),    /* 0000 0001 11 */
	VLC(10, 1, 15),    /* 0000 0010 00 */
	VLC(10, 0, 14),
	VLC(10, 3, 14),
	VLC(10, 2, 14),
	VLC(10, 1, 14),
	VLC(10, 0, 13),
	VLC2(9, 1, 13),    /* 0000 0011 1(0) */
};

static const vlc_coeff_token_t coeff1_0[] = 
{
	VLC(4, 3, 4),  /* 0100 */
	VLC(4, 3, 3),  /* 0101 */
	VLC2(3, 2, 2), /* 011(0) */
	VLC4(2, 1, 1), /* 10 */
	VLC4(2, 0, 0), /* 11 */
};

static const vlc_coeff_token_t coeff1_1[] = 
{
	VLC(6, 3, 7),   /* 0001 00 */
	VLC(6, 2, 4),   /* 0001 01 */
	VLC(6, 1, 4),   /* 0001 10 */
	VLC(6, 0, 2),   /* 0001 11 */
	VLC(6, 3, 6),   /* 0010 00 */
	VLC(6, 2, 3),   /* 0010 01 */
	VLC(6, 1, 3),   /* 0010 10 */
	VLC(6, 0, 1),   /* 0010 11*/
	VLC2(5, 3, 5),   /* 0011 0(0)*/
	VLC2(5, 1, 2),   /* 0011 1(0)*/
};

static const vlc_coeff_token_t coeff1_2[] = 
{
	VLC(9, 3, 9),   /* 0000 0010 0 */
	VLC(9, 2, 7),   /* 0000 0010 1 */
	VLC(9, 1, 7),   /* 0000 0011 0 */
	VLC(9, 0, 6),   /* 0000 0011 1 */

	VLC2(8, 0, 5),   /* 0000 0100 */
	VLC2(8, 2, 6),   /* 0000 0101 */
	VLC2(8, 1, 6),   /* 0000 0110 */
	VLC2(8, 0, 4),   /* 0000 0111 */

	VLC4(7, 3, 8),    /* 0000 100 */
	VLC4(7, 2, 5),    /* 0000 101 */
	VLC4(7, 1, 5),    /* 0000 110 */
	VLC4(7, 0, 3),    /* 0000 111 */
};

static const vlc_coeff_token_t coeff1_3[] = 
{
	VLC(11, 3, 11),   /* 0000 0001 000 */
	VLC(11, 2, 9),    /* 0000 0001 001 */
	VLC(11, 1, 9),    /* 0000 0001 010 */
	VLC(11, 0, 8),    /* 0000 0001 011 */
	VLC(11, 3, 10),   /* 0000 0001 100 */
	VLC(11, 2, 8),    /* 0000 0001 101 */
	VLC(11, 1, 8),    /* 0000 0001 110 */
	VLC(11, 0, 7),    /* 0000 0001 111 */
};

static const vlc_coeff_token_t coeff1_4[] = 
{
	VLC(12, 0, 11),   /* 0000 0000 1000 */
	VLC(12, 2, 11),   /* 0000 0000 1001 */
	VLC(12, 1, 11),   /* 0000 0000 1010 */
	VLC(12, 0, 10),   /* 0000 0000 1011 */
	VLC(12, 3, 12),   /* 0000 0000 1100 */
	VLC(12, 2, 10),   /* 0000 0000 1101 */
	VLC(12, 1, 10),   /* 0000 0000 1110 */
	VLC(12, 0, 9),    /* 0000 0000 1111 */
};

static const vlc_coeff_token_t coeff1_5[] = 
{
	VLC(13, 3, 14),   /* 0000 0000 0100 0 */
	VLC(13, 2, 13),   /* 0000 0000 0100 1 */
	VLC(13, 1, 13),   /* 0000 0000 0101 0 */
	VLC(13, 0, 13),   /* 0000 0000 0101 1 */
	VLC(13, 3, 13),   /* 0000 0000 0110 0 */
	VLC(13, 2, 12),   /* 0000 0000 0110 1 */
	VLC(13, 1, 12),   /* 0000 0000 0111 0 */
	VLC(13, 0, 12),   /* 0000 0000 0111 1 */
};

static const vlc_coeff_token_t coeff1_6[] = 
{
	VLC2((uint8_t)(-1), (uint8_t)(-1), (uint8_t)(-1)),  /* 0000 0000 0000 00 */
	VLC2(13, 3, 15),   /* 0000 0000 0000 1(0) */
	VLC(14, 3, 16),   /* 0000 0000 0001 00 */
	VLC(14, 2, 16),   /* 0000 0000 0001 01 */
	VLC(14, 1, 16),   /* 0000 0000 0001 10 */
	VLC(14, 0, 16),   /* 0000 0000 0001 11 */

	VLC(14, 1, 15),   /* 0000 0000 0010 00 */
	VLC(14, 0, 15),   /* 0000 0000 0010 01 */
	VLC(14, 2, 15),   /* 0000 0000 0010 10 */
	VLC(14, 1, 14),   /* 0000 0000 0010 11 */
	VLC2(13, 2, 14),   /* 0000 0000 0011 0(0) */
	VLC2(13, 0, 14),   /* 0000 0000 0011 1(0) */
};

static const vlc_coeff_token_t coeff0_0[] = 
{
	VLC2((uint8_t)(-1), (uint8_t)(-1), (uint8_t)(-1)), /* 0000 0000 0000 000(0) */
	VLC2(15, 1, 13),  /* 0000 0000 0000 001(0) */
	VLC(16, 0, 16),   /* 0000 0000 0000 0100 */
	VLC(16, 2, 16),   
	VLC(16, 1, 16),
	VLC(16, 0, 15),
	VLC(16, 3, 16),
	VLC(16, 2, 15),
	VLC(16, 1, 15),
	VLC(16, 0, 14),
	VLC(16, 3, 15),
	VLC(16, 2, 14),
	VLC(16, 1, 14),
	VLC(16, 0, 13),   /* 0000 0000 0000 1111 */
	VLC2(15, 3, 14),  /* 0000 0000 0001 000(0) */
	VLC2(15, 2, 13),
	VLC2(15, 1, 12),
	VLC2(15, 0, 12),
	VLC2(15, 3, 13),
	VLC2(15, 2, 12),
	VLC2(15, 1, 11),
	VLC2(15, 0, 11),  /* 0000 0000 0001 111(0) */
	VLC4(14, 3, 12),  /* 0000 0000 0010 00(00) */
	VLC4(14, 2, 11),
	VLC4(14, 1, 10),
	VLC4(14, 0, 10),
	VLC4(14, 3, 11),
	VLC4(14, 2, 10),
	VLC4(14, 1, 9),
	VLC4(14, 0, 9),  /* 0000 0000 0011 11(00) */
};

static const vlc_coeff_token_t coeff0_1[] = 
{
	VLC(13, 0, 8),   /* 0000 0000 0100 0 */
	VLC(13, 2, 9),
	VLC(13, 1, 8),
	VLC(13, 0, 7),
	VLC(13, 3, 10),
	VLC(13, 2, 8),
	VLC(13, 1, 7),
	VLC(13, 0, 6),  /* 0000 0000 0111 1 */
};

static const vlc_coeff_token_t coeff0_2[] = 
{
	VLC(11, 3, 9),   /* 0000 0000 100 */
	VLC(11, 2, 7),
	VLC(11, 1, 6),
	VLC(11, 0, 5),   /* 0000 0000 111 */
	VLC2(10, 3, 8),  /* 0000 0001 00(0) */
	VLC2(10, 2, 6),
	VLC2(10, 1, 5),
	VLC2(10, 0, 4),  /* 0000 0001 11(0) */
	VLC4(9, 3, 7),  /* 0000 0010 0(0) */
	VLC4(9, 2, 5),
	VLC4(9, 1, 4),
	VLC4(9, 0, 3),  /* 0000 0011 1(0) */
};

static const vlc_coeff_token_t coeff0_3[] = 
{
	VLC(8, 3, 6),   /* 0000 0100 */
	VLC(8, 2, 4),
	VLC(8, 1, 3),
	VLC(8, 0, 2),
	VLC2(7, 3, 5),  /* 0000 100 */
	VLC2(7, 2, 3),
	VLC4(6, 3, 4),  /* 0000 11 */
};

static const vlc_coeff_token_t coeff0_4[] = 
{
	VLC(6, 1, 2),    /* 0001 00 */
	VLC(6, 0, 1),    /* 0001 01 */
	VLC2(5, 3, 3)    /* 0001 1 */
};

static const vlc_coeff_token_t coeff0_5[] = 
{
	VLC((uint8_t)(-1), (uint8_t)(-1), (uint8_t)(-1)),   /* 000 */
	VLC(3, 2, 2),      /* 001 */
	VLC2(2, 1, 1),     /* 01 */
	VLC4(1, 0, 0)      /* 1 */
};

static const uint8_t prefix_table0[] = 
{
	(uint8_t)(-1),
	3,
	2, 2,
	1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0
};

static const uint8_t prefix_table1[] = 
{
	(uint8_t)(-1),
	7,
	6, 6,
	5, 5, 5, 5,
	4, 4, 4, 4, 4, 4, 4, 4
};

static const uint8_t prefix_table2[] =
{
	(uint8_t)(-1),
	11,
	10, 10,
	9, 9, 9, 9,
	8, 8, 8, 8, 8, 8, 8, 8
};

static const uint8_t prefix_table3[] = 
{
	(uint8_t)(-1),
	15,
	14, 14,
	13, 13, 13, 13,
	12, 12, 12, 12, 12, 12, 12, 12
};

#undef VLC
#undef VLC2
#undef VLC4
#define VLC(a, b) {a, b}
#define VLC2(a, b) VLC(a, b), VLC(a, b)
#define VLC4(a, b) VLC2(a, b), VLC2(a, b)
#define VLC8(a, b) VLC4(a, b), VLC4(a, b)

typedef struct  
{
	uint8_t num;
	uint8_t len;
} zero_count_t;

static const zero_count_t total_zero_table1_0[] = 
{
	VLC((uint8_t)(-1), (uint8_t)(-1)),
	VLC(15, 9), /* 0000 0000 1 */
	VLC(14, 9),
	VLC(13, 9), /* 0000 0001 1 */
	VLC2(12, 8),/* 0000 0010 */
	VLC2(11, 8),/* 0000 0011 */
	VLC4(10, 7),/* 0000 010 */
	VLC4(9, 7), /* 0000 011 */
	VLC8(8, 6), /* 0000 10 */
	VLC8(7, 6), /* 0000 11 */
};

static const zero_count_t total_zero_table1_1[] = 
{
	VLC2((uint8_t)(-1), (uint8_t)(-1)),
	VLC(6, 5), /* 0001 0 */
	VLC(5, 5), /* 0001 1 */
	VLC2(4, 4),/* 0010 */
	VLC2(3, 4),/* 0011 */
	VLC4(2, 3),/* 010 */
	VLC4(1, 3),/* 011 */
	VLC8(0, 1), /*1 */
	VLC8(0, 1), /*1 */
};

static const zero_count_t total_zero_table2_0[] = 
{
	VLC(14, 6), /* 0000 00 */
	VLC(13, 6),
	VLC(12, 6),
	VLC(11, 6),
	VLC2(10, 5),/* 0001 0 */
	VLC2(9, 5),
};

static const zero_count_t total_zero_table2_1[] = 
{
	VLC2((uint8_t)(-1), (uint8_t)(-1)),
	VLC(8, 4), /* 0010 */
	VLC(7, 4), /* 0011 */
	VLC(6, 4),
	VLC(5, 4),
	VLC2(4, 3),/* 011 */
	VLC2(3, 3),/* 100 */
	VLC2(2, 3), /*101 */
	VLC2(1, 3), /*110 */
	VLC2(0, 3), /*111 */
};

static const zero_count_t total_zero_table3_0[] = 
{
	VLC(13, 6), /* 0000 00 */
	VLC(11, 6),
	VLC2(12, 5),/* 0000 1 */
	VLC2(10, 5),/* 0001 0 */
	VLC2(9, 5), /* 0001 1 */
};

static const zero_count_t total_zero_table3_1[] = 
{
	VLC2((uint8_t)(-1), (uint8_t)(-1)),
	VLC(8, 4), /* 0010 */
	VLC(5, 4), /* 0011 */
	VLC(4, 4),
	VLC(0, 4),
	VLC2(7, 3),/* 011 */
	VLC2(6, 3),/* 100 */
	VLC2(3, 3), /*101 */
	VLC2(2, 3), /*110 */
	VLC2(1, 3), /*111 */
};

static const zero_count_t total_zero_table6_0[] = 
{
	VLC(10, 6), /* 0000 00 */
	VLC(0, 6),
	VLC2(1, 5),/* 0000 1 */
	VLC4(8, 4),/* 0000 1 */
};

static const zero_count_t total_zero_table6_1[] = 
{
	VLC((uint8_t)(-1), (uint8_t)(-1)),
	VLC(9, 3), /* 001 */
	VLC(7, 3), /* 010 */
	VLC(6, 3),
	VLC(5, 3),
	VLC(4, 3),
	VLC(3, 3),
	VLC(2, 3)
};

static const zero_count_t total_zero_table7_0[] = 
{
	VLC(9, 6), /* 0000 00 */
	VLC(0, 6),
	VLC2(1, 5),/* 0000 1 */
	VLC4(7, 4),/* 0001 */
};

static const zero_count_t total_zero_table7_1[] = 
{
	VLC((uint8_t)(-1), (uint8_t)(-1)),
	VLC(8, 3), /* 001 */
	VLC(6, 3), /* 010 */
	VLC(4, 3),
	VLC(3, 3),
	VLC(2, 3),
	VLC2(5, 2)
};

static const zero_count_t total_zero_table8_0[] = 
{
	VLC(8, 6), /* 0000 00 */
	VLC(0, 6),
	VLC2(2, 5),/* 0000 1 */
	VLC4(1, 4),/* 0001 */
};

static const zero_count_t total_zero_table8_1[] = 
{
	VLC((uint8_t)(-1), (uint8_t)(-1)),
	VLC(7, 3), /* 001 */
	VLC(6, 3), /* 010 */
	VLC(3, 3),
	VLC2(5, 2),
	VLC2(4, 2)
};

static const zero_count_t total_zero_table9_0[] = 
{
	VLC(1, 6), /* 0000 00 */
	VLC(0, 6),
	VLC2(7, 5),/* 0000 1 */
	VLC4(2, 4),/* 0001 */
};

static const zero_count_t total_zero_table9_1[] = 
{
	VLC((uint8_t)(-1), (uint8_t)(-1)),
	VLC(5, 3), /* 001 */
	VLC2(6, 2), /* 01 */
	VLC2(4, 2),
	VLC2(3, 2),
};

static const zero_count_t total_zero_table4_0[] = 
{
	VLC(12, 5), /* 0000 0 */
	VLC(11, 5),
	VLC(10, 5), /* 0000 1 */
	VLC(0, 5),  /* 0001 1 */
	VLC2(9, 4), /* 0010 */
	VLC2(7, 4),
	VLC2(3, 4),
	VLC2(2, 4), /* 0101 */
	VLC4(8, 3), /* 011 */
};

static const zero_count_t total_zero_table4_1[] = 
{
	VLC(6, 3),   /* 100 */
	VLC(5, 3),   /* 101 */
	VLC(4, 3),   /* 110 */
	VLC(1, 3)    /* 111 */
};

static const zero_count_t total_zero_table5_0[] = 
{
	VLC(11, 5),  /* 0000 0 */
	VLC(9, 5),
	VLC2(10, 4), /* 0000 1 */
	VLC2(8, 4),  /* 0010 */
	VLC2(2, 4),
	VLC2(1, 4),
	VLC2(0, 4),
	VLC4(7, 3)
};

static const zero_count_t total_zero_table5_1[] = 
{
	VLC(6, 3), /* 100 */
	VLC(5, 3),
	VLC(4, 3),
	VLC(3, 3)
};

static const zero_count_t total_zero_table10_0[] = 
{
	VLC(1, 5), /* 0000 0 */
	VLC(0, 5),
	VLC2(6, 4), /* 0000 1 */
};

static const zero_count_t total_zero_table10_1[] = 
{
	VLC((uint8_t)(-1), (uint8_t)(-1)),
	VLC(2, 3), /* 001 */
	VLC2(5, 2), /* 01 */
	VLC2(4, 2),
	VLC2(3, 2),
};

static const zero_count_t total_zero_table11_0[] = 
{
	VLC(0, 4), /* 0000 */
	VLC(1, 4),
	VLC2(2, 3), /* 010 */
	VLC2(3, 3),
	VLC2(5, 3),
	VLC8(4, 1)
};

static const zero_count_t total_zero_table12_0[] = 
{
	VLC(0, 4), /* 0000 */
	VLC(1, 4),
	VLC2(4, 3), /* 010 */
	VLC4(2, 2),
	VLC8(3, 1)
};

static const zero_count_t total_zero_table13_0[] = 
{
	VLC(0, 3), /* 000 */
	VLC(1, 3),
	VLC2(3, 2), /* 01 */
	VLC4(2, 1),
};

static const zero_count_t total_zero_table14_0[] = 
{
	VLC(0, 2), 
	VLC(1, 2),
	VLC2(2, 1),
};

static const zero_count_t total_zero_table_chroma[3][8] = 
{
	{
		VLC(3, 3), 
			VLC(2, 3),
			VLC2(1, 2),
			VLC4(0, 1)
	},
	{
		VLC2(2, 2),
			VLC2(1, 2),
			VLC4(0, 1)
		},
		{
			VLC4(1, 1),
				VLC4(0, 1)
		}
};

static const zero_count_t run_before_table_0[7][8] = 
{
	{
		VLC4(1, 1),
		VLC4(0, 1)
	},
	{
		VLC2(2, 2),
		VLC2(1, 2),
		VLC4(0, 1)
	},
	{
		VLC2(3, 2),
		VLC2(2, 2),
		VLC2(1, 2),
		VLC2(0, 2)
	},
	{
		VLC(4, 3),
		VLC(3, 3),
		VLC2(2, 2),
		VLC2(1, 2),
		VLC2(0, 2)
	},
	{
		VLC(5, 3),
		VLC(4, 3),
		VLC(3, 3),
		VLC(2, 3),
		VLC2(1, 2),
		VLC2(0, 2),
	},
	{
		VLC(1, 3),
		VLC(2, 3),
		VLC(4, 3),
		VLC(3, 3),
		VLC(6, 3),
		VLC(5, 3),
		VLC2(0, 2)
	},
	{
		VLC((uint8_t)(-1), (uint8_t)(-1)),
		VLC(6, 3),
		VLC(5, 3),
		VLC(4, 3),
		VLC(3, 3),
		VLC(2, 3),
		VLC(1, 3),
		VLC(0, 3)
	}
};

static const uint8_t run_before_table_1[] =
{
	(uint8_t)(-1),
	10,
	9, 9,
	8, 8, 8, 8,
	7, 7, 7, 7, 7, 7, 7, 7
};

static const uint8_t run_before_table_2[] =
{
	(uint8_t)(-1),
	14,
	13, 13,
	12, 12, 12, 12,
	11, 11, 11, 11, 11, 11, 11, 11
};
/* -- cavlc tables -- */

