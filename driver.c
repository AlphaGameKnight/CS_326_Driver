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
/*                         Function Prototypes                        */
/**********************************************************************/
void disk_drive(int code, int arg1, int arg2, int arg3, int arg4);
   /* Run the disk driver                                             */
//void senseCylinder();
//void seekToCylinder();
//void dmaSetup();
//void startMotor();
//void statusMotor();
//void readData();
//void writeData();
//void stopMotor();
//void recalibrate();


/**********************************************************************/
/*                           Main Function                            */
/**********************************************************************/
int main()
{
   
   
   return 0;
}

/**********************************************************************/
/*                         Run the DISK driver                        */
/**********************************************************************/
void disk_drive(int code, int arg1, int arg2, int arg3, int arg4)
{
   switch(code)
   {
      case SENSE_CODE:
         // sense_cylinder code
         break;
      case SEEK_CODE:
         // seek_cylinder code
         break;
      case DMA_SETUP_CODE:
         // dma_setup code
         break;
      case START_MOTOR_CODE:
         // start_motor code
         break;
      case STATUS_MOTOR_CODE:
         // status_motor code
         break;
      case READ_DATA_CODE:
         // read_data code
         break;
      case WRITE_DATA_CODE:
         // write_data code
         break;
      case STOP_MOTOR_CODE:
         // stop_motor code
         break;
      case RECALIBRATE_CODE:
         // recalibrate code
         break;
   }

   return;
}