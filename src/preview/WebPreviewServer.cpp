/*
BodySlide and Outfit Studio

Three.js Preview Integration - Web Preview Server

Provides a lightweight HTTP server to serve the Three.js viewer
and handle WebSocket connections for live mesh updates.
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#define closesocket close
#define SOCKET int
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#endif

#include <preview/WebPreviewServer.h>

namespace preview {

WebPreviewServer::WebPreviewServer(int port)
	: port(port), running(false), stopRequested(false) {
#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
	htmlPath = "preview/index.html";
}

WebPreviewServer::~WebPreviewServer() {
	Stop();
#ifdef _WIN32
	WSACleanup();
#endif
}

bool WebPreviewServer::Start() {
	if (running.load()) {
		return true;
	}

	stopRequested = false;
	serverThread = std::thread(&WebPreviewServer::ServerThread, this);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	return running.load();
}

void WebPreviewServer::Stop() {
	if (!running.load()) {
		return;
	}

	stopRequested = true;

	{
		std::lock_guard<std::mutex> lock(clientsMutex);
		for (auto client : clients) {
#ifdef _WIN32
			closesocket(client);
#else
			::close(client);
#endif
		}
		clients.clear();
	}

	if (serverThread.joinable()) {
		serverThread.join();
	}

	running = false;
}

std::string WebPreviewServer::GetPreviewUrl() const {
	std::ostringstream url;
	url << "http://localhost:" << port;
	return url.str();
}

void WebPreviewServer::BroadcastMeshUpdate(const std::string& meshJson) {
	std::lock_guard<std::mutex> lock(clientsMutex);

	for (auto client : clients) {
		std::string frame;
		frame += char(0x81);
		size_t len = meshJson.size();
		if (len < 126) {
			frame += char(len);
		} else if (len < 65536) {
			frame += char(0x7E);
			frame += char((len >> 8) & 0xFF);
			frame += char(len & 0xFF);
		} else {
			frame += char(0x7F);
			frame += char(0);
			frame += char(0);
			frame += char(0);
			frame += char(0);
			frame += char((len >> 24) & 0xFF);
			frame += char((len >> 16) & 0xFF);
			frame += char((len >> 8) & 0xFF);
			frame += char(len & 0xFF);
		}
		frame += meshJson;

#ifdef _WIN32
		send(client, frame.c_str(), static_cast<int>(frame.size()), 0);
#else
		send(client, frame.c_str(), frame.size(), 0);
#endif
	}
}

void WebPreviewServer::SetMeshData(const std::string& meshJson) {
	std::lock_guard<std::mutex> lock(meshMutex);
	meshData = meshJson;
}

int WebPreviewServer::GetClientCount() const {
	std::lock_guard<std::mutex> lock(clientsMutex);
	return static_cast<int>(clients.size());
}

void WebPreviewServer::SetHtmlPath(const std::string& path) {
	htmlPath = path;
}

void WebPreviewServer::ServerThread() {
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET) {
		std::cerr << "Failed to create socket" << std::endl;
		return;
	}

	int opt = 1;
#ifdef _WIN32
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(static_cast<short>(port));

	if (bind(serverSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		std::cerr << "Failed to bind to port " << port << std::endl;
#ifdef _WIN32
		closesocket(serverSocket);
#else
		::close(serverSocket);
#endif
		return;
	}

	if (listen(serverSocket, 5) == SOCKET_ERROR) {
		std::cerr << "Listen failed" << std::endl;
#ifdef _WIN32
		closesocket(serverSocket);
#else
		::close(serverSocket);
#endif
		return;
	}

	running = true;
	std::cout << "Web preview server started on port " << port << std::endl;

	while (!stopRequested.load()) {
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(serverSocket, &readSet);

		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		int activity = select(serverSocket + 1, &readSet, nullptr, nullptr, &timeout);
		if (activity < 0) {
			break;
		}

		if (FD_ISSET(serverSocket, &readSet)) {
			sockaddr_in clientAddr;
			socklen_t clientLen = sizeof(clientAddr);
			SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);

			if (clientSocket != INVALID_SOCKET) {
				HandleClient(clientSocket);
			}
		}
	}

#ifdef _WIN32
	closesocket(serverSocket);
#else
	::close(serverSocket);
#endif

	running = false;
}

void WebPreviewServer::HandleClient(int clientSocket) {
	char buffer[4096];
	memset(buffer, 0, sizeof(buffer));

	int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead <= 0) {
#ifdef _WIN32
		closesocket(clientSocket);
#else
		::close(clientSocket);
#endif
		return;
	}

	std::string request(buffer, bytesRead);

	if (request.find("Upgrade: websocket") != std::string::npos ||
		request.find("upgrade: websocket") != std::string::npos) {
		HandleWebSocket(clientSocket);
		return;
	}

	std::istringstream iss(request);
	std::string method, path, version;
	iss >> method >> path >> version;

	std::string response = GetHttpResponse(path);

	send(clientSocket, response.c_str(), static_cast<int>(response.size()), 0);

#ifdef _WIN32
	closesocket(clientSocket);
#else
	::close(clientSocket);
#endif
}

void WebPreviewServer::HandleWebSocket(int clientSocket) {
	std::string upgrade = "HTTP/1.1 101 Switching Protocols\r\n";
	upgrade += "Upgrade: websocket\r\n";
	upgrade += "Connection: Upgrade\r\n";
	upgrade += "Sec-WebSocket-Accept: \r\n";
	upgrade += "\r\n\r\n";

	send(clientSocket, upgrade.c_str(), static_cast<int>(upgrade.size()), 0);

	{
		std::lock_guard<std::mutex> lock(clientsMutex);
		clients.push_back(clientSocket);
	}

	{
		std::lock_guard<std::mutex> lock(meshMutex);
		if (!meshData.empty()) {
			BroadcastMeshUpdate(meshData);
		}
	}

	char buffer[4096];
	while (!stopRequested.load()) {
		int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
		if (bytesRead <= 0) {
			break;
		}
		if ((buffer[0] & 0x0F) == 0x88) {
			break;
		}
	}

	{
		std::lock_guard<std::mutex> lock(clientsMutex);
		auto it = std::find(clients.begin(), clients.end(), clientSocket);
		if (it != clients.end()) {
			clients.erase(it);
		}
	}

#ifdef _WIN32
	closesocket(clientSocket);
#else
	::close(clientSocket);
#endif
}

std::string WebPreviewServer::GetHttpResponse(const std::string& path) {
	std::string filePath = htmlPath;
	if (path != "/" && !path.empty()) {
		filePath = path.substr(1);
	}

	std::string content = ReadFile(filePath);

	if (content.empty()) {
		std::string notFound = "HTTP/1.1 404 Not Found\r\n";
		notFound += "Content-Type: text/html\r\n";
		notFound += "Content-Length: 13\r\n";
		notFound += "\r\n";
		notFound += "404 Not Found";
		return notFound;
	}

	std::string contentType = "text/html";
	if (filePath.find(".js") != std::string::npos) {
		contentType = "application/javascript";
	} else if (filePath.find(".css") != std::string::npos) {
		contentType = "text/css";
	} else if (filePath.find(".json") != std::string::npos) {
		contentType = "application/json";
	} else if (filePath.find(".gltf") != std::string::npos || filePath.find(".glb") != std::string::npos) {
		contentType = "model/gltf+json";
	}

	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n";
	response << "Content-Type: " << contentType << "\r\n";
	response << "Content-Length: " << content.size() << "\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";
	response << content;

	return response.str();
}

std::string WebPreviewServer::ReadFile(const std::string& path) {
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open()) {
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

} // namespace preview
