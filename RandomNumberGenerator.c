#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>

//This is a Paceholder function to avoid quitting when receiving a SIGUSR1
void action(){};


// Function executed by each player, to generate the random number
void player(char *name, int playerId, int fd)
{
    fd = open("sharedFile.bin", O_RDONLY);
    int points = 0, randomValue, oldScore[1];
    long int ss = 0;
    while (1)
    {
        signal(SIGUSR1, action);
        pause();
        printf("------------------------------------\n");
        // Reading the old score
        if (playerId == 1) //Player ONE
        {
            lseek(fd, 0, SEEK_SET);
            read(fd, oldScore, sizeof(int));
            printf("ONE: My current score is :: %d\n", oldScore[0]);
        }
        else if (playerId == 2) //Player TWO
        {
            lseek(fd, sizeof(int), SEEK_SET);
            read(fd, oldScore, sizeof(int));
            printf("TWO: My current score is :: %d\n", oldScore[0]);
        }
        else //Player THREE
        {
            lseek(fd, 2 * sizeof(int), SEEK_SET);
            read(fd, oldScore, sizeof(int));
            printf("THREE: My current score is :: %d\n", oldScore[0]);
        }
        close(fd);
        // Playing the dice and printing its own name and the dice score
        printf("%s: Generating my random number!\n", name);
        randomValue = (int)time(&ss) % 50 + 1;
        printf("%s: I got %d points\n", name, randomValue);
        // Updating the old score
        int old = oldScore[0];
        oldScore[0] = old + randomValue;
        printf("%s: My score now is :: %d points\n------------------------------------\n\n", name, oldScore[0]);
        // Writing the new score at the same file offset
        fd = open("sharedFile.bin", O_WRONLY);
        if (playerId == 1) //Player ONE
        {
            lseek(fd, 0, SEEK_SET);
            write(fd, oldScore, sizeof(int));
        }
        else if (playerId == 2) //Player TWO
        {
            lseek(fd, sizeof(int), SEEK_SET);
            write(fd, oldScore, sizeof(int));
        }
        else //Player THREE
        {
            lseek(fd, 2 * sizeof(int), SEEK_SET);
            write(fd, oldScore, sizeof(int));
        }
        close(fd);
        // Sleeping for 3 seconds and signaling the referee before pausing
        sleep(3);
        kill(getppid(), SIGUSR1);
    }
}
//Function executed by the referee, to print current scores and check for a winner after every player plays once set
void checkWinner(int fd, char *name)
{
    int currentScore[1];
    // read the new totals from sharedFile.bin
    read(fd, currentScore, sizeof(int));
    printf("\n");
    // print player's name and total points so far
    if (strcmp(name, "ONE") == 0)
        printf("Referee: Player ONE's current score :: ");
    else if (strcmp(name, "TWO") == 0)
        printf("Referee: Player TWO's current score :: ");
    else
        printf("Referee: Player THREE's current score :: ");
    printf("%d\n", currentScore[0]);
    sleep(2);
    // checking if there's a winner and terminating all processes in case there is
    if (currentScore[0] >= 100)
    {
        printf("\n------------GAME OVER--------------\n\n------------------------------------\n");
        printf("Referee: Player %s won the game!!!!\n------------------------------------\n\n", name);
        kill(0, SIGTERM);
    }
}


int main(int argc, char *argv[])
{
    int fd;
    pid_t pid1, pid2, pid3;
    printf("------------------------------------------------------------------------------------------------\nHello there! Welcome to the Random Number Game: a 3-player game, with another process being the mediator/referee that keeps scores!\nEach player is a process.\nEach one of the player processes generates a random number turn after turn.\nThe referee process keeps track of these scores, and declares a winner once one of the players cross 100 points!\n------------------------------------------------------------------------------------------------\n\n");
    sleep(5);

    // Creating the binary file before forking
    if ((fd = open("sharedFile.bin", O_CREAT | O_WRONLY | O_TRUNC, 0777)) == -1)
    {
        perror("Uh oh, The sharedFile.bin coukd not open, File problem, Check what you're doing wrong oops.\n\n");
        exit(1);
    }
    else
    {
        // Write three zero-integer values to the file, for teach of the three processes
        int threeZeros[3];
        threeZeros[0] = 0;
        threeZeros[1] = 0;
        threeZeros[2] = 0;
        write(fd, threeZeros, 3 * sizeof(int));
        close(fd);
    }

    // Create the players and calling the common function "player"
    if ((pid1 = fork()) == 0)
        player("ONE", 1, fd);
    if ((pid2 = fork()) == 0)
        player("TWO", 2, fd);
    if ((pid3 = fork()) == 0)
        player("THREE", 3, fd);
    sleep(1);
    signal(SIGUSR1, action);
    while (1)
    {
        // Make the players play in order: Player One, Player Two and then Player Three
        fd = open("sharedFile.bin", O_RDONLY);
        checkWinner(fd, "ONE");
        printf("Referee: Player ONE plays\n\n");
        kill(pid1, SIGUSR1);
        pause();
        checkWinner(fd, "TWO");
        printf("Referee: Player TWO plays\n\n");
        kill(pid2, SIGUSR1);
        pause();
        checkWinner(fd, "THREE");
        printf("Referee: Player THREE plays\n\n");
        kill(pid3, SIGUSR1);
        pause();
    }
}
