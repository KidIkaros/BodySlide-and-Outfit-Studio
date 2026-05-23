/*
BodySlide and Outfit Studio

Three.js Preview Integration - Preview Dialog

A wxWidgets dialog window that embeds a web view for
the Three.js 3D previewer.
*/

#include <iostream>

#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/webview.h>

#ifdef __WXGTK__
#include <webkit2/webkit2.h>
#elif defined(__WXMSW__)
#include <wx/msw/webview.h>
#elif defined(__WXMAC__)
#include <wx/osx/webview.h>
#endif

#include <preview/PreviewDialog.h>
#include <preview/WebPreviewServer.h>
#include <preview/MeshSerializer.h>

namespace preview {

PreviewDialog::PreviewDialog()
	: mode(PreviewMode::Embedded) {
	server = std::make_unique<WebPreviewServer>(8765);
}

PreviewDialog::~PreviewDialog() {
	Close();
}

bool PreviewDialog::Show(const std::string& meshJson) {
	currentMeshJson = meshJson;
	
	if (mode == PreviewMode::Standalone) {
		OpenInBrowser(meshJson);
		return true;
	}
	
	if (mode == PreviewMode::ExportGLTF) {
		// Export and open externally
		return false;
	}
	
	// Embedded mode - start server and show dialog
	if (!server->IsRunning()) {
		if (!server->Start()) {
			std::cerr << \"Failed to start preview server\" << std::endl;
			return false;
		}
	}
	
	// Send mesh data to server
	if (!meshJson.empty()) {
		server->SetMeshData(meshJson);
	}
	
	// Create and show the dialog
	wxDialog* dialog = new wxDialog(nullptr, wxID_ANY, 
		_('PolyForge 3D Preview'), 
		wxDefaultPosition, 
		wxSize(1280, 800),
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	
	// Create web view
	wxWebView* webView = wxWebView::New(dialog, wxID_ANY, server->GetPreviewUrl());
	
	// Create button sizer at bottom
	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton* closeBtn = new wxButton(dialog, wxID_CLOSE, _('Close'));
	wxButton* refreshBtn = new wxButton(dialog, wxID_ANY, _('Refresh'));
	wxStaticText* statusText = new wxStaticText(dialog, wxID_ANY, 
		_('Connected clients: 0'));
	
	buttonSizer->Add(refreshBtn, 0, wxALL, 5);
	buttonSizer->Add(statusText, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
	buttonSizer->AddStretchSpacer();
	buttonSizer->Add(closeBtn, 0, wxALL, 5);
	
	// Main vertical sizer
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(webView, 1, wxEXPAND | wxALL, 0);
	mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);
	
	dialog->SetSizer(mainSizer);
	
	// Button events
	dialog->Bind(wxEVT_BUTTON, [dialog](wxCommandEvent&) {
		dialog->EndModal(wxID_CLOSE);
	}, wxID_CLOSE);
	
	refreshBtn->Bind(wxEVT_BUTTON, [webView, this](wxCommandEvent&) {
		webView->Reload();
		statusText->SetLabel(wxString::Format(_('Connected clients: %d'), 
			server->GetClientCount()));
	});
	
	// Update status periodically (would need a timer in real implementation)
	
	dialog->ShowModal();
	
	// Cleanup
	dialog->Destroy();
	server->Stop();
	
	return true;
}

void PreviewDialog::Close() {
	if (server && server->IsRunning()) {
		server->Stop();
	}
}

void PreviewDialog::UpdateMesh(const std::string& meshJson) {
	currentMeshJson = meshJson;
	if (server && server->IsRunning()) {
		server->BroadcastMeshUpdate(meshJson);
	}
}

void PreviewDialog::OpenInBrowser(const std::string& meshJson) {
	// For standalone mode, create a temporary HTML file with embedded data
	// or use the existing preview/index.html with WebSocket connection
	
	if (!meshJson.empty()) {
		// Start server with mesh data
		if (!server->IsRunning()) {
			server->Start();
		}
		server->SetMeshData(meshJson);
	}
	
	// Open in default browser
	wxString url = server->IsRunning() ? 
		wxString(server->GetPreviewUrl().c_str()) : 
		wxString::Format(wxT(\"file://%s/preview/index.html\"), 
			wxStandardPaths::Get().GetExecutablePath().wx_str());
	
	wxLaunchDefaultBrowser(url);
}

bool PreviewDialog::ExportToFile(const std::string& filePath, const std::string& meshJson) {
	// Write mesh JSON to GLTF file
	std::ofstream file(filePath);
	if (!file.is_open()) {
		return false;
	}
	file << meshJson;
	file.close();
	
	// Open in system viewer
	wxLaunchDefaultBrowser(wxString(wxT(\"file://\") + filePath));
	
	return true;
}

std::string PreviewDialog::GetHtmlPath() {
	wxStandardPaths& paths = wxStandardPaths::Get();
	wxString exePath = paths.GetExecutablePath();
	
	// Assume HTML is next to executable or in preview/ subdirectory
	wxString htmlPath = exePath + \"/preview/index.html\";
	if (!wxFileExists(htmlPath)) {
		htmlPath = exePath + \"/../preview/index.html\";
	}
	
	return htmlPath.ToStdString();
}

} // namespace preview