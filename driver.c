/**********************************************************************/
/*                                                                    */
/* Program Name: driver - Creates a DISK device driver                */
/* Author:       William S. Wu                                        */
/* Installation: Pensacola Christian College, Pensacola, Florida      */
/* Course:       CS326, Operating Systems                             */
/* Date Written: April 15, 2023                                       */
/*                                                                    */
/**********************************************************************/

/**********************************************************************/
/*                                                                    */
/* I pledge this assignment is my own first time work.                */
/* I pledge I did not copy or try to copy work from the Internet.     */
/* I pledge I did not copy or try to copy work from any student.      */
/* I pledge I did not copy or try to copy work from any where else.   */
/* I pledge the only person I asked for help from was my teacher.     */
/* I pledge I did not attempt to help any student on this assignment. */
/* I understand if I violate this pledge I will receive a 0 grade.    */
/*                                                                    */
/*                                                                    */
/*                      Signed: _____________________________________ */
/*                                           (signature)              */
/*                                                                    */
/*                                                                    */
/**********************************************************************/

/**********************************************************************/
/*                                                                    */
/* This program accepts read/write requests from a FILE SYSTEM,       */
/* translates them from physical block numbers into disk drive        */
/* cylinder, trak, and sector numbers, then instructs a DISK device   */
/* to carry out the read and write requests.                          */
/*                                                                    */
/**********************************************************************/

#include <stdio.h>
#include <stdlib.h>

/**********************************************************************/
/*                         Symbolic Constants                         */
/**********************************************************************/
#define BLOCK_SIZE          1024 /* Size of a block                   */
#define BYTES_PER_SECTOR    512  /* Number of bytes in a sector       */
#define CYLINDERS           40   /* Number of cylinders on DISK       */
#define DISK_CAPACITY       368640 
                                 /* Total disk capacity in bytes      */
#define DMA_SETUP_CODE      3    /* Code to set DMA chip registers    */
#define MAX_SPEED_SECS      3    /* Max secs for motor's speed        */
#define MESSAGE_SIZE        20   /* Size of a message to FILE SYSTEM  */
#define READ_DATA_CODE      6    /* Code to read data                 */
#define RECALIBRATE_CODE    9    /* Code to reset DISK heads          */
#define SECTORS_PER_TRACK   9    /* Number of sectors in a track      */
#define SEEK_CODE           2    /* Code to send heads to cylinder    */
#define SENSE_CODE          1    /* Code to sense cylinder position   */
#define START_MOTOR_CODE    4    /* Code to turn DISK drive motor on  */
#define STATUS_MOTOR_CODE   5    /* Code to wait for motor's speed    */
#define STOP_MOTOR_CODE     8    /* Code to turn DISK drive motor off */
#define TOTAL_BLOCKS        360  /* Number of blocks on the DISK      */
#define TRACKS_PER_CYLINDER 2    /* Number of tracks in cylinder      */
#define WRITE_DATA_CODE     7    /* Code to write data                */

/**********************************************************************/
/*                         Program Structures                         */
/**********************************************************************/
/* A message to the file system from the DISK driver                  */
struct message
{
   int operation_code, /* DISK operation to be performed              */
       request_number, /* A unique request number                     */
       block_number,   /* Block number to be read or written          */
       block_size;     /* Block size in bytes                         */
   unsigned long int *p_data_address;
                       /* Points to the data block in memory          */
};
typedef struct message MESSAGE;

/**********************************************************************/
/*                         Function Prototypes                        */
/**********************************************************************/
void send_message(MESSAGE *p_fs_message);
   /* Send a message to the FILE SYSTEM                               */
void disk_drive(int code, int arg1, int arg2, int arg3, int arg4);
   /* Tell the DISK drive to perform some function                    */
/*
void senseCylinder();
void seekToCylinder();
void dmaSetup();
void startMotor();
void statusMotor();
void readData();
void writeData();
void stopMotor();
void recalibrate();
*/



/**********************************************************************/
/*                           Main Function                            */
/**********************************************************************/
int main()
{
   MESSAGE fs_message[MESSAGE_SIZE];
   int message_index;
   
   disk_drive(5, 0, 0, 0, 0);
   send_message(fs_message);

   for(message_index = 0; message_index < MESSAGE_SIZE; message_index++)
   {
      printf("\nOperation Code: %d", fs_message[message_index].operation_code);
      printf("\nRequest Number: %d", fs_message[message_index].request_number);
      printf("\nBlock Number  : %d", fs_message[message_index].block_number);
      printf("\nBlock Size    : %d", fs_message[message_index].block_size);
      printf("\n\n");
   }   
   return 0;
}