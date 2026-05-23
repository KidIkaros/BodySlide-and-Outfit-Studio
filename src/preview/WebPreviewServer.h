/*
BodySlide and Outfit Studio

Three.js Preview Integration - Web Preview Server

Provides a lightweight HTTP server to serve the Three.js viewer
and handle WebSocket connections for live mesh updates.
*/

#pragma once

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>

namespace preview {
	class MeshSerializer;
}

namespace preview {

class WebPreviewServer {
public:
	WebPreviewServer(int port = 8765);
	~WebPreviewServer();
	
	// Start/stop the server
	bool Start();
	void Stop();
	bool IsRunning() const { return running.load(); }
	
	// Get the port
	int GetPort() const { return port; }
	
	// Get the URL for the preview
	std::string GetPreviewUrl() const;
	
	// Send mesh update to all connected WebSocket clients
	void BroadcastMeshUpdate(const std::string& meshJson);
	
	// Set the mesh data to send when client connects
	void SetMeshData(const std::string& meshJson);
	
	// Get number of connected clients
	int GetClientCount() const;
	
	// Set the HTML file path
	void SetHtmlPath(const std::string& path);
	
private:
	void ServerThread();
	void HandleClient(int clientSocket);
	void HandleWebSocket(int clientSocket);
	std::string GetHttpResponse(const std::string& path);
	std::string ReadFile(const std::string& path);
	
	int port;
	std::atomic<bool> running;
	std::atomic<bool> stopRequested;
	std::thread serverThread;
	
	std::string htmlPath;
	std::string meshData;
	std::mutex meshMutex;
	
	std::vector<int> clients;
	mutable std::mutex clientsMutex;
};

} // namespace preview