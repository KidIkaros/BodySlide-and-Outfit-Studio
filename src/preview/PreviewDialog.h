/*
BodySlide and Outfit Studio

Three.js Preview Integration - Preview Dialog

A wxWidgets dialog window that embeds a web view for
the Three.js 3D previewer.
*/

#pragma once

#include <memory>
#include <string>

// Forward declarations
class wxWebView;
class wxBoxSizer;

namespace preview {
	class WebPreviewServer;
	class MeshSerializer;
}

namespace preview {

enum class PreviewMode {
	Standalone,      // Opens preview HTML in system browser
	Embedded,        // Embedded web view in dialog
	ExportGLTF       // Export to GLTF file and open
};

class PreviewDialog {
public:
	PreviewDialog();
	~PreviewDialog();
	
	// Show the preview dialog
	bool Show(const std::string& meshJson = std::string());
	
	// Close the preview dialog
	void Close();
	
	// Update mesh data (for live preview)
	void UpdateMesh(const std::string& meshJson);
	
	// Set preview mode
	void SetMode(PreviewMode m) { mode = m; }
	
	// Open standalone preview in browser
	static void OpenInBrowser(const std::string& meshJson = std::string());
	
	// Export mesh to GLTF file
	static bool ExportToFile(const std::string& filePath, const std::string& meshJson);
	
private:
	// Platform-specific web view initialization
	bool CreateWebView();
	std::string GetHtmlPath();
	
	PreviewMode mode;
	std::string currentMeshJson;
	std::unique_ptr<WebPreviewServer> server;
};

} // namespace preview