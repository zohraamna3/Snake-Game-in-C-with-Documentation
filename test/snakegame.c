#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <ncurses.h>

// Define constants for directions
#define LEFT 1
#define RIGHT 2
#define UP 3
#define DOWN 4

int flow = RIGHT;
pthread_mutex_t mutex; // Mutex for synchronization
int arrow_pressed = 0; // Flag to indicate arrow key press
int snake_speed = 200; // Initial speed
int current_score = 0; // Initial score

// Function prototypes
void *controlSnake(void *arg);
void *getup();
void *adjustSpeed(void *arg);
void updateScore(int sc);
void showStatus(char *s, int c);

int kbhit(void)
{
    int ch = getch();
    if (ch != ERR)
    {
        ungetch(ch);
        return 1;
    }
    else
    {
        return 0;
    }
}

int main()
{
    int size, i, xb, yb;
    int restart = 1, tmp;
    int xpos[100], ypos[100], scr;
    pthread_t controlThread, gameThread, speedThread;
    srand(time(NULL));
    initscr();			   
    cbreak();			   
    noecho();			   
    keypad(stdscr, TRUE);  
    nodelay(stdscr, TRUE); 

    // Initialize mutex
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&gameThread, NULL, getup, NULL);
    pthread_create(&speedThread, NULL, adjustSpeed, NULL);

    while (1)
    {
        if (restart)
        {
            size = 5;
            scr = 0;
            updateScore(scr);
            flow = RIGHT;
            xpos[0] = 20;
            for (i = 0; i < size; i++)
            {
                xpos[i] = xpos[0] - i * 2;
                ypos[i] = 10;
            }
            for (tmp = 1;;)
            {
                do
                {
                    xb = rand() % 75 + 3;
                } while (xb % 2 != 0);
                yb = rand() % 17 + 2;
                for (i = 0; i < size; i++)
                    if (xb == xpos[i] && yb == ypos[i])
                    {
                        tmp = 0;
                        break;
                    }
                if (tmp)
                    break;
            }
            gotoxy(xb, yb);
            printw("@");
            restart = 0;
        }

        pthread_create(&controlThread, NULL, controlSnake, NULL);
        
        if (!kbhit())
        {
            if (xpos[0] == xb && ypos[0] == yb)
            {
                for (tmp = 1;;)
                {
                    do
                    {
                        xb = rand() % 75 + 3;
                    } while (xb % 2 != 0);
                    yb = rand() % 17 + 2;
                    for (i = 0; i < size; i++)
                        if (xb == xpos[i] && yb == ypos[i])
                        {
                            tmp = 0;
                            break;
                        }
                    if (tmp)
                        break;
                }
                gotoxy(xb, yb);
                printw("@");
                size++;
                scr++;
                updateScore(scr);
            }
            gotoxy(xpos[size - 1], ypos[size - 1]);
            for (i = size - 1; i > 0; i--)
            {
                xpos[i] = xpos[i - 1];
                ypos[i] = ypos[i - 1];
            }
            switch (flow)
            {
                case RIGHT:
                    xpos[i] += 2;
                    break;
                case LEFT:
                    xpos[i] -= 2;
                    break;
                case UP:
                    ypos[i] -= 1;
                    break;
                case DOWN:
                    ypos[i] += 1;
            }
            tmp = 1;
            for (i = 1; i < size; i++)
                if (xpos[i] == xpos[0] && ypos[i] == ypos[0])
                {
                    tmp = 0;
                    break;
                }
            if (xpos[0] > 76 || xpos[0] < 4 || ypos[0] < 2 || ypos[0] > 18)
                tmp = 0;
            if (tmp)
            {
                printw(" ");
                gotoxy(xpos[0], ypos[0]);
                printw("O");
                gotoxy(xpos[1], ypos[1]);
                printw("o");
            }
            else
            {
                printw("o");
                gotoxy(xpos[1], ypos[1]);
                printw("O");
                for (i = 2; i < size; i++)
                {
                    gotoxy(xpos[i], ypos[i]);
                    printw("o");
                }
                showStatus("Game Over", 12); 
                restart = 1;
                usleep(2000000); 
                clear();
                pthread_create(&gameThread, NULL, getup, NULL);
                pthread_join(gameThread, NULL);
            }
            usleep(snake_speed * 1000);
        }
        pthread_join(controlThread, NULL);
    }
    pthread_mutex_destroy(&mutex);
    return 0;
}

