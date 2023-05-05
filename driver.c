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
/* cylinder, track, and sector numbers, then instructs a DISK device  */
/* to carry out the read and write requests, sending completed tasks  */
/* back to the FILE SYSTEM and fixing any checksum or seek errors.    */
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
#define MOTOR_OFF           0    /* Status of motor when off          */
#define IDLE_MOTOR_OFF      2    /* Code indicating DISK motor is off */
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
                 int operation_code, /* Operation to be performed     */
                     request_number, /* A unique request number       */
                     block_number,   /* Block to be read or written   */
                     block_size;     /* Block size in bytes           */
   unsigned long int *p_data_address;
                                     /* Points to a memory data block */
};
typedef struct message MESSAGE;

/**********************************************************************/
/*                         Function Prototypes                        */
/**********************************************************************/

void send_message(MESSAGE *p_request_list);
   /* Send a message to the FILE SYSTEM from the driver               */
void create_idle_request(MESSAGE *fs_message);
   /* Create an idle request for the FILE SYSTEM to request work      */
void populate_driver_list(MESSAGE *fs_message,
                          MESSAGE *pending_requests, int *p_requests);
   /* Populate driver's requests list with FILE SYSTEM requests       */
void populate_message(MESSAGE *fs_message, MESSAGE *pending_requests);
   /* Populate message for FILE SYSTEM with completed request         */
int  disk_drive(int code, int arg1, int arg2, int arg3, 
                                               unsigned long int *arg4);
   /* Tell the DISK drive to perform some function                    */
void convert_block(int block_number,    int *p_cylinder_number,
                   int *p_track_number, int *p_sector_number);
   /* Convert a block number to cylinder, track, and sector numbers   */
int count_requests(MESSAGE *p_request_list);
   /* Count the number of pending requests                            */
void sort_requests(MESSAGE *p_request_list, int *p_requests);
   /* Sort requests within the message by ascending block number      */
void remove_finished_request(MESSAGE *p_request_list);
   /* Remove a finished request from the driver's pending requst list */
int check_request(MESSAGE request);
   /* Check a pending request for any incorrect parameters            */
void clear_requests(MESSAGE *p_request_list);
   /* Clear requests from a request list                              */
void clear_first_request(MESSAGE *p_request_list, int *p_requests);
   /* Clear data from a completed request in the pending list         */
void reset_lists(MESSAGE *p_request_list, MESSAGE *p_message_list,
                                                       int *p_requests);
   /* Resets the pending requests lists for more requests             */

