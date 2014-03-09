#ifndef _BASE64_H
#define _BASE64_H

#include <stdint.h>

template <bool>
struct b64_bool{};

struct b64_sfinae
{
	struct static_assert_failed;
	static static_assert_failed f(...);
	static int f(b64_bool<true>);
};

#define B64_STATIC_ASSERT(x) sizeof(b64_sfinae::f(b64_bool<x>()))

/* base64_encode:
 * 		Description:
 * 			Encode a string of characters as base64
 * 		Parameters:
 * 			output: the output buffer for the encoding, stores the encoded string
 * 			input: the input buffer for the encoding, stores the binary to be encoded
 * 			inputLen: the length of the input buffer, in bytes
 * 		Return value:
 * 			Returns the length of the encoded string
 * 		Requirements:
 * 			1. output must not be null or empty
 * 			2. input must not be null
 * 			3. inputLen must be greater than or equal to 0
 */
uint16_t base64_encode(char* output, char const* input, uint16_t inputLen);

template <typename T, typename U>
uint16_t base64_encode(T* output, U const* input, uint16_t inputLen)
{
    B64_STATIC_ASSERT(sizeof(T) == 1);
    B64_STATIC_ASSERT(sizeof(U) == 1);

    return base64_encode(reinterpret_cast<char*>(output),
                         reinterpret_cast<char const*>(input),
                         inputLen);
}

/* base64_decode:
 * 		Description:
 * 			Decode a base64 encoded string into bytes
 * 		Parameters:
 * 			output: the output buffer for the decoding,
 * 					stores the decoded binary
 * 			input: the input buffer for the decoding,
 * 				   stores the base64 string to be decoded
 * 			inputLen: the length of the input buffer, in bytes
 * 		Return value:
 * 			Returns the length of the decoded string
 * 		Requirements:
 * 			1. output must not be null or empty
 * 			2. input must not be null
 * 			3. inputLen must be greater than or equal to 0
 */
uint16_t base64_decode(char* output, char const* input, uint16_t inputLen);

template <typename T, typename U>
uint16_t base64_decode(T* output, U const* input, uint16_t inputLen)
{
    B64_STATIC_ASSERT(sizeof(T) == 1);
    B64_STATIC_ASSERT(sizeof(U) == 1);

    return base64_decode(reinterpret_cast<char*>(output),
                         reinterpret_cast<char const*>(input),
                         inputLen);
}

/* base64_enc_len:
 * 		Description:
 * 			Returns the length of a base64 encoded string whose decoded
 * 			form is inputLen bytes long
 * 		Parameters:
 * 			inputLen: the length of the decoded string
 * 		Return value:
 * 			The length of a base64 encoded string whose decoded form
 * 			is inputLen bytes long
 * 		Requirements:
 * 			None
 */
int base64_enc_len(int inputLen);

/* base64_dec_len:
 * 		Description:
 * 			Returns the length of the decoded form of a
 * 			base64 encoded string
 * 		Parameters:
 * 			input: the base64 encoded string to be measured
 * 			inputLen: the length of the base64 encoded string
 * 		Return value:
 * 			Returns the length of the decoded form of a
 * 			base64 encoded string
 * 		Requirements:
 * 			1. input must not be null
 * 			2. input must be greater than or equal to zero
 */
int base64_dec_len(char *input, int inputLen);

#endif // _BASE64_H
