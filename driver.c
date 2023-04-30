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
#include <math.h>

/**********************************************************************/
/*                         Symbolic Constants                         */
/**********************************************************************/
#define BLOCK_SIZE          1024 /* Size of a block                   */
#define BYTES_PER_SECTOR    512  /* Number of bytes in a sector       */
#define CYLINDERS           40   /* Number of cylinders on DISK       */
#define DISK_CAPACITY       368640 
                                 /* Total disk capacity in bytes      */
#define DMA_SETUP_CODE      3    /* Code to set DMA chip registers    */
#define INVALID_BLOCK       -4   /* Error for invalid block number    */
#define INVALID_ADDRESS     -16  /* Error for invalid data address    */
#define INVALID_OPERATION   -1   /* Error for invalid operation code  */
#define INVALID_REQUEST     -2   /* Error for invalid request number  */
#define INVALID_BLOCK_SIZE  -8   /* Error for invalid block size      */
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
void send_idle_message(MESSAGE *p_fs_message);
   /* Send an idle message to the FILE SYSTEM to ask for jobs         */
void send_message(MESSAGE *p_fs_message);
   /* Send a message to the FILE SYSTEM                               */
int  disk_drive(int code, int arg1, int arg2, int arg3, 
                                               unsigned long int *arg4);
   /* Tell the DISK drive to perform some function                    */
void convert_block(int block_number,    int *p_cylinder_number,
                   int *p_track_number, int *p_sector_number);
   /* Convert a block number to cylinder, track, and sector numbers   */
void sort_requests(MESSAGE *p_fs_message);
   /* Sort requests within the message by ascending block number      */
void check_request(MESSAGE request);
   /* Check a request for any incorrect parameters                    */
int check_operation_code(MESSAGE request);
   /* Check a request for invalid operation code                      */
int check_request_number(MESSAGE request);
   /* Check a request for invalid request number                      */
int check_block_number(MESSAGE request);
   /* Check a request for invalid block number                        */
int check_block_size(MESSAGE request);
   /* Check a request for invalid block size                          */
int check_address(MESSAGE request);
   /* Check a request for invalid data address                        */

/**********************************************************************/
/*                           Main Function                            */
/**********************************************************************/
int main()
{
   MESSAGE fs_message[MESSAGE_SIZE]; /* Message sent to FILE SYSTEM   */
   int message_index;                /* Index for request in message  */
   int block,                        /* Block number                  */
       cylinder,                     /* Cylinder number               */
       sector,                       /* Sector number                 */
       track;                        /* Track number                  */
                      
   send_idle_message(fs_message);

   /*
   for(message_index = 0; message_index < 1; message_index++)
   {
      printf("\nOperation Code: %d", fs_message[message_index].operation_code);
      printf("\nRequest Number: %d", fs_message[message_index].request_number);
      printf("\nBlock Number  : %d", fs_message[message_index].block_number);
      printf("\nBlock Size    : %d", fs_message[message_index].block_size);
      printf("\n\n");
   }
   */

   if(disk_drive(START_MOTOR_CODE, 0, 0, 0, 0) == 1)
   {
      for(message_index = 0; message_index < MESSAGE_SIZE; message_index++)
      {
         if(disk_drive(STATUS_MOTOR_CODE, 0, 0, 0, 0) == 0)
         {
            convert_block(fs_message[message_index].block_number, &cylinder, &track, &sector);
            
            printf("\n");
            printf("\nOperation Code: %d", fs_message[message_index].operation_code);
            printf("\nRequest Number: %d", fs_message[message_index].request_number);
            printf("\nBlock Number  : %d", fs_message[message_index].block_number);
            printf("\nBlock Size    : %d", fs_message[message_index].block_size);
            printf("\n\n");

            if(disk_drive(SENSE_CODE, 0, 0, 0, 0) != cylinder)
            {
               
               printf("\n\nCylinder is: %d", disk_drive(SENSE_CODE, 0, 0, 0, 0));
               printf("\nCylinder should be: %d", cylinder);
               printf("\n\n");
               
               while(disk_drive(SEEK_CODE, cylinder, 0, 0, 0) != cylinder &&
                     disk_drive(SEEK_CODE, cylinder, 0, 0, 0) != -1)
               {
                  printf("\n\nNot on correct cylinder\n\n");
               }
            }
            else
            {
               while(disk_drive(RECALIBRATE_CODE, 0, 0, 0, 0) != 0)
               {
                  printf("\n\nNot on cylinder 0\n\n");
               }
               
               if(disk_drive(RECALIBRATE_CODE, 0, 0, 0, 0) == 0)
               {
                  disk_drive(SEEK_CODE, cylinder, 0, 0, 0);
               }

            }
            
            if(disk_drive(DMA_SETUP_CODE, sector, track, 
                          fs_message[message_index].block_size,
                          fs_message[message_index].p_data_address) == 0)
            {
               if(disk_drive(WRITE_DATA_CODE, 0, 0, 0, 0) == 0)
               {
                  fs_message[message_index].operation_code = 0;
                  fs_message[message_index].request_number = fs_message[message_index].request_number;
                  fs_message[message_index].block_number   = fs_message[message_index].block_number;
                  fs_message[message_index].block_size     = fs_message[message_index].block_size;
                  fs_message[message_index].p_data_address = fs_message[message_index].p_data_address;
                  send_message(fs_message);

               }
               else
               {
                  if(disk_drive(WRITE_DATA_CODE, 0, 0, 0, 0) == 0)
                  {
                     fs_message[message_index].operation_code = 0;
                     fs_message[message_index].request_number = fs_message[message_index].request_number;
                     fs_message[message_index].block_number   = fs_message[message_index].block_number;
                     fs_message[message_index].block_size     = fs_message[message_index].block_size;
                     fs_message[message_index].p_data_address = fs_message[message_index].p_data_address;
                     send_message(fs_message);

                     printf("\n\nbooyah! Success\n\n");
                  }
               }
            }    
         }
      }
   }
    
   return 0;
}

