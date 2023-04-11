/**********************************************************************/
/*                                                                    */
/* Program Name: convert - Convert File System block numbers          */
/* Author:       William S. Wu                                        */
/* Installation: Pensacola Christian College, Pensacola, Florida      */
/* Course:       CS326, Operating Systems                             */
/* Date Written: April 9, 2023                                        */
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
/* This program converts 360 File System blocks into their            */
/* corresponding cylinder, track, and sector numbers and displays the */
/* the block number with the converted values.                        */
/*                                                                    */
/**********************************************************************/

#include <stdio.h>
#include <math.h>

/**********************************************************************/
/*                         Symbolic Constants                         */
/**********************************************************************/
#define BYTES_PER_BLOCK     1024 /* Number of bytes in a block        */
#define BYTES_PER_SECTOR    512  /* Number of bytes in a sector       */
#define CYLINDERS_PER_DISK  40   /* Number of cylinders in a disk     */
#define SECTORS_PER_TRACK   9    /* Number of sectors in a track      */
#define TRACKS_PER_CYLINDER 2    /* Number of tracks in a cylinder    */

/**********************************************************************/
/*                         Function Prototypes                        */
/**********************************************************************/
void convert_block(int block_number,    int *p_cylinder_number,
                   int *p_track_number, int *p_sector_number);
   /* Convert a block number to cylinder, track, and sector numbers   */

/**********************************************************************/
/*                            Main Function                           */
/**********************************************************************/
int main()
{
   int cylinder_number, /* Cylinder number of the associated block    */
       track_number,    /* Track number of the associated block       */
       sector_number,   /* Sector number of the associated block      */
       counter;         /* Counts through all 360 disk blocks         */

   printf("Block   Cylinder   Track   Sector\n");
   printf("-----   --------   -----   ------\n");
   
   for (counter = 1; counter <= 360; counter++)
   {
      convert_block(counter, &cylinder_number, &track_number, &sector_number);
      printf("%3d        %2d        %d        %d\n", counter, cylinder_number,
         track_number, sector_number);
   }

   return 0;
}

/**********************************************************************/
/*     Convert block number to cylinder, track, and sector numbers    */
/**********************************************************************/
void convert_block(int block_number,    int *p_cylinder_number,
                   int *p_track_number, int *p_sector_number)
{  
   float sectors_per_block,
         cylinders_per_sector,
         cylinders_per_block;
   
   sectors_per_block    = BYTES_PER_BLOCK / BYTES_PER_SECTOR;
   cylinders_per_sector =
                  (1.0f/SECTORS_PER_TRACK) * (1.0f/TRACKS_PER_CYLINDER);
   cylinders_per_block  = cylinders_per_sector * sectors_per_block;
   
   /* Calculate the cylinder number from the associated block number  */
   *p_cylinder_number = (block_number - 1) * cylinders_per_block;

   /* Calculate the track number from the associated block number     */
   if ((block_number - 1) % (int) (1.0f/cylinders_per_block) <
                            (int) ((1.0f/cylinders_per_block/2.0f) + 0.5f))
   {
      *p_track_number = 0;
   }
   else
   {
      *p_track_number = 1;
   }

   /* Calculate the sector number from the associated block number    */
   *p_sector_number   = 
        (block_number - 1) * (int)sectors_per_block % SECTORS_PER_TRACK;

   return;
}


