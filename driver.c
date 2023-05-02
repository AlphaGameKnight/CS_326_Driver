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
#include <stdbool.h>

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
int count_requests(MESSAGE *p_fs_message);
   /* Count the number of pending requests                            */
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
   MESSAGE *p_fs_message; /* Points to a message sent to FILE SYSTEM  */
   int message_index;     /* Index for request in message             */
   int cylinder,          /* Cylinder number                          */
       sector,            /* Sector number                            */
       status_code,       /* Status of an operation                   */
       track;             /* Track number                             */

   /* Create an empty list of pending requests                        */
   p_fs_message = (MESSAGE *) malloc(MESSAGE_SIZE * sizeof(MESSAGE));

   /* Send an idle message to the FILE SYSTEM, requesting work        */
   send_idle_message(p_fs_message);

   /* Loop continuously processing the pending requests list          */
   while(true)
   {
      if(p_fs_message[0].operation_code != 0 &&
         p_fs_message[0].request_number != 0 &&
         p_fs_message[0].block_number   != 0 &&
         p_fs_message[0].block_size     != 0 &&
         p_fs_message[0].p_data_address != NULL)
      {
         disk_drive(START_MOTOR_CODE, 0, 0, 0, 0);
         disk_drive(STATUS_MOTOR_CODE, 0, 0, 0, 0);
      }

      for(message_index = 0; message_index < MESSAGE_SIZE; message_index++)
      {
         if(p_fs_message[message_index+1].operation_code != 0)
         {
            sort_requests(p_fs_message);
            printf("\n\nSORTING!!!!!!!!!\n\n");
         }

         printf("REQUESTS: %d", count_requests(p_fs_message));
         
         if(p_fs_message[message_index].operation_code == 1 ||
            p_fs_message[message_index].operation_code == 2)
         {  
            convert_block(p_fs_message[message_index].block_number, &cylinder, &track, &sector);
      
            int counter;
            for(counter = 0; counter < MESSAGE_SIZE; counter++)
            {
               printf("\nOperation Code: %d", p_fs_message[counter].operation_code);
               printf("\nRequest Number: %d", p_fs_message[counter].request_number);
               printf("\nBlock Number  : %d", p_fs_message[counter].block_number);
               printf("\nBlock Size    : %d", p_fs_message[counter].block_size);
               printf("\n\n");
            }

            if(disk_drive(SENSE_CODE, 0, 0, 0, 0) != cylinder)
            {
               printf("\n\nCylinder is: %d", disk_drive(SENSE_CODE, 0, 0, 0, 0));
               printf("\nCylinder should be: %d", cylinder);
               printf("\n\n");
               
               while(status_code = disk_drive(SEEK_CODE, cylinder, 0, 0, 0),
                     status_code != cylinder && status_code != -1)
               {
                  while(disk_drive(RECALIBRATE_CODE, 0, 0, 0, 0) != 0);
               }
            }
            
            if(disk_drive(DMA_SETUP_CODE, sector, track, 
                        p_fs_message[message_index].block_size,
                        p_fs_message[message_index].p_data_address) == 0)
            {
               switch(p_fs_message[message_index].operation_code)
               {
                  case 1:
                     switch(disk_drive(READ_DATA_CODE, 0, 0, 0, 0))
                     {
                        case -1:
                           /* DMA error handling code here */
                           break;
                        case -2:
                           while(disk_drive(READ_DATA_CODE, 0, 0, 0, 0) == -2);
                        case  0:
                           p_fs_message[message_index].operation_code = 0;
                           printf("\n\nGOOD READ\n\n");
                           send_message(p_fs_message);
                           break;
                        default:
                           printf("\n\nREAD ERROR\n\n");
                           break;
                     }
                     break;
                  case 2:
                     switch(disk_drive(WRITE_DATA_CODE, 0, 0, 0, 0))
                     {
                        case -1:
                           /* DMA error handling code here */
                           break;
                        case -2:
                           while(disk_drive(WRITE_DATA_CODE, 0, 0, 0, 0) == -2);
                        case  0:
                           p_fs_message[message_index].operation_code = 0;
                           printf("\n\nGOOD WRITE\n\n");
                           send_message(p_fs_message);
                           break;
                        default:                           
                           printf("\n\nWRITE ERROR\n\n");
                           break;
                     }
                     break;
                  default:
                     printf("\n\nMESSAGE ERROR\n\n");
                     break;
               }
            }
         }
         else
         {
            printf("\n\nEnd of Message\n\n");

            int counter;
            for(counter = 0; counter < MESSAGE_SIZE; counter++)
            {
               printf("\nOperation Code: %d", p_fs_message[counter].operation_code);
               printf("\nRequest Number: %d", p_fs_message[counter].request_number);
               printf("\nBlock Number  : %d", p_fs_message[counter].block_number);
               printf("\nBlock Size    : %d", p_fs_message[counter].block_size);
               printf("\n\n");
            }
            send_message(p_fs_message);
            break;
         }
      }
      
      if(p_fs_message[0].operation_code == 0 &&
         p_fs_message[0].request_number == 0 &&
         p_fs_message[0].block_number   == 0 &&
         p_fs_message[0].block_size     == 0 &&
         p_fs_message[0].p_data_address == NULL)
      {
         disk_drive(STOP_MOTOR_CODE, 0, 0, 0, 0);
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
      *p_sector_number =
            (block_remainder * TRACKS_PER_CYLINDER) - SECTORS_PER_TRACK;
   }

   return;
}

