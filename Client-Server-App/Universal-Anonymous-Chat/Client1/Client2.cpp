#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <inaddr.h>
#include <stdio.h>
#include <vector>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

void SendMessagesAsync(SOCKET clientSocket) {

    vector<char> clientBuff(1024);

    while (true) {
        cout << "Your message: ";
        fgets(clientBuff.data(), clientBuff.size(), stdin);

        // Check whether the client wants to stop chatting 
        if (clientBuff[0] == 'x' && clientBuff[1] == 'x' && clientBuff[2] == 'x') {
            shutdown(clientSocket, SD_BOTH);
            closesocket(clientSocket);
            WSACleanup();
            return;
        }

        // Send the message to the server
        int packet_size = send(clientSocket, clientBuff.data(), clientBuff.size(), 0);

        if (packet_size == SOCKET_ERROR) {
            cout << "Can't send message to Server. Error # " << WSAGetLastError() << endl;
            closesocket(clientSocket);
            WSACleanup();
            return;
        }
    }
}

int main(void) {

    const char SERVER_IP[] = "127.0.0.1";					// Enter IPv4 address of Server
    const short SERVER_PORT_NUM = 17177;				// Enter Listening port on Server side
    const short BUFF_SIZE = 1024;					// Maximum size of buffer for exchange info between server and client

    // Key variables for all program
    int erStat;										// For checking errors in sockets functions

    //IP in string format to numeric format for socket functions. Data is in "ip_to_num"
    in_addr ip_to_num;
    inet_pton(AF_INET, SERVER_IP, &ip_to_num);


    // WinSock initialization
    WSADATA wsData;
    erStat = WSAStartup(MAKEWORD(2, 2), &wsData);

    if (erStat != 0) {
        cout << "Error WinSock version initializaion #";
        cout << WSAGetLastError();
        return 1;
    }
    else
        cout << "WinSock initialization is OK" << endl;

    // Socket initialization
    SOCKET ClientSock = socket(AF_INET, SOCK_STREAM, 0);

    if (ClientSock == INVALID_SOCKET) {
        cout << "Error initialization socket # " << WSAGetLastError() << endl;
        closesocket(ClientSock);
        WSACleanup();
    }
    else
        cout << "Client socket initialization is OK" << endl;

    // Establishing a connection to Server
    sockaddr_in servInfo;

    ZeroMemory(&servInfo, sizeof(servInfo));

    servInfo.sin_family = AF_INET;
    servInfo.sin_addr = ip_to_num;
    servInfo.sin_port = htons(SERVER_PORT_NUM);

    erStat = connect(ClientSock, (sockaddr*)&servInfo, sizeof(servInfo));

    if (erStat != 0) {
        cout << "Connection to Server is FAILED. Error # " << WSAGetLastError() << endl;
        closesocket(ClientSock);
        WSACleanup();
        return 1;
    }
    else
        cout << "Connection established SUCCESSFULLY. Ready to send a message to Server" << endl;


    //Exchange text data between Server and Client. Disconnection if a Client send "xxx"

    vector <char> servBuff(BUFF_SIZE), clientBuff(BUFF_SIZE);							// Buffers for sending and receiving data
    short packet_size = 0;

    // ������� ����� ��� ����������� �������� ���������
    std::thread sendThread(SendMessagesAsync, ClientSock);

    //vector<char> servBuff(1024);
   // short packet_size = 0;

    while (true) {
        // Receive messages from the server
        packet_size = recv(ClientSock, servBuff.data(), servBuff.size(), 0);

        if (packet_size == SOCKET_ERROR) {
            cout << "Can't receive message from Server. Error # " << WSAGetLastError() << endl;
            closesocket(ClientSock);
            WSACleanup();
            return 1;
        }
        else if (packet_size > 0) {
            cout << "\n\nServer message: " << servBuff.data() << endl;
            cout << "Your message: ";
        }
    }

    // ������� ���������� ������ ��������
    sendThread.join();

    closesocket(ClientSock);
    WSACleanup();

    return 0;
}