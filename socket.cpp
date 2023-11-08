#include <iostream>
#include <fstream>
#include <windows.h>
#include <winsock2.h>
#include <vector>
#include <map>
#include <queue>
#include <mutex>

const char* sharedMemoryName = "Dune2SharedMemory";

SOCKET clientSocket;

extern std::vector<long> g_eXList;
extern std::vector<long> g_eYList;
extern std::vector<int> g_enewIdList;
extern std::vector<int> g_etypeList;

extern std::vector<int> g_bNewId;
extern std::vector<float> g_bX;
extern std::vector<float> g_bY;
extern std::vector<float> g_bTargX;
extern std::vector<float> g_bTargY;
extern std::vector<int> g_bType;

extern std::map<int/*newId*/, float/*posX*/> g_bUpdateX;
extern std::map<int/*newId*/, float/*posY*/> g_bUpdateY;

extern std::vector<int> g_unewID;
extern std::vector<int16_t> g_uux;
extern std::vector<int16_t> g_uuy;
extern std::vector<unsigned char> g_upacked;
extern std::vector<unsigned char> g_umoving;

void SendPacket(std::vector<int>& unewID, std::vector<int16_t>& uux, std::vector<int16_t>& uuy,
  std::vector<unsigned char>& upacked, std::vector<unsigned char>& umoving,

  //  unsigned char isExplosion, int16_t ex16, int16_t ey16,
  std::vector<long>& eXList, std::vector<long>& eYList, std::vector<int>& enewIdList, std::vector<int>& etypeList,

  std::vector<int>& bNewId, std::vector<float>& bX, std::vector<float>& bY,
  std::vector<float>& bTargX, std::vector<float>& bTargY,
  std::vector<int>& bType,
  std::map<int/*newId*/, float/*posX*/>& bUpdateX, std::map<int/*newId*/, float/*posY*/>& bUpdateY);

struct Packet
{
public:
  std::vector<int> unewID;
  std::vector<int16_t> uux;
  std::vector<int16_t> uuy;
  std::vector<unsigned char> upacked; 
  std::vector<unsigned char> umoving;

  std::vector<long> eXList; 
  std::vector<long> eYList; 
  std::vector<int> enewIdList; 
  std::vector<int> etypeList;

  std::vector<int> bNewId; std::vector<float> bX; std::vector<float> bY;
  std::vector<float> bTargX; std::vector<float> bTargY; 
  std::vector<int> bType;
  std::map<int/*newId*/, float/*posX*/> bUpdateX; 
  std::map<int/*newId*/, float/*posY*/> bUpdateY;
};
struct PacketS
{
public:
  uint16_t NumUU;
  int16_t uux[300];
  int16_t uuy[300];
};
std::queue<Packet> myQueue;
//std::mutex mtx;
PacketS* g_sharedMemory;
bool g_createdSHM = false;
HANDLE hMapFile;

void CreateSharedMemory()
{
  const size_t sharedMemorySize = sizeof(PacketS);
  hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sharedMemorySize, sharedMemoryName);
  if (hMapFile == NULL) {
    std::cerr << "CreateFileMapping failed: " << GetLastError() << std::endl;
    return;
  }

    g_sharedMemory = static_cast<PacketS*>(MapViewOfFile(hMapFile, FILE_MAP_WRITE, 0, 0, sharedMemorySize));
    if (g_sharedMemory == nullptr) {
      std::cerr << "MapViewOfFile failed: " << GetLastError() << std::endl;
      CloseHandle(hMapFile);
      return;
    }

    /*
    // Write data to shared memory
    *sharedMemory = 42;

    // Read data from shared memory
    int value = *sharedMemory;
    */

    g_createdSHM = true;
}

  void UpdateSharedMemory(Packet& p)
  {
    if (!g_createdSHM)
      CreateSharedMemory();

    // Write data to shared memory
//    PacketS ps;
//    ps.NumUU = p.uux.size();

   // p.uux
    
      g_sharedMemory->NumUU = (uint16_t)p.uux.size();
      for (size_t i = 0; i < p.uux.size(); ++i)
      {
        g_sharedMemory->uux[i] = p.uux[i];
        g_sharedMemory->uuy[i] = p.uuy[i];
      }

//    if (g_sharedMemory->uux.size() > 7)
    {
 //     std::cout << "test sm[6]: " << g_sharedMemory->uux[6]; //960
    }
  }

  void RemoveSharedMemory()
  {
    if (!UnmapViewOfFile(g_sharedMemory)) {
      std::cerr << "UnmapViewOfFile failed: " << GetLastError() << std::endl;
    }
    CloseHandle(hMapFile);
  }


HANDLE hMutex;

