#include <iostream>
#include <fstream>
#include <windows.h>
#include <winsock2.h>

SOCKET clientSocket;

int InitSocket()
{
  std::ofstream socketFile("d:\\tmp\\socket.txt");

  // Initialize Winsock
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    std::cerr << "WSAStartup failed." << std::endl;
    socketFile << "WSAStartup failed." << std::endl;
    socketFile.close();
    return 1;
  }

  // Create a socket
  clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket == INVALID_SOCKET) {
    std::cerr << "Socket creation failed." << std::endl;
    socketFile << "Socket creation failed." << std::endl;
    socketFile.close();
    return 1;
  }

  // Define the server's address and port
  sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(8080); // Port number
  serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address

  // Connect to the server
  if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
    std::cerr << "Connection to the server failed." << std::endl;
    socketFile << "Connection to the server failed." << std::endl;
    closesocket(clientSocket);
    WSACleanup();
    socketFile.close();
    return 1;
  }

  socketFile.close();
  return 0;
}

void SendHello()
{
  unsigned char byteToSend = 0xFF;
  if (send(clientSocket, reinterpret_cast<const char*>(&byteToSend), 1, 0) == SOCKET_ERROR) {
    std::cerr << "Sending data failed." << std::endl;
    //    socketFile << "Sending data failed." << std::endl;
  }
  else {
    std::cout << "Data sent successfully." << std::endl;
    //   socketFile << "Data sent successfully." << std::endl;
  }

  byteToSend = 14;
  if (send(clientSocket, reinterpret_cast<const char*>(&byteToSend), 1, 0) == SOCKET_ERROR) {
    std::cerr << "Sending data failed." << std::endl;
    //    socketFile << "Sending data failed." << std::endl;
  }
  else {
    std::cout << "Data sent successfully." << std::endl;
    //   socketFile << "Data sent successfully." << std::endl;
  }

  // Send a string to the server
  const char* message = "Hello, Server!";
  if (send(clientSocket, message, strlen(message), 0) == SOCKET_ERROR) {
    std::cerr << "Sending data failed." << std::endl;
//    socketFile << "Sending data failed." << std::endl;
  }
  else {
    std::cout << "Data sent successfully." << std::endl;
 //   socketFile << "Data sent successfully." << std::endl;
  }

}

void CloseSocket();


int main2() {

  InitSocket();
  SendHello();
  SendHello();
//  CloseSocket();
  return 0;
}

extern bool g_explosion;

void SendPacket(int16_t ux16, int16_t uy16, unsigned char packedFacing, unsigned char moving, unsigned char isExplosion, int16_t ex16, int16_t ey16)
{
  unsigned char byteToSend = 0xFE;
  if (send(clientSocket, reinterpret_cast<const char*>(&byteToSend), 1, 0) == SOCKET_ERROR) {
    std::cerr << "Sending data failed." << std::endl;
    //    socketFile << "Sending data failed." << std::endl;

    CloseSocket();
    InitSocket();
  }
  else {
  //  std::cout << "Data sent successfully." << std::endl;
    //   socketFile << "Data sent successfully." << std::endl;
  }

  if (send(clientSocket, reinterpret_cast<const char*>(&ux16), sizeof(ux16), 0) == SOCKET_ERROR) {
    std::cerr << "Sending data failed." << std::endl;
    //    socketFile << "Sending data failed." << std::endl;
  }
  else {
  //  std::cout << "Data sent successfully." << std::endl;
    //   socketFile << "Data sent successfully." << std::endl;
  }
  if (send(clientSocket, reinterpret_cast<const char*>(&uy16), sizeof(uy16), 0) == SOCKET_ERROR) {
    std::cerr << "Sending data failed." << std::endl;
    //    socketFile << "Sending data failed." << std::endl;
  }
  else {
  //  std::cout << "Data sent successfully." << std::endl;
    //   socketFile << "Data sent successfully." << std::endl;
  }
  if (send(clientSocket, reinterpret_cast<const char*>(&packedFacing), sizeof(packedFacing), 0) == SOCKET_ERROR) {
    std::cerr << "Sending data failed." << std::endl;
    //    socketFile << "Sending data failed." << std::endl;
  }
  else {
 //   std::cout << "Data sent successfully." << std::endl;
    //   socketFile << "Data sent successfully." << std::endl;
  }
  if (send(clientSocket, reinterpret_cast<const char*>(&moving), sizeof(moving), 0) == SOCKET_ERROR) {
    std::cerr << "Sending data failed." << std::endl;
    //    socketFile << "Sending data failed." << std::endl;
  }
  else {
  //  std::cout << "Data sent successfully." << std::endl;
    //   socketFile << "Data sent successfully." << std::endl;
  }
  if (isExplosion)
  {
    if (send(clientSocket, reinterpret_cast<const char*>(&isExplosion), sizeof(isExplosion), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
    g_explosion = false;
  }
  else
  {
    if (send(clientSocket, reinterpret_cast<const char*>(&isExplosion), sizeof(isExplosion), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
  }
  if (send(clientSocket, reinterpret_cast<const char*>(&ex16), sizeof(ex16), 0) == SOCKET_ERROR) {
    std::cerr << "Sending data failed." << std::endl;
    //    socketFile << "Sending data failed." << std::endl;
  }
  if (send(clientSocket, reinterpret_cast<const char*>(&ey16), sizeof(ey16), 0) == SOCKET_ERROR) {
    std::cerr << "Sending data failed." << std::endl;
    //    socketFile << "Sending data failed." << std::endl;
  }
}

void CloseSocket()
{
  // Clean up and close the socket
  closesocket(clientSocket);
  WSACleanup();

}