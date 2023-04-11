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
/* corresponding cylinder, track, and sector numbers.                 */
/*                                                                    */
/**********************************************************************/

#include <stdio.h>
#include <math.h>

/**********************************************************************/
/*                         Symbolic Constants                         */
/**********************************************************************/
#define BYTES_PER_BLOCK     1024
#define BYTES_PER_SECTOR    512
#define CYLINDERS_PER_DISK  40
#define SECTORS_PER_TRACK   9
#define TRACKS_PER_CYLINDER 2

/**********************************************************************/
/*                         Function Prototypes                        */
/**********************************************************************/
void convert_block(int block_number,    int *p_cylinder_number,
                   int *p_track_number, int *p_sector_number);
    /* Convert block number to cylinder, track, and sector numbers    */

/**********************************************************************/
/*                            Main Function                           */
/**********************************************************************/
int main()
{
   int cylinder_number, /* Cylinder number of the associated block */
       track_number,    /* Track number of the associated block    */
       sector_number,   /* Sector number of the associated block   */
       counter;         /* Counts through all 360 disk blocks      */

   printf("Block   Cylinder   Track   Sector\n");
   printf("-----   --------   -----   ------\n");
   
   for (counter = 1; counter <= 360; counter++)
   {
      convert_block(counter, &cylinder_number, &track_number, &sector_number);
      printf("%3d         %d        %d        %d\n", counter, cylinder_number,
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
   /* Calculate the cylinder number from the associated block number  */
   *p_cylinder_number = (int) ((block_number-1)/9.0f);

   /* Calculate the track number from the associated block number     */
   if ((block_number-1)%9 < 5)
   {
      *p_track_number    = 0;
   }
   else
   {
      *p_track_number    = 1;
   }

   /* Calculate the sector number from the associated block number    */
   if (((block_number-1)%9 * 2) < 9)
   {
      *p_sector_number = ((block_number-1)%9) * 2;
   }
   else
   {
      *p_sector_number = ((((block_number-1)%9) * 2) - 9);
   }

   return;
}