void CreateM()
{
  hMutex = CreateMutex(NULL, FALSE, NULL);
}
void ReleaseM()
{
  ReleaseMutex(hMutex);
}
bool WaitForSingle()
{
  return WaitForSingleObject(hMutex, INFINITE) == WAIT_OBJECT_0;
}

void ReceiveAck()
{
  // Receive an integer from the client
  int receivedInt;
  int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&receivedInt), sizeof(receivedInt), 0);

  if (bytesReceived == sizeof(receivedInt)) {
    // Successfully received the integer
    receivedInt = ntohl(receivedInt); // Convert from network byte order (if needed)
//    std::cout << "Received Integer: " << receivedInt << std::endl;
  }
  else if (bytesReceived == 0) {
    // Connection closed by the client
    std::cout << "Client disconnected." << std::endl;
  }
  else {
    // Error occurred during receive
    std::cerr << "recv failed." << std::endl;
  }
  if (receivedInt == 255)
  {
    //ok
  }
  else
  {
    //bad
  }
}

// Function that will be executed by the new thread
DWORD WINAPI MyThreadFunction(LPVOID lpParam) {
  int threadNumber = *((int*)lpParam);
    std::cout << "Thread " << threadNumber << " is running." << std::endl;

    while (true)
    {
      // Lock the mutex before accessing the queue
      //std::lock_guard<std::mutex> lock(mtx);
//      DWORD result = WaitForSingleObject(hMutex, INFINITE);
      WaitForSingleObject(hMutex, INFINITE);

//      if (result == WAIT_OBJECT_0) {
        // Mutex acquired, perform critical section operations
 //       std::cout << "A Thread has acquired the mutex." << std::endl;

///      Packet p;
//      bool hasPacket = false;
      std::queue<Packet> localQueue;
      while (!myQueue.empty()) {
        //         std::cout << "B Thread queue NOT empty." << std::endl;
                 // Consume data
        Packet p = myQueue.front();
        myQueue.pop();
        localQueue.push(p);
 //       hasPacket = true;
        //std::cout << "Consumed: " << data << std::endl;
      }
      ReleaseMutex(hMutex);

      std::cout << "Local queue size: " << localQueue.size() << std::endl;
      while (!localQueue.empty())
      {
        Packet p = localQueue.front();
        localQueue.pop();
        //        std::cout << "Thread sending." << std::endl;
        SendPacket(p.unewID, p.uux, p.uuy, p.upacked, p.umoving, p.eXList, p.eYList, p.enewIdList, p.etypeList, p.bNewId, p.bX, p.bY, p.bTargX, p.bTargY, p.bType, p.bUpdateX, p.bUpdateY);
        ReceiveAck();
      }
       


//      }

//      SendPacket(g_unewID, g_uux, g_uuy, g_upacked, g_umoving, g_eXList, g_eYList, g_enewIdList, g_etypeList, g_bNewId, g_bX, g_bY, g_bTargX, g_bTargY, g_bType, g_bUpdateX, g_bUpdateY);

 

      Sleep(5);
    }


  return 0; // Return a DWORD (typically 0) to indicate success
}

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

