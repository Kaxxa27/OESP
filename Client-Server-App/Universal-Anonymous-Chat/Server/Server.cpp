#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

// ���������� ������� ��� ����������� ������� � ������� ������� ��������
std::mutex clientsMutex;

// ������� ��� ��������� ��������� �� ������� � ��������� ������
void HandleClient(SOCKET clientSocket, std::vector<std::shared_ptr<SOCKET>>& clientSockets)
{
    char buffer[1024];
    int bytesReceived;

    while (true)
    {
        // ��������� ��������� �� �������
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0)
        {
            // ������ ��� ������ ����������
            closesocket(clientSocket);

            // ������� ����� �� �������
            std::lock_guard<std::mutex> lock(clientsMutex);
            auto it = std::find_if(clientSockets.begin(), clientSockets.end(),
                [clientSocket](const std::shared_ptr<SOCKET>& ptr) {
                    return *ptr == clientSocket;
                });

            if (it != clientSockets.end())
            {
                clientSockets.erase(it);
            }

            cout << "[DISCONNECT] Client disconnected " << endl;
            cout << "[INFO] Number of clients on the server: " << clientSockets.size() << endl;

            break;
        }

        // ���������� ��������� ���� ������������ ��������
        // std::lock_guard<std::mutex> lock(clientsMutex);
        for (const auto& socketPtr : clientSockets)
        {
            SOCKET otherClientSocket = *socketPtr;

            if (otherClientSocket != clientSocket)
            {
                send(otherClientSocket, buffer, bytesReceived, 0);
            }
        }
    }
}

int main()
{
    const char IP_SERV[] = "127.0.0.1";
    const int PORT_NUM = 17177;
    const short BUFF_SIZE = 1024;

    int erStat;

    in_addr ip_to_num;
    erStat = inet_pton(AF_INET, IP_SERV, &ip_to_num);

    if (erStat <= 0)
    {
        cout << "[ERROR] Error in IP translation to special numeric format" << endl;
        return 1;
    }

    WSADATA wsData;
    erStat = WSAStartup(MAKEWORD(2, 2), &wsData);

    if (erStat != 0)
    {
        cout << "[ERROR] Error WinSock version initialization #" << WSAGetLastError() << endl;
        return 1;
    }
    else
        cout << "[INITIALIZE] WinSock initialization is OK" << endl;

    SOCKET ServSock = socket(AF_INET, SOCK_STREAM, 0);

    if (ServSock == INVALID_SOCKET)
    {
        cout << "[ERROR] Error initialization socket # " << WSAGetLastError() << endl;
        closesocket(ServSock);
        WSACleanup();
        return 1;
    }
    else
        cout << "[INITIALIZE] Server socket initialization is OK" << endl;

    sockaddr_in servInfo;
    ZeroMemory(&servInfo, sizeof(servInfo));

    servInfo.sin_family = AF_INET;
    servInfo.sin_addr = ip_to_num;
    servInfo.sin_port = htons(PORT_NUM);

    erStat = bind(ServSock, (sockaddr*)&servInfo, sizeof(servInfo));

    if (erStat != 0)
    {
        cout << "[ERROR] Error Socket binding to server info. Error # " << WSAGetLastError() << endl;
        closesocket(ServSock);
        WSACleanup();
        return 1;
    }
    else
        cout << "[INITIALIZE] Binding socket to Server info is OK" << endl;

    erStat = listen(ServSock, SOMAXCONN);

    if (erStat != 0)
    {
        cout << "[ERROR] Can't start to listen to. Error # " << WSAGetLastError() << endl;
        closesocket(ServSock);
        WSACleanup();
        return 1;
    }
    else
    {
        cout << "[LISTEN] Listening..." << endl;
    }

    // ������ ��� �������� ������� ��������
    std::vector<std::shared_ptr<SOCKET>> clientSockets;

    while (true)
    {
        sockaddr_in clientInfo;
        int clientInfo_size = sizeof(clientInfo);
        ZeroMemory(&clientInfo, clientInfo_size);

        SOCKET ClientConn = accept(ServSock, (sockaddr*)&clientInfo, &clientInfo_size);

        if (ClientConn == INVALID_SOCKET)
        {
            cout << "[ERROR] Client detected, but can't connect to a client. Error # " << WSAGetLastError() << endl;
            closesocket(ServSock);
            closesocket(ClientConn);
            WSACleanup();
            return 1;
        }
        else
        {
            cout << "[CONNECT] Connection to a client established successfully" << endl;
            char clientIP[22];

            inet_ntop(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN);
            cout << "[CONNECT] Client connected with IP address " << clientIP << endl;

            // ��������� ����� ������� � ������
            std::shared_ptr<SOCKET> clientSocketPtr = std::make_shared<SOCKET>(ClientConn);
            std::lock_guard<std::mutex> lock(clientsMutex);
            clientSockets.push_back(clientSocketPtr);

            cout << "[INFO] Number of clients on the server: " << clientSockets.size() << endl;

            // ������� ����� ��� ��������� ��������� �������
            std::thread clientThread(HandleClient, ClientConn, std::ref(clientSockets));
            // ������� �����, ����� �� ���������� ��������������
            clientThread.detach();
        }
    }

    closesocket(ServSock);
    WSACleanup();

    return 0;
}

