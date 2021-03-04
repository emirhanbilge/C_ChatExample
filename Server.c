#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static int pointC = 0;
static int pointG = 0;
typedef struct
{
    int socket_number;
    char ChatGroupID[50];
    char phone[30];
    struct sockaddr_in clientAddress;

} Client;
Client *clients[1024];

typedef struct
{
    char ChatGroupName[50];
    int size;
    int ChatGroupMembetsSockets[250];
} ChatGroup;
ChatGroup *ChatGroups[512];

pthread_mutex_t count_mutex;

void *requestAccept(void *socket_start)
{
    Client *localSocket = (Client *)socket_start;
    char buffer[1024];
    while (1)
    {
        recv(localSocket->socket_number, buffer, 1024, 0);
        if (strlen(buffer) < 0)
        {
            printf("Client leaving server ... \n");
            break;
        }
        else
        {
            if (strncmp(buffer, "-exit", 5) == 0 && strlen(buffer) == 6)
            {
                break;
            }
            else if (strncmp(buffer, "-join", 5) == 0)
            {
                char tempChatGroupName[75];
                bzero(tempChatGroupName, sizeof(tempChatGroupName));
                int i = 0, cursor = 0, flag = 0;
                while (buffer[i] != '\n')
                {
                    if (flag)
                    {
                        tempChatGroupName[cursor] = buffer[i];
                        cursor++;
                    }
                    if (buffer[i] == '/')
                    {
                        flag = 1;
                    }
                    i++;
                }
                flag = 0;
                for (i = 0; i < pointG; i++)
                {
                    if (strcmp(ChatGroups[i]->ChatGroupName, tempChatGroupName) == 0)
                    {
                        for (int k = 0; k < pointC; k++)
                        {
                            if (clients[k]->socket_number == localSocket->socket_number)
                            {
                                ChatGroups[i]->ChatGroupMembetsSockets[ChatGroups[i]->size] = localSocket->socket_number;
                                ChatGroups[i]->size = ChatGroups[i]->size + 1;
                                write(localSocket->socket_number, "Join is successfull\n", 21);
                                break;
                            }
                        }
                        flag = 1;
                        break;
                    }
                }
                char *infoMessage[75];
                if (flag)
                {
                    for (i = 0; i < pointC; i++)
                    {
                        if (clients[i]->socket_number == localSocket->socket_number)
                        {
                            strcat(clients[i]->ChatGroupID, tempChatGroupName);
                            strcat(infoMessage, tempChatGroupName);
                            strcat(infoMessage, "You are conncected group\n");
                            break;
                        }
                    }
                }
                else
                {
                    strcat(infoMessage, tempChatGroupName);
                    strcat(infoMessage, "You can not in group\n");
                    write(localSocket->socket_number, infoMessage, strlen(infoMessage));
                }
                bzero(infoMessage, sizeof(infoMessage));
                bzero(buffer, sizeof(buffer));
                bzero(tempChatGroupName, sizeof(tempChatGroupName));
            }
            else if (strncmp(buffer, "-whoami", 7) == 0)
            {
                char phoneTemp[50];
                bzero(buffer,sizeof(buffer));
                  bzero(phoneTemp, sizeof(phoneTemp));
                strcat(phoneTemp, localSocket->phone);
                write(localSocket->socket_number, phoneTemp, strlen(phoneTemp));
                bzero(buffer,sizeof(buffer));
                bzero(phoneTemp, sizeof(phoneTemp));
            }
            else if (strncmp(buffer, "-exit", 5) == 0 && strlen(buffer) != 6) // Grup çıkması
            {
                int i, indexer;
                char ChatGroupTempName[75];
                strcat(ChatGroupTempName, localSocket->ChatGroupID);
                for (i = 0; i < pointG; i++)
                {
                    if (strcmp(ChatGroupTempName, ChatGroups[i]->ChatGroupName) == 0)
                    {
                        for (indexer = 0; indexer < ChatGroups[i]->size; indexer++)
                        {
                            if (ChatGroups[i]->ChatGroupMembetsSockets[indexer] == localSocket->socket_number)
                            {
                                char tempBuffer[75] = "\0";
                                strcpy(localSocket->ChatGroupID, tempBuffer);
                                ChatGroups[i]->ChatGroupMembetsSockets[indexer] = -1;
                                strcat(tempBuffer, "Exit Group\n");
                                write(localSocket->socket_number, tempBuffer, strlen(tempBuffer));
                                break;
                            }
                        }
                        break;
                    }
                }
                 bzero(ChatGroupTempName,sizeof(ChatGroupTempName));
                  bzero(buffer,sizeof(buffer));
            }
            else if (buffer[0] == '{')
            {
                pthread_mutex_lock(&count_mutex);
                int q = 0, flag = 0, cursor = 0, counterJsonData = 0;
                char displayMessage[256];
                bzero(displayMessage, sizeof(displayMessage));
                while (buffer[q] != '\n')
                {
                    if (buffer[q] == ',' || buffer[q] == '}')
                    {
                        flag = 0;
                    }
                    if (flag && counterJsonData != 2)
                    {
                        displayMessage[cursor] = buffer[q];
                        cursor++;
                    }
                    if (buffer[q] == ':')
                    {
                        counterJsonData++;
                        flag = 1;
                        if (counterJsonData == 2)
                        {
                            displayMessage[cursor] = ':';
                            cursor++;
                        }
                    }
                    q++;
                }
                displayMessage[q] = '\n';
                strcat(displayMessage, "\n");

                for (int i = 0; i < pointG; i++)
                {
                    if (strcmp(localSocket->ChatGroupID, ChatGroups[i]->ChatGroupName) == 0)
                    {
                        for (int m = 0; m < ChatGroups[i]->size; m++)
                        {
                            if (ChatGroups[i]->ChatGroupMembetsSockets[m] != localSocket->socket_number)
                            {
                                write(ChatGroups[i]->ChatGroupMembetsSockets[m], displayMessage, strlen(displayMessage));
                            }
                        }
                    }
                }
                bzero(buffer, sizeof(buffer));
                pthread_mutex_unlock(&count_mutex);
            }
            else if (strncmp(buffer, "-gcreate", 8) == 0)
            {
                char tempChatGroupName[75];
                char *infoMessage[75];
                int i = 0, cursor = 0, flag = 0;
                while (buffer[i] != '\n')
                {
                    if (flag)
                    {
                        tempChatGroupName[cursor] = buffer[i];
                        cursor++;
                    }
                    if (buffer[i] == '+')
                    {
                        flag = 1;
                    }
                    i++;
                }
                strcat(localSocket->ChatGroupID, tempChatGroupName);
                ChatGroup *grp = (ChatGroup *)malloc(sizeof(ChatGroup));
                strcat(grp->ChatGroupName, tempChatGroupName);
                grp->size = 0;
                grp->ChatGroupMembetsSockets[grp->size] = localSocket->socket_number;
                grp->size = grp->size + 1;
                ChatGroups[pointG] = (ChatGroup *)grp;
                pointG++;
                strcat(infoMessage, "Group created successfully\n");
                write(localSocket->socket_number, infoMessage, strlen(infoMessage));
                bzero(infoMessage, sizeof(infoMessage));
            }
            bzero(buffer, sizeof(buffer));
        }
    }
}

int main(int argc, char *argv[])
{
    int socket_start, client_sock, c;
    Client *new_sock = (Client *)malloc(sizeof(Client));
    struct sockaddr_in server, client;

    socket_start = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_start == -1)
    {
        printf("Could not create socket");
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(3205);
    if (bind(socket_start, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Bind failed. Error");
        return 1;
    }
    listen(socket_start, 3);
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    while ((client_sock = accept(socket_start, (struct sockaddr *)&client, (socklen_t *)&c)))
    {
        printf("Connection accepted from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        Client *cli = (Client *)malloc(sizeof(Client));
        char num[30];
        bzero(num,sizeof(num));
        recv(client_sock, num, 20, 0);
        cli->clientAddress = client;
        cli->socket_number = client_sock;
        strcat(cli->phone, num);
        clients[pointC] = cli;
        pointC++;
        pthread_t sniffer_thread;
        if (pthread_create(&sniffer_thread, NULL, (void *)requestAccept, (void *)cli) < 0)
        {
            perror("could not create thread");
            return 1;
        }
    }
    return 0;
}