// Function for controlling the movement of the snake
void *controlSnake(void *arg)
{
    pthread_mutex_lock(&mutex);
    
    int ch;
    
    /*static int time_slice = 0; // Static variable to keep track of time slice
     // Round Robin scheduling algorithm
    switch (time_slice % 120)
    {
        case 0:
            flow = RIGHT;
            break;
        case 30:
            flow = DOWN;
            break;
        case 60:
            flow = LEFT;
            break;
        case 90:
            flow = UP;
            break;
    }
    time_slice++;
    
    */
    ch = getch();
    if (ch != ERR)
    {
        switch (ch)
        {
            case 'x':
            case 'X':
                endwin();
                exit(0);
            case ' ':
                showStatus("Paused", 14); 
                while (1)
                {
                    int z = getch();
                    if (z == 'x' || z == 'X')
                    {
                        endwin(); 
                        return 0;
                    }
                    if (z == ' ')
                    {
                        showStatus("Playing", 10); 
                        break;
                    }
                }
                break;
            case KEY_UP:
                if (flow != DOWN)
                    flow = UP;
                break;
            case KEY_DOWN:
                if (flow != UP)
                    flow = DOWN;
                break;
            case KEY_LEFT:
                if (flow != RIGHT)
                    flow = LEFT;
                break;
            case KEY_RIGHT:
                if (flow != LEFT)
                    flow = RIGHT;
                break;
        }
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

// Function to adjust snake speed based on score
void *adjustSpeed(void *arg)
{
    static int time_slice = 0; // Static variable to keep track of time slice

    while (1)
    {
        // Round Robin scheduling for snake speed
        switch (time_slice % 65)
        {
            case 0:
                snake_speed +=10;
                break;
           /* case 50:
                snake_speed = 250;
                break;*/
           
        }
        time_slice++;

        usleep(500000); // Adjust the sleep duration as needed
    }

    return NULL;
}
void textcolor(int fc, int bc)
{
    start_color();
    init_pair(1, fc, bc);
    attron(COLOR_PAIR(1));
}

void gotoxy(int x, int y)
{
    move(y, x);
}

void *getup()
{
    int x;
    attron(A_BOLD);
    printw("\n  %c", 218);
    for (x = 0; x < 75; x++)
        printw("%c", 196);
    printw("%c  ", 191);
    for (x = 0; x < 17; x++)
    {
        gotoxy(2, x + 2);
        printw("%c", 179);
        gotoxy(78, x + 2);
        printw("%c ", 179);
    }
    printw("  %c", 192);
    for (x = 0; x < 75; x++)
        printw("%c", 196);
    printw("%c  ", 217);
    printw(" %c", 218);
    for (x = 0; x < 21; x++)
        printw("%c", 196);
    printw("%c\n", 191);
    printw("  %c M A D    S N A K E %c\n", 179, 179);
    printw("  %c", 192);
    for (x = 0; x < 21; x++)
        printw("%c", 196);
    printw("%c", 217);
    gotoxy(59, 20);
    printw("%c", 218);
    for (x = 0; x < 18; x++)
        printw("%c", 196);
    printw("%c", 191);
    gotoxy(59, 21);
    printw("%c SCORE :      0      %c", 179, 179);
    gotoxy(59, 22);
    printw("%c STATUS: Playing  %c", 179, 179);
    gotoxy(59, 23);
    printw("%c", 192);
    for (x = 0; x < 18; x++)
        printw("%c", 196);
    printw("%c", 217);
    gotoxy(28, 20);
    printw("Press 'x' to Exit");
    gotoxy(28, 21);
    printw("Press Space to Pause and Play");
    gotoxy(10, 23);
    attron(COLOR_PAIR(1));
    printw(" Project 2022-SE-31 & 2022-SE-31 ");
    attroff(A_BOLD);
    attron(COLOR_PAIR(1));

    return NULL;
}

void updateScore(int sc)
{
    current_score = sc;
    gotoxy(69, 21);
    printw("%6d", current_score * 10);
}

void showStatus(char *s, int c)
{
    attron(COLOR_PAIR(1));
    gotoxy(69, 22);
    printw("%-8s", s); 
    attroff(COLOR_PAIR(1));
}
