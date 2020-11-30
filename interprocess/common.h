/* 
 * Operating Systems <2INCO> Practical Assignment
 * Interprocess Communication
 *
 * Contains definitions which are commonly used by the farmer and the workers
 *
 */

#ifndef COMMON_H
#define COMMON_H


// maximum size for any message in the tests
#define MAX_MESSAGE_LENGTH	6
#define WORKER_NAME    "./worker"

// TODO: put your definitions of the datastructures here
/// This struct defines which data is send from the farmer to the worker
typedef struct MQ_REQUEST_MESSAGE {
    int  iD; //id of the result
    char startCharacter; //string starts with this character
    char firstCharAlphabet; //first character of the used alphabet
    char lastCharAlphabet; //last character of the used alphabet
    uint128_t MD5_checksum; //MD5 value we need to "decode"
} MQ_REQUEST_MESSAGE;

typedef struct MQ_RESULT_MESSAGE {
    int  iD; //id of the result
    char resultingString[MAX_MESSAGE_LENGTH + 5]; //the result of the working operation, a string whihc can be decoded in the requested MD5 ??? code extra space for \r\n\0 at end of string
} MQ_RESPONSE_MESSAGE;

#endif