/**********************************************************************/
/*     Convert block number to cylinder, track, and sector numbers    */
/**********************************************************************/
void convert_block(int block_number,    int *p_cylinder_number,
                   int *p_track_number, int *p_sector_number)
{  
   int block_remainder; /* Remainder of the block left                */
       
   block_remainder = (block_number - 1) % SECTORS_PER_TRACK;
   
   /* Calculate the cylinder number from its associated block number  */
   *p_cylinder_number = (block_number - 1) / SECTORS_PER_TRACK;

   /* Calculate the track number from its associated block number     */
   if (block_remainder < (int) (((float) SECTORS_PER_TRACK * 0.5f) + 0.5f))
   {
      *p_track_number = 0;
   }
   else
   {
      *p_track_number = 1;
   }

   /* Calculate the sector number from its associated block number    */
   if (block_remainder * TRACKS_PER_CYLINDER < SECTORS_PER_TRACK)
   {
      *p_sector_number = (block_remainder) * TRACKS_PER_CYLINDER;
   }
   else
   {
      *p_sector_number = (block_remainder * TRACKS_PER_CYLINDER) - SECTORS_PER_TRACK;
   }

   return;
}

/**********************************************************************/
/*      Send an idle message to the FILE SYSTEM to ask for jobs       */
/**********************************************************************/
void send_idle_message(MESSAGE *p_fs_message)
{
   p_fs_message[0].operation_code = 0;
   p_fs_message[0].request_number = 0;
   p_fs_message[0].block_number   = 0;
   p_fs_message[0].block_size     = 0;
   p_fs_message[0].p_data_address = NULL;
   send_message(p_fs_message);

   return;
}

/**********************************************************************/
/*     Sort requests within the message by ascending block number     */
/**********************************************************************/
void sort_requests(MESSAGE *p_fs_message)
{
   int message_index;    /* Index for request in message              */
   MESSAGE temp_request; /* Temporary request for swap                */

   for(message_index = 0; message_index < MESSAGE_SIZE; message_index++)
   {
      if(p_fs_message[message_index].block_number > p_fs_message[message_index + 1].block_number &&
        (message_index + 1) < MESSAGE_SIZE);
      {
         temp_request = p_fs_message[message_index];
         p_fs_message[message_index] = p_fs_message[message_index + 1];
         p_fs_message[message_index + 1] = temp_request;
      }
   }

   return;
}

/**********************************************************************/
/*            Check a request for any incorrect parameters            */
/**********************************************************************/
void check_request(MESSAGE request)
{
   if(check_operation_code(request) && check_request_number(request))
   {
      request.operation_code = INVALID_OPERATION + INVALID_REQUEST;
   }
   else
   {
      if(check_operation_code(request) && check_block_number(request))
      {
         request.operation_code = INVALID_OPERATION + INVALID_REQUEST;
      }
      else
      {
         if(check_request_number(request) && check_block_number(request))
         {
            request.operation_code = INVALID_REQUEST + INVALID_BLOCK;
         }
         else
         {
            if(check_operation_code(request) && check_request_number(request) &&
               check_block_number(request))
            {
               request.operation_code = INVALID_OPERATION + 
                                        INVALID_REQUEST + INVALID_BLOCK;
            }
            else
            {
               if(check_block_size(request) && check_operation_code(request))
               {
                  request.operation_code = INVALID_BLOCK_SIZE + INVALID_OPERATION;
               }
            }
         }
      }
   }

   return;
}

/**********************************************************************/
/*             Check a request for invalid operation code             */
/**********************************************************************/
int check_operation_code(MESSAGE request)
{  
   if (request.operation_code != 1 || request.operation_code != 2)
   {
      request.operation_code = INVALID_OPERATION;
   }
   
   return (request.operation_code != 1 || request.operation_code != 2);
}

/**********************************************************************/
/*             Check a request for invalid request number             */
/**********************************************************************/
int check_request_number(MESSAGE request)
{
   if (request.request_number <= 0)
   {
      request.operation_code = INVALID_REQUEST;
   }
   
   return (request.request_number <= 0);
}

/**********************************************************************/
/*              Check a request for invalid block number              */
/**********************************************************************/
int check_block_number(MESSAGE request)
{
   if (request.block_number < 1 || request.block_number > 360)
   {
      request.operation_code = INVALID_BLOCK;
   }
   
   return (request.block_number < 1 || request.block_number > 360);
}

/**********************************************************************/
/*               Check a request for invalid block size               */
/**********************************************************************/
int check_block_size(MESSAGE request)
{
   int bytes_per_cylinder; /* Number of bytes per DISK cylinder       */

   bytes_per_cylinder = BYTES_PER_SECTOR * SECTORS_PER_TRACK *
                        TRACKS_PER_CYLINDER;
   
   if (request.block_size < 0 || (double) request.block_size ==
                               log10(request.block_size) / log10(2.0) ||
                               request.block_size > bytes_per_cylinder)
   {
      request.operation_code = INVALID_BLOCK_SIZE;
   }

   return (request.block_size < 0 ||
          (double) request.block_size ==
                               log10(request.block_size) / log10(2.0) ||
          request.block_size > bytes_per_cylinder);
}

/**********************************************************************/
/*              Check a request for invalid data address              */
/**********************************************************************/
int check_address(MESSAGE request)
{
   if (request.p_data_address < 0)
   {
      request.operation_code = INVALID_ADDRESS;
   }
   
   return (request.p_data_address < 0);
}