/**********************************************************************/
/*                           Main Function                            */
/**********************************************************************/
int main()
{
   MESSAGE fs_message[MESSAGE_SIZE],
                           /* Message of requests sent to FILE SYSTEM */
           pending_requests[MESSAGE_SIZE];
                           /* Driver's pending requests list          */
   int     cylinder,       /* Cylinder number                         */
           drive_status,   /* Flag for the status of the disk drive   */
           error_number,   /* Indicates invalid request paramters     */
           idle_counter,   /* Counter for number of idle requests     */
           sector,         /* Sector number                           */
           total_requests, /* Total number of pending requests        */
           track;          /* Track number                            */

   drive_status   = 0;
   idle_counter   = 0;
   total_requests = 0;

   /* Initialize the FILE SYSTEM message and the driver's pending     */
   /* requests list; create an idle request and send to FILE SYSTEM   */
   clear_requests(fs_message);
   clear_requests(pending_requests);
   create_idle_request(fs_message);
   send_message(fs_message);

   /* Loop continuously processing pending requests from FILE SYSTEM  */
   while(true)
   {    
      /* Populate the driver's pending request list with FILE SYSTEM  */
      /* requests                                                     */
      populate_driver_list(fs_message, pending_requests, &total_requests);

      /* Send idle message to FILE SYSTEM if no pending requests      */
      if(total_requests == 0)
      {
         create_idle_request(fs_message);
         idle_counter += 1;
         
         /* Turn motor off if number of idle requests is sufficient   */
         if(idle_counter == IDLE_MOTOR_OFF)
         {
            send_message(fs_message);
            
            if(drive_status != MOTOR_OFF)
            {
               disk_drive(STOP_MOTOR_CODE, 0, 0, 0, 0);
               drive_status = MOTOR_OFF;
            }
            idle_counter = 0;
         }
      }
      else
      {
         /* Turn motor on and verify it is up to speed                */
         if(drive_status == MOTOR_OFF)
         {
            drive_status = disk_drive(START_MOTOR_CODE, 0, 0, 0, 0);
            disk_drive(STATUS_MOTOR_CODE, 0, 0, 0, 0);
            idle_counter = 0;
         }
         
         /* Sort driver's pending requests list in ascending block    */
         /* number                                                    */
         sort_requests(pending_requests, &total_requests);

         /* Check if the current request has invalid paramters        */
         error_number = check_request(pending_requests[0]);
         if(error_number == 0)
         {
            
            /* Convert a request's block number to its corresponding  */
            /* cylinder, track, and sector numbers                    */
            convert_block(pending_requests[0].block_number,
                                             &cylinder, &track, &sector);

            /* Seek for the cylinder the request is located in        */
            if(disk_drive(SENSE_CODE, 0, 0, 0, 0) != cylinder)
            {
               while(disk_drive(SEEK_CODE, cylinder, 0, 0, 0) != cylinder)
               {
                  while(disk_drive(RECALIBRATE_CODE, 0, 0, 0, 0) != 0);
               }
            }
            
            /* Set DMA chip registers for the next read/write job     */
            disk_drive(DMA_SETUP_CODE, sector, track,
                        pending_requests[0].block_size,
                        pending_requests[0].p_data_address);
            
            if(pending_requests[0].operation_code == 1)
            {
               /* Perform read request and fix checksum errors        */  
               while(disk_drive(READ_DATA_CODE, 0, 0, 0, 0) != 0);
               pending_requests[0].operation_code = 0;
               reset_lists(pending_requests, fs_message, &total_requests);
            }
            else
            {
               /* Perform write request and fix checksum errors       */  
               while(disk_drive(WRITE_DATA_CODE, 0, 0, 0, 0) != 0);
               pending_requests[0].operation_code = 0;
               reset_lists(pending_requests, fs_message, &total_requests);
            }
         }
         else
         {
            /* Send the request with incorrect parameters back to the */
            /* FILE SYSTEM                                            */
            pending_requests[0].operation_code = error_number;
            reset_lists(pending_requests, fs_message, &total_requests);
         }
      }
   }
   
   return 0;
}

/**********************************************************************/
/*     Create an idle request for the FILE SYSTEM to request work     */
/**********************************************************************/
void create_idle_request(MESSAGE *p_message_list)
{
   p_message_list[0].operation_code = 0;
   p_message_list[0].request_number = 0;
   p_message_list[0].block_number   = 0;
   p_message_list[0].block_size     = 0;
   p_message_list[0].p_data_address = NULL;

   return;
}

/**********************************************************************/
/*   Add requests to driver's pending requests list from FILE SYSTEM  */
/**********************************************************************/
void populate_driver_list(MESSAGE *p_message_list,
                          MESSAGE *p_request_list, int *p_requests)
{
   int index; /* Index for a request in the pending requests list     */

   index = 0;

   while((index < MESSAGE_SIZE) &&
         (p_message_list[index].operation_code != 0))
   {
      p_request_list[*p_requests] = p_message_list[index];
      *p_requests += 1;
      index++;
   }
 
   return;
}

