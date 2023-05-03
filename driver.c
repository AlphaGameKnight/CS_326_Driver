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
void send_message(MESSAGE *fs_message);
   /* Send a message to the FILE SYSTEM                               */
int  disk_drive(int code, int arg1, int arg2, int arg3, 
                                               unsigned long int *arg4);
   /* Tell the DISK drive to perform some function                    */
void convert_block(int block_number,    int *p_cylinder_number,
                   int *p_track_number, int *p_sector_number);
   /* Convert a block number to cylinder, track, and sector numbers   */
int count_requests(MESSAGE *fs_message);
   /* Count the number of pending requests                            */
void sort_requests(MESSAGE *fs_message);
   /* Sort requests within the message by ascending block number      */
void check_request(MESSAGE request, MESSAGE *fs_message);
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
void schedule_disk(MESSAGE *fs_message);
   /* Schedule the next request for the driver to work on             */


/**********************************************************************/
/*                           Main Function                            */
/**********************************************************************/
int main()
{
   MESSAGE *fs_message; /* Points to message sent to/from FILE SYSTEM */
   int cylinder,        /* Cylinder number                            */
       drive_status,    /* Flag for the status of the disk drive      */
       idle_counter,    /* Counter for number of idle requests        */
       index,           /* Index for a request in the message         */
       sector,          /* Sector number                              */
       track;           /* Track number                               */

   /* Create an empty list of pending requests to send to FILE SYSTEM */
   fs_message = (MESSAGE *) malloc(MESSAGE_SIZE * sizeof(MESSAGE));

   /* Send an idle message to the FILE SYSTEM, requesting work        */
   send_message(fs_message);
   drive_status = 0;
   idle_counter = 0;

   /* Loop continuously processing the pending requests list          */
   while(true)
   {
      if(idle_counter == 0 && 
         drive_status == 0 &&
         fs_message[0].operation_code != 0)
      {
         drive_status = disk_drive(START_MOTOR_CODE, 0, 0, 0, 0);
         disk_drive(STATUS_MOTOR_CODE, 0, 0, 0, 0);
         drive_status = -1;
      }

      /* Handle an idle requests                                      */
      if(fs_message[0].operation_code == 0 &&
         fs_message[0].request_number == 0 &&
         fs_message[0].block_number   == 0 &&
         fs_message[0].block_size     == 0)
      {
         idle_counter += 1;
         if(idle_counter == 2)
         {
            send_message(fs_message);
            if(drive_status == -1)
            {  
               disk_drive(STOP_MOTOR_CODE, 0, 0, 0, 0);
               
               drive_status = 0;
               idle_counter = 0;
            }
            else
            {
               if(drive_status == 0)
               {
                  idle_counter = 0;
               }
            }
         }
         continue;
      }
      else
      {
         
         printf("\nOperation Code: %d", fs_message[0].operation_code);
         printf("\nRequest Number: %d", fs_message[0].request_number);
         printf("\nBlock Number  : %d", fs_message[0].block_number);
         printf("\nBlock Size    : %d", fs_message[0].block_size);
         printf("\n\n");

         int oof;
            printf("\n\nBefore sorting222:");
            for(oof = 0; oof < MESSAGE_SIZE; oof++)
            {
               printf("\nRequest Number: %d", fs_message[oof].request_number);
            }
            printf("\n\n");
            
            sort_requests(fs_message);
            
            /*check_request(fs_message[index], fs_message);*/
            int counter;
            printf("\n\nAfter sorting:");
            for(counter = 0; counter < MESSAGE_SIZE; counter++)
            {
               printf("\nRequest Number: %d", fs_message[counter].request_number);
            }
            printf("\n\n");
         
         
         /* Process each individual request in the list                  */
         for(index = 0; index < MESSAGE_SIZE; index++)
         {           
            sort_requests(fs_message);

            printf("\n\nAfter sorting:");
            for(oof = 0; oof < MESSAGE_SIZE; oof++)
            {
               printf("\nRequest Number: %d", fs_message[oof].request_number);
            }
            printf("\n\n");

            int counter;
            printf("\n\nList:");
            for(counter = 0; counter < MESSAGE_SIZE; counter++)
            {
               printf("\nRequest Number: %d", fs_message[counter].request_number);
            }
            printf("\n\n");

            printf("\nindex: %d", index);

            if((fs_message[index].operation_code == 1  ||
                fs_message[index].operation_code == 2) &&
                fs_message[0].operation_code != 0)
            {            
               convert_block(fs_message[index].block_number, &cylinder,
                                                         &track, &sector);

               /* Seek for the correct cylinder the request is at        */
               if(disk_drive(SENSE_CODE, 0, 0, 0, 0) != cylinder)
               {
                  printf("\n\nCylinder is: %d", disk_drive(SENSE_CODE, 0, 0, 0, 0));
                  printf("\nCylinder should be: %d", cylinder);
                  printf("\n\n");

                  while(disk_drive(SEEK_CODE, cylinder, 0, 0, 0) != cylinder)
                  {
                     while(disk_drive(RECALIBRATE_CODE, 0, 0, 0, 0) != 0);
                  }
               }

               /* Set DMA chip registers for the next read/write job    */
               disk_drive(DMA_SETUP_CODE, sector, track,
                          fs_message[index].block_size,
                          fs_message[index].p_data_address);
               
               if(fs_message[index].operation_code == 1)
               {
                  /* Perform read request and fix checksum errors      */  
                  while(disk_drive(READ_DATA_CODE, 0, 0, 0, 0) != 0);
                  fs_message[index].operation_code = 0;
                  
                  
                  

                  int oof;
                  printf("\n\nList before sending:");
                  for(oof = 0; oof < MESSAGE_SIZE; oof++)
                  {
                     printf("\nRequest Number: %d", fs_message[oof].request_number);
                  }
                  printf("\n\n");

                  send_message(fs_message);
                  

                  if(fs_message[index].request_number == 0 &&
                     index == 0)
                  {
                     printf("\n\nVerified!\n\n");
                     sort_requests(fs_message);
                  }

                  break;
               }
               else
               {
                  /* Perform write request and fix checksum errors     */
                  while(disk_drive(WRITE_DATA_CODE, 0, 0, 0, 0) != 0);
                  fs_message[index].operation_code = 0;
                  
                  printf("\n\nList before sending333:");
                  for(oof = 0; oof < MESSAGE_SIZE; oof++)
                  {
                     printf("\nRequest Number: %d", fs_message[oof].request_number);
                  }
                  printf("\n\n");

                  send_message(fs_message);
                  
                  printf("\n\nList after sending:");
                  for(oof = 0; oof < MESSAGE_SIZE; oof++)
                  {
                     printf("\nRequest Number: %d", fs_message[oof].request_number);
                  }
                  printf("\n\n");

                  if(fs_message[index].request_number == 0 &&
                     index == 0)
                  {
                     printf("\n\nVerified 2!\n\n");
                     sort_requests(fs_message);
                  }

                  

                  break;
               }
            }
            else
            {
               if(fs_message[0].operation_code == 0)
               {
                  index = 0;
               }
               else
               {
                  break;
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
      *p_sector_number =
            (block_remainder * TRACKS_PER_CYLINDER) - SECTORS_PER_TRACK;
   }

   return;
}

/**********************************************************************/
/*                Count the number of pending requests                */
/**********************************************************************/
int count_requests(MESSAGE *fs_message)
{
   int requests; /* Number of pending requests in the message         */

   requests = 0;
   while(requests < MESSAGE_SIZE &&
         fs_message[requests].operation_code != 0 &&
         fs_message[requests+1].operation_code != 0)
   {
      requests += 1;
   }
   
   return requests;
}

/**********************************************************************/
/*     Sort requests within the message by ascending block number     */
/**********************************************************************/
void sort_requests(MESSAGE *fs_message)
{        
   MESSAGE temp_request; /* Temporary request for swap               */
       int index;        /* Index for a request in the message       */

   index = 0;
   while(index < MESSAGE_SIZE)
   {
      if(((fs_message[index].block_number > 
           fs_message[index+1].block_number) &&
          (fs_message[index+1].block_number != 0)) ||
          (fs_message[index].block_number == 0))
      {
         temp_request        = fs_message[index];
         fs_message[index]   = fs_message[index+1];
         fs_message[index+1] = temp_request;
      }
      
      index++;
   }

   return;
}

/**********************************************************************/
/*            Check a request for any incorrect parameters            */
/**********************************************************************/
void check_request(MESSAGE request, MESSAGE *fs_message)
{
   if(check_address       (request) && check_block_size    (request) &&
      check_block_number  (request) && check_request_number(request) &&
      check_operation_code(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK_SIZE +
                               INVALID_BLOCK   + INVALID_REQUEST    +
                               INVALID_OPERATION;

      send_message(fs_message);
   }
   
   if(check_address     (request) && check_block_size    (request) &&
      check_block_number(request) && check_request_number(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK_SIZE +
                               INVALID_BLOCK   + INVALID_REQUEST;
      
      send_message(fs_message);
   }

   if(check_address     (request) && check_block_size    (request) &&
      check_block_number(request) && check_operation_code(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK_SIZE +
                               INVALID_BLOCK   + INVALID_OPERATION;
      
      send_message(fs_message);
   }

   if(check_address     (request) && check_block_size    (request) &&
      check_block_number(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK_SIZE +
                               INVALID_BLOCK;
      
      send_message(fs_message);
   }

   if(check_address       (request) && check_block_size    (request) &&
      check_request_number(request) && check_operation_code(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK_SIZE +
                               INVALID_REQUEST + INVALID_OPERATION;
      
      send_message(fs_message);
   }

   if(check_address       (request) && check_block_size(request) &&
      check_request_number(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK_SIZE +
                               INVALID_REQUEST;
      
      send_message(fs_message);
   }

   if(check_address       (request) && check_block_size(request) &&
      check_operation_code(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK_SIZE +
                               INVALID_OPERATION;
      
      send_message(fs_message);
   }

   if(check_address(request) && check_block_size(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK_SIZE;
      
      send_message(fs_message);
   }

   if(check_address     (request) && check_request_number(request) &&
      check_block_number(request) && check_operation_code(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_REQUEST +
                               INVALID_BLOCK   + INVALID_OPERATION;
      
      send_message(fs_message);
   }

   if(check_address     (request) && check_request_number(request) &&
      check_block_number(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_REQUEST +
                               INVALID_BLOCK;
      
      send_message(fs_message);
   }

   if(check_address       (request) && check_block_number(request) &&
      check_operation_code(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK +
                               INVALID_OPERATION;
      
      send_message(fs_message);
   }

   if(check_address(request) && check_block_number(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_BLOCK;
      
      send_message(fs_message);
   }

   if(check_address       (request) && check_request_number(request) &&
      check_operation_code(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_REQUEST +
                               INVALID_OPERATION;
      
      send_message(fs_message);
   }

   if(check_address(request) && check_request_number(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_REQUEST;
      
      send_message(fs_message);
   }

   if(check_address(request) && check_operation_code(request))
   {
      request.operation_code = INVALID_ADDRESS + INVALID_OPERATION;
      
      send_message(fs_message);
   }

   if(check_address(request))
   {
      request.operation_code = INVALID_ADDRESS;
      
      send_message(fs_message);
   }

   if(check_block_size    (request) && check_block_number  (request) &&
      check_request_number(request) && check_operation_code(request))
   {
      request.operation_code = INVALID_BLOCK_SIZE + INVALID_BLOCK +
                               INVALID_REQUEST    + INVALID_OPERATION;
      
      send_message(fs_message);
   }

   if(check_block_size    (request) && check_block_number(request) &&
      check_request_number(request))
   {
      request.operation_code = INVALID_BLOCK_SIZE + INVALID_BLOCK +
                               INVALID_REQUEST;
      
      send_message(fs_message);
   }

   if(check_block_size    (request) && check_block_number(request) &&
      check_operation_code(request))
   {
      request.operation_code = INVALID_BLOCK_SIZE + INVALID_BLOCK +
                               INVALID_OPERATION;
      
      send_message(fs_message);
   }

   if(check_block_size(request) && check_block_number(request))
   {
      request.operation_code = INVALID_BLOCK_SIZE + INVALID_BLOCK;
      
      send_message(fs_message);
   }

   if(check_block_size    (request) && check_request_number(request) &&
      check_operation_code(request))
   {
      request.operation_code = INVALID_BLOCK_SIZE + INVALID_REQUEST +
                               INVALID_OPERATION;
      
      send_message(fs_message);
   }

   if(check_block_size(request) && check_request_number(request))
   {
      request.operation_code = INVALID_BLOCK_SIZE + INVALID_REQUEST;
      
      send_message(fs_message);
   }

   if(check_block_size(request) && check_operation_code(request))
   {
      request.operation_code = INVALID_BLOCK_SIZE + INVALID_OPERATION;
      
      send_message(fs_message);
   }

   if(check_block_size(request))
   {
      request.operation_code = INVALID_BLOCK_SIZE;
      
      send_message(fs_message);
   }

   if(check_block_number  (request) && check_request_number(request) &&
      check_operation_code(request))
   {
      request.operation_code = INVALID_BLOCK + INVALID_REQUEST +
                               INVALID_OPERATION;
      
      send_message(fs_message);
   }

   if(check_block_number(request) && check_request_number(request))
   {
      request.operation_code = INVALID_BLOCK + INVALID_REQUEST;
      
      send_message(fs_message);
   }

   if(check_block_number(request) &&check_operation_code(request))
   {
      request.operation_code = INVALID_BLOCK + INVALID_OPERATION;
      
      send_message(fs_message);
   }

   if(check_block_number(request))
   {
      request.operation_code = INVALID_BLOCK;
      
      send_message(fs_message);
   }

   if(check_request_number(request) && check_operation_code(request))
   {
      request.operation_code = INVALID_REQUEST + INVALID_OPERATION;
      
      send_message(fs_message);
   }

   if(check_request_number(request))
   {
      request.operation_code = INVALID_REQUEST;
      
      send_message(fs_message);
   }

   if(check_operation_code(request))
   {
      request.operation_code = INVALID_OPERATION;
      
      send_message(fs_message);
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

/**********************************************************************/
/*         Schedule the next request for the driver to work on        */
/**********************************************************************/
void schedule_disk(MESSAGE *fs_message)
{
   int cylinder,        /* Cylinder number                            */
       sector,          /* Sector number                              */
       track;           /* Track number                               */
   
   convert_block(fs_message[1].block_number, &cylinder, &track, &sector);

   if(disk_drive(SENSE_CODE, 0, 0, 0, 0) < cylinder)
   {
      while(disk_drive(SEEK_CODE, 0, 0, 0, 0) != cylinder)
      {
         while(disk_drive(RECALIBRATE_CODE, 0, 0, 0, 0) != 0);
      }
   }

   return;
}