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
#include <stdlib.h>

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
void convert_block(int block_number);
    /* Convert block number to cylinder, track, and sector numbers    */

/**********************************************************************/
/*                            Main Function                           */
/**********************************************************************/
int main()
{

    return 0;
}

/**********************************************************************/
/*     Convert block number to cylinder, track, and sector numbers    */
/**********************************************************************/
void convert_block(int block_number)
{

    return
}


