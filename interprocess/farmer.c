/* 
 * Operating Systems <2INCO> Practical Assignment
 * Interprocess Communication
 *
 * Paul Borusas (1243150)
 * Georgi Dachev (STUDENT_NR_2)
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
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>         // for execlp
#include <mqueue.h>         // for mq

#include "settings.h"
#include "common.h"

#define STUDENT_NAME        "PaulBorusas&GeorgiDachev"

//FIXME: delete this if shit on line 79 (error checking) does not work
extern int errno;

int main (int argc, char * argv[])
{
    if (argc != 1)
    {
        fprintf (stderr, "%s: invalid arguments\n", argv[0]);
    }

    // TODO:
    //  * create the message queues (see message_queue_test() in interprocess_basic.c)
    //  * create the child processes (see process_test() and message_queue_test())
    //  * do the farming
    //  * wait until the chilren have been stopped (see process_test())
    //  * clean up the message queues (see message_queue_test())

    // Important notice: make sure that the names of the message queues contain your
    // student name and the process id (to ensure uniqueness during testing)

    char farmer2WorkerQueue[100]; //name of the task queue
    char worker2FarmerQueue[100]; //name of the results queue

    pid_t processId; //ID indicating farmer process
    pid_t forkedProcessId;  //ID indicating the forked process

    mqd_t mq_fd_request; //Farmer's message queue descriptor
    mqd_t mq_fd_response; //Worker's message queue descriptor

    //MQ_REQUEST_MESSAGE  req; // variable storing the request message
    MQ_RESPONSE_MESSAGE rsp; //variable storing the response message

    struct mq_attr attr; //queue attributes
    attr.mq_maxmsg  = MQ_MAX_MESSAGES; //setting the max amount of messages supported

    MQ_RESPONSE_MESSAGE jobResults[JOBS_NROF]; //stroing all the response messages

    int receivedJobResultNo = 0; //amount of received response message results
    int jobID = 0; //job id varibale to use for restarting iterative algorithm

    //1-----------------------------------------------------------------------
    //Assign the names of the queues
    processId = getpid()
    sprintf(farmer2WorkerQueue, "/mq_farmer_%s_%d", STUDENT_NAME, processId);
    sprintf(worker2FarmerQueue, "/mq_worker_%s_%d", STUDENT_NAME, processId);


    //2-----------------------------------------------------------------------
    //Create the farmer queue
    attr.mq_msgsize = sizeof (MQ_REQUEST_MESSAGE);
    mq_fd_request = mq_open(farmer2WorkerQueue, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);
    // Handling the error
    //FIXME: Check this later
    if (mq_fd_request == -1) {
        if (errno == EEXIST) { //TODO: check this shit
            puts("Farmer queue already exists. The program will now close the queue and exit...\r\n");
            // lets remove the previous queue in order to recreate it in the next startup
            if (mq_unlink(farmer2WorkerQueue) == -1) {
                puts("Failed to unlink farmer queue. Please close it manually.");
            }

            exit(2);
        }
        printf("errno = %d\r\n", (int)errno);
        exit (3);
    }


    //3-----------------------------------------------------------------------
    //Create worker queue
    attr.mq_msgsize = sizeof(MQ_RESPONSE_MESSAGE); //setting the size of the response message
    mq_fd_response = mq_open(worker2FarmerQueue, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);
    // Handling the error
    //FIXME: Check this later
    if (mq_fd_response == -1) {
        if (errno == EEXIST) {
            puts("Worker queue already exists. The program will now close the queue and exit...\r\n");
            if (mq_close(mq_fd_request) == -1) {
                puts("Something went wrong while closing the farmer queue :-( .");
                exit(4);
            }
            // lets remove the previous queues in order to recreate it in the next startup
            if (mq_unlink(farmer2WorkerQueue) == -1) {
                puts("Failed to unlink farmer queue. Please close it manually.");
            }
            if (mq_unlink(worker2FarmerQueue) == -1) {
                puts("Failed to unlink farmer queue. Please close it manually.");
            }

            exit(5);
        }
        printf("errno = %d\r\n", (int)errno);
        exit(6);
    }

    //4-----------------------------------------------------------------------
    //Initialize worker processes

    //initialize NROF_WORKERS processes
    for (int i = 0; i < NROF_WORKERS; i++) {

        forkedProcessId = fork(); //fork the separate process

        if (forkedProcessId < 0)
        {
            printf("fork() failed, returned %d", forkedProcessId);
            exit (7);
        } else if (forkedProcessId == 0) {

            // We are the child/worker process. OK, then it is time to load the worker program.
            execlp (WORKER_NAME, WORKER_NAME, farmer2WorkerQueue, worker2FarmerQueue, NULL);
            exit (0); // This is probarbly never going to be called, but just in case.
        }
        // We are the parent process. Lets continue producing child/worker processes.
    }


    //4-----------------------------------------------------------------------
    //Farming

    //keep farming until all job results are gotten
    while (receivedJobResultNo < JOBS_NROF)
    {
        // set the job for the worker
        mq_getattr(mq_fd_request, &attr);
        while ((attr.mq_curmsgs < MQ_MAX_MESSAGES) && (jobID < JOBS_NROF))
        {
            //FIXME: delete this shit ass comment
            // Define next job
            // If this looks complicated then hopefully the following explaination makes it clear what's going on here.
            // The jobs are devided over the workers so that each worker has an MD5 and string start character.
            // Because there are N characters in our aphabet and the WorkResultID gets increased after each letter -
            //   the MD5-index gets increased every N increments. At that moment the first new job starts with -
            //   the first letter in our alphabet. Therefore MD5-index = WorkResultID / N
            //   and startChar = FirstLetterInAplhabet + (WorkResultID modulo N)
            int md5_no = JobID / ALPHABET_NROF_CHAR; //every time alphabet ends -- check another MD5
            char startCharacter = ALPHABET_START_CHAR + (JobID % ALPHABET_NROF_CHAR); //change the starting character every iteration

            //Set the data structure parameters
            MQ_REQUEST_MESSAGE message;
            message.iD = JobID;
            message.startCharacter = startCharacter;
            message.firstCharAlphabet = ALPHABET_START_CHAR;
            message.lastCharAlphabet = ALPHABET_END_CHAR;
            message.MD5_checksum = md5_list[md5_no]; //assigns specific MD5 to a message checksum attribute

            // Send message
            // Since there can only be one farmer at a given time this will never block (because of condition in while-statement)
            //TODO: understand the comment above
            mq_send(mq_fd_request, (const char*)&message, sizeof(message), 0);

            // Increase job ID for next loop
            jobID++;
            mq_getattr(mq_fd_request, &attr);
        }

        // Receive results from workers (blocking to avoid busy-waiting)
        MQ_RESULT_MESSAGE resultMessage;
        //TODO: tf is this
        unsigned messagePriority;

        mq_receive(mq_fd_response, (char*)&resultMessage, sizeof(MQ_RESULT_MESSAGE), &messagePriority);
        if ((resultMessage.iD >= 0)  && (resultMessage.iD < JOBS_NROF))
        {
            jobResults[resultMessage.iD] = resultMessage;
        } else {
            printf("The resultID == %d is outside our range!\r\n", resultMessage.iD);
            exit(10);
        }
        receivedJobResultNo++;
    }


    //5-----------------------------------------------------------------------
    //Result output to console
    //TODO: change the code so that it looks different from their
    for (int i = 0; i < JOBS_NROF; i++)
    {
        // This implicitly skips empty strings
        if (strlen(jobResults[i].MatchingString) != 0)
        {
            printf("\'%s\'\r\n", jobResults[i].MatchingString);
        }
    }


    //TODO: change error messages
    //5-----------------------------------------------------------------------
    //Process ending
    if (mq_close(mq_fd_request) == -1)
    {
        puts("Something went wrong while closing the farmer queue :-( .");
        exit (21);
    }
    if (mq_close(mq_fd_response) == -1)
    {
        puts("Something went wrong while closing the worker queue :-( .");
        exit (22);
    }

    if (mq_unlink(farmer2WorkerQueue) == -1)
    {
        puts("Failed to unlink farmer queue. Please close it manually.");
        exit (23);
    }
    if (mq_unlink(worker2FarmerQueue) == -1)
    {
        puts("Failed to unlink farmer queue. Please close it manually.");
        exit (24);
    }

    return (0);
}

