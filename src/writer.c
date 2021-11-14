#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#define FIFO_NAME "myfifo"
#define BUFFER_SIZE 300

#define fd_stdout 1

int32_t fd_named_fifo;

void signal_handler(int signal)
{
    if (SIGUSR1 == signal)
    {
        write(fd_named_fifo, "SIGN:1\n", 8);
    }
    else
    {
        write(fd_named_fifo, "SIGN:2\n", 8);
    }
}

int main(void)
{
    pid_t pid = getpid();
    printf("PID: %d\n\r", pid);

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    //sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    char outputBuffer[BUFFER_SIZE];
    uint32_t bytesWrote;
    int32_t returnCode;

    /* Create named fifo. -1 means already exists so no action if already exists */
    if ((returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0)) < -1)
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }

    /* Open named fifo. Blocks until other process opens it */
    printf("waiting for readers...\n");
    if ((fd_named_fifo = open(FIFO_NAME, O_WRONLY)) < 0)
    {
        printf("Error opening named fifo file: %d\n", fd_named_fifo);
        exit(1);
    }

    /* open syscalls returned without error -> other process attached to named fifo */
    printf("got a reader--type some stuff\n");

    /* Loop forever */
    while (1)
    {
        /* Get some text from console */
        fgets(outputBuffer, BUFFER_SIZE, stdin);

        /* Write buffer to named fifo. Strlen - 1 to avoid sending \n char */
        if ((bytesWrote = write(fd_named_fifo, outputBuffer, strlen(outputBuffer) - 1)) == -1)
        {
            perror("write");
        }
        else
        {
            printf("writer: wrote %d bytes\n", bytesWrote);
        }
    }
    return 0;
}