#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#define MAX_CLIENTS 2
// Define constants for directions
#define LEFT 1
#define RIGHT 2
#define UP 3
#define DOWN 4

// Global variables
int flow = RIGHT;
pthread_mutex_t mutex; // Mutex for synchronization
int snake_speed = 200; // Initial speed
int current_score = 0; // Initial score
int num_clients = 0;    // Number of connected clients
struct Client {
    int socket_fd;
    // Add more fields as needed
} clients[MAX_CLIENTS]; // Array to store client information

// Function prototypes
void *controlSnake(void *arg);
void updateScore(int sc);
void broadcastGameState(int xpos[], int ypos[], int size);
void *adjustSpeed(void *arg);

int main()
{
    int size, i, xb, yb;
    int restart = 1, tmp;
    int xpos[100], ypos[100], scr;

    // Initialize mutex
    pthread_mutex_init(&mutex, NULL);

    srand(time(NULL));

    // Create a socket
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Create thread for adjusting snake speed
    pthread_t speed_thread;
    pthread_create(&speed_thread, NULL, adjustSpeed, NULL);

    while (1)
    {
        printf("Waiting for connection...\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                 (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Connection established with client.\n");

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
            broadcastGameState(xpos, ypos, size);
            restart = 0;
        }

        pthread_t controlThread;
        pthread_create(&controlThread, NULL, controlSnake, NULL);

        if (new_socket > 0)
        {
            // Add client information to the array
            clients[num_clients].socket_fd = new_socket;
            num_clients++;
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
    ch = getchar();
    if (ch != EOF)
    {
        switch (ch)
        {
        case 'x':
        case 'X':
            exit(0);
        case ' ':
            break; // Handle pause/resume logic
        case 'w':
            if (flow != DOWN)
                flow = UP;
            break;
        case 's':
            if (flow != UP)
                flow = DOWN;
            break;
        case 'a':
            if (flow != RIGHT)
                flow = LEFT;
            break;
        case 'd':
            if (flow != LEFT)
                flow = RIGHT;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

// Function to update the score
void updateScore(int sc)
{
    current_score = sc;
}
void *adjustSpeed(void *arg)
{
    static int time_slice = 0; // Static variable to keep track of time slice

    while (1)
    {
        // Round Robin scheduling for snake speed
        switch (time_slice % 100)
        {
            case 0:
                snake_speed = 100;
                break;
            case 50:
                snake_speed = 250;
                break;
           
        }
        time_slice++;

        usleep(500000); // Adjust the sleep duration as needed
    }

    return NULL;
}
// Function to broadcast game state to clients
void broadcastGameState(int xpos[], int ypos[], int size)
{
    // Here you would send the game state (xpos, ypos, size, score, etc.) to all connected clients
    for (int i = 0; i < num_clients; i++)
    {
        send(clients[i].socket_fd, xpos, sizeof(int) * size, 0); // Send xpos array
        send(clients[i].socket_fd, ypos, sizeof(int) * size, 0); // Send ypos array
        send(clients[i].socket_fd, &size, sizeof(int), 0);       // Send size
        send(clients[i].socket_fd, &current_score, sizeof(int), 0); // Send score
        // Add error handling if necessary
    }
}