/**********************************************************************/
/*      Populate message for FILE SYSTEM with completed requests      */
/**********************************************************************/
void populate_message(MESSAGE *p_message_list, MESSAGE *p_request_list)
{    
   p_message_list[0].operation_code = p_request_list[0].operation_code;
   p_message_list[0].request_number = p_request_list[0].request_number;
   p_message_list[0].block_number   = p_request_list[0].block_number;
   p_message_list[0].block_size     = p_request_list[0].block_size;
   p_message_list[0].p_data_address = p_request_list[0].p_data_address;

   return;
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
int count_requests(MESSAGE *p_request_list)
{
   int index,    /* Index for a request in the driver's list          */
       requests; /* Number of pending requests in the message         */

   index    = 0,
   requests = 0;
   
   while(index < MESSAGE_SIZE && p_request_list[index].operation_code != 0)
   {
      requests += 1;
      index++;
   }
   
   return requests;
}

/**********************************************************************/
/*           Sort pending requests by ascending block number          */
/**********************************************************************/
void sort_requests(MESSAGE *p_request_list, int *p_requests)
{        
   MESSAGE temp_request; /* Temporary request for swap               */
       int inner_index,  /* Index of a request for inner loop        */
           outer_index;  /* Index of a request for outer loop        */           
   
   for(outer_index = 0; outer_index < *p_requests; outer_index++)
   {
      for(inner_index = 0; inner_index < *p_requests - outer_index - 1;
                                                         inner_index++)
      {
         if((p_request_list[inner_index].block_number >
             p_request_list[inner_index+1].block_number) &&
            (p_request_list[inner_index+1].block_number != 0))
         {
            temp_request = p_request_list[inner_index];
            p_request_list[inner_index] 
                         = p_request_list[inner_index + 1];
            p_request_list[inner_index + 1]
                         = temp_request;
         }
      }
   }   

   return;
}

/**********************************************************************/
/*   Remove a finished request from the driver's pending requst list  */
/**********************************************************************/
void remove_finished_request(MESSAGE *p_request_list)
{
   MESSAGE temp_request; /* Temporary request for swap                */
   int     index;        /* Index for a request in the driver's list  */

   for(index = 0; index < MESSAGE_SIZE; index++)
   {
      if(p_request_list[index].operation_code == 0 &&
         p_request_list[index].request_number == 0 &&
         p_request_list[index].block_number   == 0 &&
         p_request_list[index].block_size     == 0 &&
         p_request_list[index].p_data_address == NULL &&
         p_request_list[index + 1].operation_code != 0)
      {
         temp_request              = p_request_list[index];
         p_request_list[index]     = p_request_list[index + 1];
         p_request_list[index + 1] = temp_request; 
      }
   }

   return;
}

/**********************************************************************/
/*            Check a request for any incorrect parameters            */
/**********************************************************************/
int check_request(MESSAGE request)
{
   int bytes_per_cylinder, /* Number of bytes in a cylinder           */
       error_number;       /* Number indicating invalid request       */
                           /* parameters                              */
   
   error_number       = 0;
   bytes_per_cylinder = BYTES_PER_SECTOR * SECTORS_PER_TRACK *
                        TRACKS_PER_CYLINDER;

   if(request.operation_code != 1 &&
      request.operation_code != 2)
   {
      error_number += INVALID_OPERATION;
   }

   if(request.request_number <= 0)
   {
      error_number += INVALID_REQUEST;
   }

   if(request.block_number < 1 || request.block_number > 360)
   {
      error_number += INVALID_BLOCK;
   }

   if(request.block_size < 0 || (double) request.block_size ==
      log10(request.block_size) / log10(2.0) ||
      request.block_size > bytes_per_cylinder)
   {
      error_number += INVALID_BLOCK_SIZE;
   }

   if(request.p_data_address < 0)
   {
      error_number += INVALID_ADDRESS;
   }

   return error_number;
}

/**********************************************************************/
/*                 Clear requests from a request list                 */
/**********************************************************************/
void clear_requests(MESSAGE *p_request_list)
{
   int index; /* Index for a request in a pending request list        */
   
   for(index = 0; index < MESSAGE_SIZE; index++)
   {
      p_request_list[index].operation_code = 0;
      p_request_list[index].request_number = 0;
      p_request_list[index].block_number   = 0;
      p_request_list[index].block_size     = 0;
      p_request_list[index].p_data_address = NULL;
   }

   return;
}

/**********************************************************************/
/*       Clear data from a completed request in the pending list      */
/**********************************************************************/
void clear_first_request(MESSAGE *p_request_list, int *p_requests)
{
   p_request_list[0].operation_code = 0;
   p_request_list[0].request_number = 0;
   p_request_list[0].block_number   = 0;
   p_request_list[0].block_size     = 0;
   p_request_list[0].p_data_address = NULL;
   *p_requests -= 1;

   return;
}

/**********************************************************************/
/*         Resets the pending requests lists for more requests        */
/**********************************************************************/
void reset_lists(MESSAGE *p_request_list, MESSAGE *p_message_list,
                                                        int *p_requests)
{
   clear_requests(p_message_list);
   populate_message(p_message_list, p_request_list);
   send_message(p_message_list);
   clear_first_request(p_request_list, p_requests);
   remove_finished_request(p_message_list);
   remove_finished_request(p_request_list);
   sort_requests(p_message_list, p_requests);

   return;
}