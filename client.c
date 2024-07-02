#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ncurses.h>

// Global variables for game state
int xpos[100];
int ypos[100];
int size;
int current_score;
int food_x;
int food_y;

// Function prototypes
void initializeGameState();
void updateLocalGameState(int received_xpos[], int received_ypos[], int received_size, int received_score);
void generateFoodPosition();
int collisionDetected();
void gameOver();
void drawGameBoard();

int main()
{
    // Initialize the game state
    initializeGameState();

    // Create a socket
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Connected to server.\n");

    while (1)
    {
        // Here you would receive updates from the server and update the game state accordingly
        int received_xpos[100];
        int received_ypos[100];
        int received_size;
        int received_score;

        // Receive the updated game state from the server
        recv(sock, received_xpos, sizeof(received_xpos), 0); // Receive xpos array
        recv(sock, received_ypos, sizeof(received_ypos), 0); // Receive ypos array
        recv(sock, &received_size, sizeof(int), 0);           // Receive size
        recv(sock, &received_score, sizeof(int), 0);          // Receive score

        // Update the local game state
        updateLocalGameState(received_xpos, received_ypos, received_size, received_score);

        // Optionally, you can update the screen to reflect the new game state
        // This may involve redrawing the snake and any other elements on the screen
        // based on the updated xpos, ypos, and size values
        drawGameBoard();
    }

    return 0;
}

void initializeGameState()
{
    // Initialize the snake's starting position and size
    size = 5;
    for (int i = 0; i < size; i++)
    {
        xpos[i] = 20 - i * 2;
        ypos[i] = 10;
    }

    // Initialize the food's position
    generateFoodPosition();

    // Initialize the score
    current_score = 0;
}

void updateLocalGameState(int received_xpos[], int received_ypos[], int received_size, int received_score)
{
    // Update the local game state based on the received updates from the server
    if (received_size > 0)
    {
        // Update the snake's position and size
        memcpy(xpos, received_xpos, sizeof(received_xpos));
        memcpy(ypos, received_ypos, sizeof(received_ypos));
        size = received_size;

        // Check for collision with food
        if (received_xpos[0] == food_x && received_ypos[0] == food_y)
        {
            // Increase the snake's size
            size++;
            // Generate new coordinates for the food
            generateFoodPosition();
            // Increment the score
            current_score++;
        }

        // Check for collision with walls or the snake's own body
        if (collisionDetected())
        {
            // Game over logic
            gameOver();
        }
    }
}

void generateFoodPosition()
{
    // Generate random coordinates for the food within the game board boundaries
    food_x = rand() % 38 * 2 + 4; // Random even number between 4 and 76
    food_y = rand() % 17 + 2;      // Random number between 2 and 18
}

int collisionDetected()
{
    // Check if the snake collides with walls or its own body
    if (xpos[0] < 4 || xpos[0] > 76 || ypos[0] < 2 || ypos[0] > 18)
    {
        return 1; // Collision with walls
    }

    for (int i = 1; i < size; i++)
    {
        if (xpos[0] == xpos[i] && ypos[0] == ypos[i])
        {
            return 1; // Collision with its own body
        }
    }

    return 0; // No collision
}

void gameOver()
{
    // Handle game over logic here
    printf("Game over!\n");
    exit(0);
}

void drawGameBoard()
{
    // Draw the game board with the snake, food, and score
    // You can use Ncurses or any other graphics library for this purpose
    // For simplicity, I'll just print the game state to the console

    system("clear");

    printf("Score: %d\n", current_score);
    printf("+-");
    for (int i = 0; i < 76; i++)
    {
        printf("-");
    }
    printf("+\n");

    for (int y = 0; y < 20; y++)
    {
        printf("| ");
        for (int x = 0; x < 38; x++)
        {
            if (y == food_y && x * 2 == food_x - 4)
            {
                printf("@");
            }
            else
            {
                int found = 0;
                for (int i = 0; i < size; i++)
                {
                    if (xpos[i] == x * 2 + 4 && ypos[i] == y)
                    {
                        if (i == 0)
                        {
                            printf("O");
                        }
                        else
                        {
                            printf("o");
                        }
                        found = 1;
                        break;
                    }
                }
                if (!found)
                {
                    printf(" ");
                }
            }
        }
        printf(" |\n");
    }

    printf("+-");
    for (int i = 0; i < 76; i++)
    {
        printf("-");
    }
    printf("+\n");

    fflush(stdout);
}

