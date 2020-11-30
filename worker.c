/* 
 * Operating Systems <2INCO> Practical Assignment
 * Interprocess Communication
 *
 * STUDENT_NAME_1 (STUDENT_NR_1)
 * STUDENT_NAME_2 (STUDENT_NR_2)
 *
 * Grading:
 * Students who hand in clean code that fully satisfies the minimum requirements will get an 8. 
 * Extra steps can lead to higher marks because we want students to take the initiative. 
 * These extra steps must be agreed with the tutor before delivery.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>          // for perror()
#include <unistd.h>         // for getpid()
#include <mqueue.h>         // for mq-stuff
#include <time.h>           // for time()
#include <complex.h>

#include "common.h"
#include "md5s.h"

static void rsleep (int t);
char * workerProcess(char fstLetter, char start, char end,	uint128_t MD5_checksum, char result[7]);
void generate(char *r, char count, char start, char end);


int main (int argc, char * argv[])
{
    mqd_t	FarmerQueue; //defining Farmer's queue
    mqd_t	WorkersQueue; //defining Workers' queue

    struct mq_attr	FarmerQueueAttributes; //defining Farmer's queue attributes
    struct mq_attr	WorkersQueueAttributes; //defining Workers' queue attributes


    FarmerQueue = mq_open (argv[1], O_RDONLY, 0600, &FarmerQueueAttributes); //open Farmer's queue
    WorkersQueue = mq_open (argv[2], O_WRONLY, 0600, &WorkersQueueAttributes); //open Workers' queue

    MQ_REQUEST_MESSAGE MsgFromFarmer; //defining msg from Farmer to Worker (see common.h)
    MQ_RESULT_MESSAGE MsgFromWorker; //defining msg from Worker to Farmer (see common.h)

    char msgToBeSent[MAX_MESSAGE_LENGTH]; //defining the msg to be sent to farmer

    while(true)
    {
        /// mg_receive(Queue from which we receive, buffer where we put the msg after extracting, size of buffer, msg priority)
        mq_receive(FarmerQueue, (char*)&MsgFromFarmer, sizeof(MQ_REQUEST_MESSAGE), 0); //Receive (extract) the oldest message from Farmer's queue and load it in MsgFromFarmer
        rsleep(10000); //wait

        /// provide the worker process function with proper parameters from Farmer and the message to be sent to Farmer
        workerProcess(MsgFromFarmer.startCharacter, MsgFromFarmer.firstCharAlphabet, MsgFromFarmer.lastCharAlphabet, MsgFromFarmer.MD5_checksum, msgToBeSent);
        if(msgToBeSent[0]!='\0'){ //if message is correct
            strcpy(MsgFromWorker.resultingString, msgToBeSent); //copy the calculated string by workerProcess to MsgFromWorker.resultingString
        } else{
            strcpy(MsgFromWorker.resultingString, "\0"); //set MsgFromWorker.resultingString to "\0"
        }
        MsgFromWorker.iD = MsgFromFarmer.iD; //set the id to the message to be sent the same as the received one

        mq_send(WorkersQueue, (char*)&MsgFromWorker, sizeof(MQ_RESULT_MESSAGE), 0); //send  MsgToFarmer to Workers' queue

    }

    return (0);
}

/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time
 * between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
    static bool first_call = true;
    
    if (first_call == true)
    {
        srandom (time (NULL) % getpid ());
        first_call = false;
    }
    usleep (random() % t);
}

/*!
 * Routine of the worker. Use {@code generate) to generate all possible strings and
 * compare them with the given hash.
 * @param fstLetter the letter given by Farmer to start with
 * @param start first letter of the alphabet
 * @param end last letter of the alphabet
 * @param md5checksum hash to compare with
 * @param result string containing the result
*/
char * workerProcess(char fstLetter, char start, char end,	uint128_t MD5_checksum, char result[MAX_MESSAGE_LENGTH]){
    char strToCheck [MAX_MESSAGE_LENGTH]; //this will be the last generated string by brute-forcing. We will use it to stop the workerProcess if hashes dont match until this string
    char count = 0;
    for (size_t i = 0; i < MAX_MESSAGE_LENGTH; i++) {
        strToCheck[i] = end;
        result[i]='\0'; //create an empty char[]
    }
    strToCheck [0] = fstLetter; //set first char of our string to the first letter given by Farmer
    result[0]= fstLetter; //set first char of result array to the same thing


    while (1){

        uint128_t hash = md5s(result, strlen(result)); //calculate MD5 hash of result string

        if(hash==MD5_checksum){ //compare with hash given by farmer

            return result; //if true return result
        }
        generate(result, count,start,end); //produce all possible strings via backtracking

        /// If we have tried all the options with that start letter set the result string to be empty
        if(!strcmp(result,strToCheck)){
            for (size_t i = 0; i < 7; i++) {
                result[i]='\0';
            }
            return result; //return empty string
        }
    }

}

/*!
 * Compute all possible string of given length, starting with the given letter from Farmer. Use backtracking
 * to check if each of these is the correct one
 * @param r pointer to the first element of the result array in {@code workerProcess}
 * @param count number of times we add a new symbol
 * @param start first character of the alphabet
 * @param end last character of the alphabet
*/
void generate(char *r, char count, char start, char end){
    if(r[1]=='\0'){ //if element after the pointer is empty set it to the first char of the alphabet
        r[1]=start;
        count++;
    }
    else{ //else change it to the next char of the alphabet
        r[1]++;
    }
    ///When we have gone past the last character of the alphabet and still have space in the message
    if(r[1]>end && count<=MAX_MESSAGE_LENGTH-1){
        r[1]=start; //set element after the pointer to the first char of the alphabet
        generate(r+1, count, start, end); //move the pointer by 1 point and increment next symbol
    }
}


