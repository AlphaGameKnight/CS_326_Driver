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
/* This program converts File System blocks numbers into their        */
/* corresponding cylinder, track, and sector numbers and displays the */
/* the block number with the converted values.                        */
/*                                                                    */
/**********************************************************************/

#include <stdio.h>

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
   int block,     /* Block number                                      */
       cylinder,  /* Cylinder number                                   */
       max_block, /* Maximum block number for the disk drive           */
       sector,    /* Sector number                                     */
       track;     /* Track number                                      */
       
   max_block =
            (CYLINDERS_PER_DISK * TRACKS_PER_CYLINDER * SECTORS_PER_TRACK *
             BYTES_PER_SECTOR / BYTES_PER_BLOCK);

   /* Print the heading lines                                          */
   printf("     Block   Cylinder   Track   Sector");
   printf("\n     -----   --------   -----   ------");
   
   /* Loop processing block numbers until it reaches end of disk drive */
   for (block = 1; block <= max_block; block++)
   {
      /* Convert a block number to its cylinder, track, and sector     */
      /* numbers and display them in a table                           */
      convert_block(block, &cylinder, &track, &sector);
      printf("\n     %3d        %2d        %d        %d", block,
         cylinder, track, sector);
   }
   printf("\n");

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