//  int sendBufferSize = 1024*10; // Size in bytes
//  int sendBufferSize = 1073741824; //gig too big
//  int sendBufferSize = 1048576; //mb too big  
//  int sendBufferSize = 1024; //1k too big
//  int sendBufferSize = 500; //500 too big
  int sendBufferSize = 10; //500 too big
  int result = setsockopt(clientSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, sizeof(int));
  if (result == SOCKET_ERROR) {
    // Handle the error
  }
  

  HANDLE thread;


    int threadNumber = 1;
    thread = CreateThread(NULL, 0, MyThreadFunction, &threadNumber, 0, NULL);
    if (thread == NULL) {
 //     std::cerr << "Thread creation failed." << std::endl;
 //     return 1; // Return an error code
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

//extern bool g_explosion;
#include <map>

#include <chrono>
std::chrono::time_point<std::chrono::high_resolution_clock> g_timerStart;
bool g_timerEnabled = false;

int g_seq = 0;

void SendPacket(std::vector<int>& unewID, std::vector<int16_t>& uux, std::vector<int16_t>& uuy, 
  std::vector<unsigned char>& upacked, std::vector<unsigned char>& umoving,

//  unsigned char isExplosion, int16_t ex16, int16_t ey16,
std::vector<long>& eXList, std::vector<long>& eYList, std::vector<int>& enewIdList, std::vector<int>& etypeList,

  std::vector<int>& bNewId, std::vector<float>& bX, std::vector<float>& bY,
  std::vector<float>& bTargX, std::vector<float>& bTargY, 
  std::vector<int>& bType,
  std::map<int/*newId*/, float/*posX*/>& bUpdateX, std::map<int/*newId*/, float/*posY*/>& bUpdateY)
{


  unsigned char byteToSend = 0xFE;
  if (send(clientSocket, reinterpret_cast<const char*>(&byteToSend), 1, 0) == SOCKET_ERROR) {
    std::cerr << "Sending data failed." << std::endl;
    //    socketFile << "Sending data failed." << std::endl;

    if (!g_timerEnabled)
    {
      g_timerStart = std::chrono::high_resolution_clock::now();
      g_timerEnabled = true;
    }
    else
    {
      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> elapsed_seconds = end - g_timerStart;
      if (elapsed_seconds.count() > 10.0)
      {
        CloseSocket();
        InitSocket();
        g_timerEnabled = false;
      }
    }

    return;
  }
  else {
  //  std::cout << "Data sent successfully." << std::endl;
    //   socketFile << "Data sent successfully." << std::endl;
  }

  //int sendBufferSize = 1024; // Size in bytes
  //int result = setsockopt(clientSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, sizeof(int));
  //if (result == SOCKET_ERROR) {
  //  // Handle the error
  //}

  g_seq++;
  if (send(clientSocket, reinterpret_cast<const char*>(&g_seq), sizeof(g_seq), 0) == SOCKET_ERROR) {
    std::cerr << "Sending data failed." << std::endl;
    //    socketFile << "Sending data failed." << std::endl;
  }

  int unewIdSize = unewID.size();
  if (send(clientSocket, reinterpret_cast<const char*>(&unewIdSize), sizeof(unewIdSize), 0) == SOCKET_ERROR) {
    std::cerr << "Sending data failed." << std::endl;
    //    socketFile << "Sending data failed." << std::endl;
  }
  for (unsigned int i = 0; i < unewID.size(); ++i)
  {
    int id = unewID[i];
    if (send(clientSocket, reinterpret_cast<const char*>(&id), sizeof(id), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }

    int16_t ux = uux[id];
    int16_t uy = uuy[id];
    unsigned char packed = upacked[id];
    unsigned char moving = umoving[id];
    if (send(clientSocket, reinterpret_cast<const char*>(&ux), sizeof(ux), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
    if (send(clientSocket, reinterpret_cast<const char*>(&uy), sizeof(uy), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
    if (send(clientSocket, reinterpret_cast<const char*>(&packed), sizeof(packed), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
    if (send(clientSocket, reinterpret_cast<const char*>(&moving), sizeof(moving), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
  }


 // if (send(clientSocket, reinterpret_cast<const char*>(&ux16), sizeof(ux16), 0) == SOCKET_ERROR) {
 //   std::cerr << "Sending data failed." << std::endl;
 //   //    socketFile << "Sending data failed." << std::endl;
 // }
 // else {
 // //  std::cout << "Data sent successfully." << std::endl;
 //   //   socketFile << "Data sent successfully." << std::endl;
 // }
 // if (send(clientSocket, reinterpret_cast<const char*>(&uy16), sizeof(uy16), 0) == SOCKET_ERROR) {
 //   std::cerr << "Sending data failed." << std::endl;
 //   //    socketFile << "Sending data failed." << std::endl;
 // }
 // else {
 // //  std::cout << "Data sent successfully." << std::endl;
 //   //   socketFile << "Data sent successfully." << std::endl;
 // }
 // if (send(clientSocket, reinterpret_cast<const char*>(&packedFacing), sizeof(packedFacing), 0) == SOCKET_ERROR) {
 //   std::cerr << "Sending data failed." << std::endl;
 //   //    socketFile << "Sending data failed." << std::endl;
 // }
 // else {
 ////   std::cout << "Data sent successfully." << std::endl;
 //   //   socketFile << "Data sent successfully." << std::endl;
 // }
 // if (send(clientSocket, reinterpret_cast<const char*>(&moving), sizeof(moving), 0) == SOCKET_ERROR) {
 //   std::cerr << "Sending data failed." << std::endl;
 //   //    socketFile << "Sending data failed." << std::endl;
 // }
 // else {
 // //  std::cout << "Data sent successfully." << std::endl;
 //   //   socketFile << "Data sent successfully." << std::endl;
 // }


//  if (isExplosion)
//  {
//    if (send(clientSocket, reinterpret_cast<const char*>(&isExplosion), sizeof(isExplosion), 0) == SOCKET_ERROR) {
//      std::cerr << "Sending data failed." << std::endl;
//      //    socketFile << "Sending data failed." << std::endl;
//    }
////    g_explosion = false;
//  }
//  else
//  {
//    if (send(clientSocket, reinterpret_cast<const char*>(&isExplosion), sizeof(isExplosion), 0) == SOCKET_ERROR) {
//      std::cerr << "Sending data failed." << std::endl;
//      //    socketFile << "Sending data failed." << std::endl;
//    }
//  }
//  if (send(clientSocket, reinterpret_cast<const char*>(&ex16), sizeof(ex16), 0) == SOCKET_ERROR) {
//    std::cerr << "Sending data failed." << std::endl;
//    //    socketFile << "Sending data failed." << std::endl;
//  }
//  if (send(clientSocket, reinterpret_cast<const char*>(&ey16), sizeof(ey16), 0) == SOCKET_ERROR) {
//    std::cerr << "Sending data failed." << std::endl;
//    //    socketFile << "Sending data failed." << std::endl;
//  }

  //std::vector<long>& eXList, std::vector<long>& eYList, std::vector<int>& enewIdList, std::vector<int>& etypeList,
  int eSize = eXList.size();
  if (send(clientSocket, reinterpret_cast<const char*>(&eSize), sizeof(eSize), 0) == SOCKET_ERROR) {
    std::cerr << "Sending data failed." << std::endl;
    //    socketFile << "Sending data failed." << std::endl;
  }
  for (int i = 0; i < eSize; ++i)
  {
    int v = (int)eXList[i];
    if (send(clientSocket, reinterpret_cast<const char*>(&v), sizeof(v), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
    v = (int)eYList[i];
    if (send(clientSocket, reinterpret_cast<const char*>(&v), sizeof(v), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
    int iv = enewIdList[i];
    if (send(clientSocket, reinterpret_cast<const char*>(&iv), sizeof(iv), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
    iv = etypeList[i];
    if (send(clientSocket, reinterpret_cast<const char*>(&iv), sizeof(iv), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
  }

  //max size is 300
  int bulletSize = bNewId.size();
  if (send(clientSocket, reinterpret_cast<const char*>(&bulletSize), sizeof(bulletSize), 0) == SOCKET_ERROR) {
    std::cerr << "Sending data failed." << std::endl;
    //    socketFile << "Sending data failed." << std::endl;
  }
  for (int i = 0; i < bulletSize; ++i)
  {
    int newId = bNewId[i];
    float bulletX = bX[i];
    float bulletY = bY[i];
    float bulletTargetX = bTargX[i];
    float bulletTargetY = bTargY[i];
    int bulletType = bType[i];
    if (send(clientSocket, reinterpret_cast<const char*>(&newId), sizeof(newId), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
    if (send(clientSocket, reinterpret_cast<const char*>(&bulletX), sizeof(bulletX), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
    if (send(clientSocket, reinterpret_cast<const char*>(&bulletY), sizeof(bulletY), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
    if (send(clientSocket, reinterpret_cast<const char*>(&bulletTargetX), sizeof(bulletTargetX), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
    if (send(clientSocket, reinterpret_cast<const char*>(&bulletTargetY), sizeof(bulletTargetY), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
    if (send(clientSocket, reinterpret_cast<const char*>(&bulletType), sizeof(bulletType), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
  }
  bNewId.clear();
  bX.clear();
  bY.clear();
  bTargX.clear();
  bTargY.clear();
  bType.clear();

  int numBulletXUpdates = bUpdateX.size();
  if (send(clientSocket, reinterpret_cast<const char*>(&numBulletXUpdates), sizeof(numBulletXUpdates), 0) == SOCKET_ERROR) {
    std::cerr << "Sending data failed." << std::endl;
    //    socketFile << "Sending data failed." << std::endl;
  }
  for (auto it = bUpdateX.begin(); it != bUpdateX.end(); ++it) {
    int key = it->first;
    float value = it->second;
    if (send(clientSocket, reinterpret_cast<const char*>(&key), sizeof(key), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
    if (send(clientSocket, reinterpret_cast<const char*>(&value), sizeof(value), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
  }
  int numBulletYUpdates = bUpdateY.size();
  if (send(clientSocket, reinterpret_cast<const char*>(&numBulletYUpdates), sizeof(numBulletYUpdates), 0) == SOCKET_ERROR) {
    std::cerr << "Sending data failed." << std::endl;
    //    socketFile << "Sending data failed." << std::endl;
  }
  for (auto it = bUpdateY.begin(); it != bUpdateY.end(); ++it) {
    int key = it->first;
    float value = it->second;
    if (send(clientSocket, reinterpret_cast<const char*>(&key), sizeof(key), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
    if (send(clientSocket, reinterpret_cast<const char*>(&value), sizeof(value), 0) == SOCKET_ERROR) {
      std::cerr << "Sending data failed." << std::endl;
      //    socketFile << "Sending data failed." << std::endl;
    }
  }
  // or clear when bullet dies?
  bUpdateX.clear();
  bUpdateY.clear();
}

void CloseSocket()
{
  // Clean up and close the socket
  closesocket(clientSocket);
  WSACleanup();

}