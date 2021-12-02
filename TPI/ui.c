#include <ncurses.h>			/* ncurses.h includes stdio.h */  
#include <string.h> 
#include <stdlib.h>
 
int main()
{
    initscr();				/* start the curses mode */
    cbreak();
    noecho();

    int row, col, y, x;
    int pos = 1;
    getmaxyx(stdscr,row,col);
    char str[80];
    char msgs[80*10] = "messages";

    y = x = 0;
    WINDOW * receiver = newwin(row-3, col, y, x);
    y = row-3;
    WINDOW * sender = newwin(row/8, col, y, x);
    refresh();
    box(receiver, 104, 104);
    box(sender, 105, 105);
    mvwprintw(receiver, 1, 1, "%s", msgs);
    mvwprintw(sender, 1, 1, "P:");
    wrefresh(receiver);
    wrefresh(sender);
    echo();
    wgetstr(sender, str);
    while(1) {
        mvwprintw(sender, 1, 1, "                                                  "); // TODO find better way
        mvwprintw(receiver, ++pos, 1, str);
        mvwprintw(sender, 1, 1, "P:");
        wrefresh(receiver);
        wrefresh(sender);
        wgetstr(sender, str);
        if (pos >= row-5) { // look for bett way
            pos = 1;
        }
    }
    getch();
    endwin();

    return 0;
}