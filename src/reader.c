#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#define FIFO_NAME "myfifo"
#define BUFFER_SIZE 300

int main(void)
{
    pid_t pid = getpid();
    printf("READER PID: %d\n\r", pid);

    uint8_t inputBuffer[BUFFER_SIZE];
    int32_t bytesRead, returnCode, fd;

    /* Create named fifo. -1 means already exists so no action if already exists */
    if ((returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0)) < -1)
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }

    /* Open named fifo. Blocks until other process opens it */
    printf("waiting for writers...\n");
    if ((fd = open(FIFO_NAME, O_RDONLY)) < 0)
    {
        printf("Error opening named fifo file: %d\n", fd);
        exit(1);
    }

    /* open syscalls returned without error -> other process attached to named fifo */
    printf("got a writer\n");

    FILE *MSG = fopen("msg.log", "w+");
    if (!MSG)
    {
        perror("fopen MSG");
        exit(EXIT_FAILURE);
    }
    FILE *SIG = fopen("sig.log", "w+");
    if (!SIG)
    {
        perror("fopen MSG");
        exit(EXIT_FAILURE);
    }
    /* Loop until read syscall returns a value <= 0 */
    do
    {
        /* read data into local buffer */
        if ((bytesRead = read(fd, inputBuffer, BUFFER_SIZE)) == -1)
        {
            perror("read");
        }
        else
        {
            inputBuffer[bytesRead] = '\0';
            printf("reader: read %d bytes: \"%s\"\n", bytesRead, inputBuffer);
            if ((0 == strcmp("SIGN:1", inputBuffer)) || (0 == strcasecmp("SIGN:2", inputBuffer)))
            {
                fputs(inputBuffer, SIG);
            }
            else
            {
                fputs(inputBuffer, MSG);
            }
        }
    } while (bytesRead > 0);
    close(MSG);
    close(SIG);
    return EXIT_SUCCESS;
}