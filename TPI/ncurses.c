#include <ncurses.h>			/* ncurses.h includes stdio.h */  
#include <string.h> 
#include <stdlib.h>
 
int main()
{
 char mesg[]="Enter a string: ";		/* message to be appeared on the screen */
 char str[80];
 int row,col;				/* to store the number of rows and *
					 * the number of colums of the screen */
char *b1 = malloc(col+1);
memset(b1, '*', col);
b1[col] = '\0';
char *b2 = malloc(row+1);
memset(b2, '*', row);
b2[row] = '\0';
 initscr();				/* start the curses mode */
 getmaxyx(stdscr,row,col);		/* get the number of rows and columns */
 mvprintw(0,0,"%s",b1);
 mvprintw(row,0,"%s",b1);
 mvprintw(row/2,(col-strlen(mesg))/2,"%s",mesg);
                     		/* print the message at the center of the screen */
 getstr(str);
 mvprintw(LINES - 2, 0, "You Entered: %s", str);
 getch();
 endwin();

 return 0;
}