/**********************************************************************/
/*                Count the number of pending requests                */
/**********************************************************************/
int count_requests(MESSAGE *p_fs_message)
{
   int requests; /* Number of pending requests in the message         */

   requests = 0;
   while(requests < MESSAGE_SIZE &&
         p_fs_message[requests].operation_code != 0)
   {
      requests += 1;
   }
   
   return requests;
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
           
   MESSAGE temp_request;  /* Temporary request for swap               */
       int inner_index,   /* Loop control for inner loop              */
           outer_index;   /* Loop control for outer loop              */

   for(outer_index = 0; outer_index < count_requests(p_fs_message);
                                                          outer_index++)
   {
      for(inner_index = 0; inner_index < 
                      (count_requests(p_fs_message) - 1); inner_index++)
      {
         if(p_fs_message[outer_index].block_number < 
                                 p_fs_message[inner_index].block_number)
         {
            temp_request              = p_fs_message[outer_index];
            p_fs_message[outer_index] = p_fs_message[inner_index];
            p_fs_message[inner_index] = temp_request;
         }
      }
   }
      

   return;
}

/**********************************************************************/
/*            Check a request for any incorrect parameters            */
/**********************************************************************/
void check_request(MESSAGE request)
{
   if(check_address       (request) && check_block_size    (request) &&
      check_block_number  (request) && check_request_number(request) &&
      check_operation_code(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK_SIZE +
                               INVALID_BLOCK   + INVALID_REQUEST    +
                               INVALID_OPERATION;
   }
   
   if(check_address     (request) && check_block_size    (request) &&
      check_block_number(request) && check_request_number(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK_SIZE +
                               INVALID_BLOCK   + INVALID_REQUEST;
   }

   if(check_address     (request) && check_block_size    (request) &&
      check_block_number(request) && check_operation_code(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK_SIZE +
                               INVALID_BLOCK   + INVALID_OPERATION;
   }

   if(check_address     (request) && check_block_size    (request) &&
      check_block_number(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK_SIZE +
                               INVALID_BLOCK;
   }

   if(check_address       (request) && check_block_size    (request) &&
      check_request_number(request) && check_operation_code(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK_SIZE +
                               INVALID_REQUEST + INVALID_OPERATION;
   }

   if(check_address       (request) && check_block_size(request) &&
      check_request_number(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK_SIZE +
                               INVALID_REQUEST;
   }

   if(check_address       (request) && check_block_size(request) &&
      check_operation_code(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK_SIZE +
                               INVALID_OPERATION;
   }

   if(check_address(request) && check_block_size(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK_SIZE;
   }

   if(check_address     (request) && check_request_number(request) &&
      check_block_number(request) && check_operation_code(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_REQUEST +
                               INVALID_BLOCK   + INVALID_OPERATION;
   }

   if(check_address     (request) && check_request_number(request) &&
      check_block_number(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_REQUEST +
                               INVALID_BLOCK;
   }

   if(check_address       (request) && check_block_number(request) &&
      check_operation_code(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK +
                               INVALID_OPERATION;
   }

   if(check_address(request) && check_block_number(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK;
   }

   if(check_address       (request) && check_request_number(request) &&
      check_operation_code(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_REQUEST +
                               INVALID_OPERATION;
   }

   if(check_address(request) && check_request_number(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_REQUEST;
   }

   if(check_address(request) && check_operation_code(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_OPERATION;
   }

   if(check_address(request))
   {
      request.operation_code = INVALID_ADDRESS;
   }

   if(check_block_size    (request) && check_block_number  (request) &&
      check_request_number(request) && check_operation_code(request))
   {
      request.operation_code = INVALID_BLOCK_SIZE + INVALID_BLOCK +
                               INVALID_REQUEST    + INVALID_OPERATION;
   }

   if(check_block_size    (request) && check_block_number(request) &&
      check_request_number(request))
   {
      request.operation_code = INVALID_BLOCK_SIZE + INVALID_BLOCK +
                               INVALID_REQUEST;
   }

   if(check_block_size    (request) && check_block_number(request) &&
      check_operation_code(request))
   {
      request.operation_code = INVALID_BLOCK_SIZE + INVALID_BLOCK +
                               INVALID_OPERATION;
   }

   if(check_block_size(request) && check_block_number(request))
   {
      request.operation_code = INVALID_BLOCK_SIZE + INVALID_BLOCK;
   }

   if(check_block_size    (request) && check_request_number(request) &&
      check_operation_code(request))
   {
      request.operation_code = INVALID_BLOCK_SIZE + INVALID_REQUEST +
                               INVALID_OPERATION;
   }

   if(check_block_size(request) && check_request_number(request))
   {
      request.operation_code = INVALID_BLOCK_SIZE + INVALID_REQUEST;
   }

   if(check_block_size(request) && check_operation_code(request))
   {
      request.operation_code = INVALID_BLOCK_SIZE + INVALID_OPERATION;
   }

   if(check_block_size(request))
   {
      request.operation_code = INVALID_BLOCK_SIZE;
   }

   if(check_block_number  (request) && check_request_number(request) &&
      check_operation_code(request))
   {
      request.operation_code = INVALID_BLOCK + INVALID_REQUEST +
                               INVALID_OPERATION;
   }

   if(check_block_number(request) && check_request_number(request))
   {
      request.operation_code = INVALID_BLOCK + INVALID_REQUEST;
   }

   if(check_block_number(request) &&check_operation_code(request))
   {
      request.operation_code = INVALID_BLOCK + INVALID_OPERATION;
   }

   if(check_block_number(request))
   {
      request.operation_code = INVALID_BLOCK;
   }

   if(check_request_number(request) && check_operation_code(request))
   {
      request.operation_code = INVALID_REQUEST + INVALID_OPERATION;
   }

   if(check_request_number(request))
   {
      request.operation_code = INVALID_REQUEST;
   }

   if(check_operation_code(request))
   {
      request.operation_code = INVALID_OPERATION;
   }

   return;
}

/**********************************************************************/
/*             Check a request for invalid operation code             */
/**********************************************************************/
int check_operation_code(MESSAGE request)
{   
   return (request.operation_code != 1 || request.operation_code != 2);
}

/**********************************************************************/
/*             Check a request for invalid request number             */
/**********************************************************************/
int check_request_number(MESSAGE request)
{
   return (request.request_number <= 0);
}

/**********************************************************************/
/*              Check a request for invalid block number              */
/**********************************************************************/
int check_block_number(MESSAGE request)
{
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
   return (request.p_data_address < 0);
}