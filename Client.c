#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>

int socket_desc = 0;
char Name[50];
char GroupName[50];
#define telnet "127.0.0.1"
#define port_no 3205
int loopControl = 1;

void *send_data()
{
    char buffer[1024];
    while (loopControl)
    {
        fflush(stdout);
        fgets(buffer, 512, stdin);
        if (strncmp(buffer, "-send", 5) == 0)
        {
            char text[512];
            bzero(text, sizeof(text));
            int i = 6, cursor = 0;
            strcat(text, "{from :");
            strcat(text, Name);
            strcat(text, ", ");
            strcat(text, "to:");
            strcat(text, GroupName);
            strcat(text, ", ");
            strcat(text, "message:");
            char texttemp[512];
            while (buffer[i] != '\n')
            {
                texttemp[cursor] = buffer[i];
                i++;
                cursor++;
            }
            strcat(text, texttemp);
            strcat(text, "}\n");
            send(socket_desc, text, strlen(text), 0);
            bzero(texttemp, sizeof(texttemp));
            bzero(buffer, sizeof(buffer));
            bzero(text, sizeof(text));
        }
        else
        {
            if (strncmp(buffer, "-exit", 5) == 0 && strlen(buffer) == 6)
            {
                loopControl = 0;
                break;
            }
            if (strncmp(buffer, "-join", 5) == 0)
            {
                bzero(GroupName, sizeof(GroupName));
                int i = 5, cursor = 0;
                while (buffer[i] != '/')
                {
                    i++;
                }
                i++;
                while (buffer[i] != '\n')
                {
                    GroupName[cursor] = buffer[i];
                    i++;
                    cursor++;
                }
            }
            if (strncmp(buffer, "-gcreate", 8) == 0)
            {

                bzero(GroupName, sizeof(GroupName));
                int i = 8, cursor = 0;
                while (buffer[i] != '+')
                {
                    i++;
                }
                i++;
                while (buffer[i] != '\n')
                {
                    GroupName[cursor] = buffer[i];
                    i++;
                    cursor++;
                }
            }
            send(socket_desc, buffer, strlen(buffer), 0);
            bzero(buffer, sizeof(buffer));
        }
    }
}

void *get_data()
{
    char server_reply[3000];
    while (loopControl)
    {
        recv(socket_desc, server_reply, 2000, 0);
        printf("%s", &server_reply);
        bzero(server_reply, sizeof(server_reply));
    }
}

int main(int argc, char *argv[])
{
    struct sockaddr_in server;
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        puts("Could not create socket");
        return 1;
    }
    server.sin_addr.s_addr = inet_addr(telnet);
    server.sin_family = AF_INET;
    server.sin_port = htons(port_no);
    if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("Connection error");
        return 1;
    }
    printf("Enter Name : ");
    fgets(Name, 50, stdin);
    int deleteNextLine = 0;
    while (Name[deleteNextLine] != '\n')
    {
        deleteNextLine++;
    }
    Name[deleteNextLine] = '\0';
    pthread_t sender, getter;
    pthread_create(&sender, NULL, (void *)send_data, NULL);
    pthread_create(&getter, NULL, (void *)get_data, NULL);
    printf("Enter Phone Number : ");
    while (1)
    {
        if (!loopControl)
        {
            printf("\n Client is left \n");
            break;
        }
    }
    close(socket_desc);
    return 0;
}
