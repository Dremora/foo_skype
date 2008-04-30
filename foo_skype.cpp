#include "../SDK/foobar2000.h"

#define COMPONENT_TITLE "Skype now playing notifications"
#define NOW_PLAYING_TEXT "Now playing: [%artist% - ]%title%"
#define PAUSED_TEXT "Paused: [%artist% - ]%title%"
#define STOPPED_TEXT "Stopped."
#define FB2K_NOT_STARTED "foobar2000 is not started."

DECLARE_COMPONENT_VERSION(COMPONENT_TITLE, "0.1 beta 2", "Copyright (C) 2008 Dremora");

HWND GlobalMainWindowHandle;
HWND GlobalSkypeAPIWindowHandle = 0;
unsigned int GlobalSkypeControlAPIAttachMsg;
unsigned int GlobalSkypeControlAPIDiscoverMsg;
wchar_t WindowClassName[] = L"foobar2000_skype_class";

enum {
	SKYPECONTROLAPI_ATTACH_SUCCESS,               // Client is successfully attached and API window handle can be found in wParam parameter
	SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION, // Skype has acknowledged connection request and is waiting for confirmation from the user. The client is not yet attached and should wait for SKYPECONTROLAPI_ATTACH_SUCCESS message
	SKYPECONTROLAPI_ATTACH_REFUSED,               // User has explicitly denied access to client
	SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE,         // API is not available at the moment. For example, this happens when no user is currently logged in. Client should wait for SKYPECONTROLAPI_ATTACH_API_AVAILABLE broadcast before making any further connection attempts.
	SKYPECONTROLAPI_ATTACH_API_AVAILABLE         = 0x8001
};

void skype_send(char *title) {
	if (!GlobalSkypeAPIWindowHandle) return;
	COPYDATASTRUCT CopyData;
	CopyData.dwData = 0;
	CopyData.cbData = 23 + strlen(title);
	CopyData.lpData = new char[CopyData.cbData];
	strcpy_s((char *)CopyData.lpData, CopyData.cbData, "SET PROFILE MOOD_TEXT ");
	strcat_s((char *)CopyData.lpData, CopyData.cbData, title);
	if (!SendMessage(GlobalSkypeAPIWindowHandle, WM_COPYDATA, (WPARAM)GlobalMainWindowHandle, (LPARAM)&CopyData)) {
		GlobalSkypeAPIWindowHandle = 0;
		console::printf(COMPONENT_TITLE ": Disconnected.");
	}
	delete CopyData.lpData;
}

void skype_stopped() { 
	if (!GlobalSkypeAPIWindowHandle) return;
	skype_send(STOPPED_TEXT);
}

void skype_now_playing() {
	if (!GlobalSkypeAPIWindowHandle) return;
	static_api_ptr_t<playback_control> pc;
	static_api_ptr_t<titleformat_compiler> tc;
	service_ptr_t<titleformat_object> script;
	pfc::string8 text;
	if (pc->is_paused()) tc->compile_safe(script, PAUSED_TEXT);
	else tc->compile_safe(script, NOW_PLAYING_TEXT);
	if (!pc->playback_format_title(0, text, script, 0, playback_control::display_level_titles)) {
		skype_stopped();
		return;
	}
	char *title = new char[text.get_length()+1];
	strcpy_s(title, text.get_length()+1, text);
	skype_send(title);
	delete title;
}

void skype_connect() {
	PostMessage(HWND_BROADCAST, GlobalSkypeControlAPIDiscoverMsg, (WPARAM)GlobalMainWindowHandle, 0);
}
static LRESULT __stdcall WindowProc(HWND hWindow, UINT uiMessage, WPARAM uiParam, LPARAM ulParam) {
	switch (uiMessage) {
		case WM_COPYDATA:
			if (GlobalSkypeAPIWindowHandle == (HWND)uiParam) return 1;
			break;
		default:
			if (uiMessage == GlobalSkypeControlAPIAttachMsg) {
				switch (ulParam) {
					case SKYPECONTROLAPI_ATTACH_SUCCESS:
					{
						GlobalSkypeAPIWindowHandle = (HWND)uiParam;
						static_api_ptr_t<playback_control> pc;
						if (pc->is_playing()) skype_now_playing();
						else skype_stopped();
						console::printf(COMPONENT_TITLE ": Connection successful.");
						break;
					}
					case SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION:
						console::printf(COMPONENT_TITLE ": Pending authorization.");
						break;
					case SKYPECONTROLAPI_ATTACH_REFUSED:
						console::printf(COMPONENT_TITLE ": Connection refused.");
						break;
					case SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE:
						console::printf(COMPONENT_TITLE ": Skype API is not available, waiting.");
						break;
					case SKYPECONTROLAPI_ATTACH_API_AVAILABLE:
						console::printf(COMPONENT_TITLE ": Skype API is now available.");
						/*if (!GlobalSkypeAPIWindowHandle) */skype_connect();
						break;
				}
				return 0;
			} else return DefWindowProc(hWindow, uiMessage, uiParam, ulParam);
	}
}

class skype_initquit : public initquit {
	virtual void on_init() {
		GlobalSkypeControlAPIAttachMsg = RegisterWindowMessage(L"SkypeControlAPIAttach");
		GlobalSkypeControlAPIDiscoverMsg = RegisterWindowMessage(L"SkypeControlAPIDiscover");
		WNDCLASS WindowClass = {};
		WindowClass.lpfnWndProc = WindowProc;
		WindowClass.hInstance = core_api::get_my_instance();
		WindowClass.lpszClassName = WindowClassName;
		RegisterClass(&WindowClass);
		GlobalMainWindowHandle = CreateWindowEx(0, WindowClassName, 0, 0, 0, 0, 0, 0, 0, 0, core_api::get_my_instance(), 0);
		skype_connect();
	}
	virtual void on_quit() {
		skype_send(FB2K_NOT_STARTED);
		UnregisterClass(WindowClassName, core_api::get_my_instance());
	}
};

static initquit_factory_t<skype_initquit> foo_initquit;

class skype_play_callback_static : public play_callback_static {
	void on_playback_new_track(metadb_handle_ptr p_track) { skype_now_playing(); }
	void on_playback_edited(metadb_handle_ptr p_track) { skype_now_playing(); }
	void on_playback_dynamic_info_track(const file_info & p_info) { skype_now_playing(); }
	void on_playback_pause(bool p_state) { skype_now_playing(); }
	void on_playback_stop(play_control::t_stop_reason reason) { skype_stopped(); }

	void on_playback_starting(play_control::t_track_command p_command,bool p_paused) {}
	void on_playback_seek(double p_time) {}
	void on_playback_time(double p_time) {}
	void on_playback_dynamic_info(const file_info & p_info) {}
	void on_volume_change(float p_new_val) {}

	unsigned get_flags() { return flag_on_playback_new_track|flag_on_playback_stop|flag_on_playback_pause|flag_on_playback_edited|flag_on_playback_dynamic_info_track; }
};

static play_callback_static_factory_t<skype_play_callback_static> foo_play_callback_static;
