#include "../SDK/foobar2000.h"

#define COMPONENT_TITLE "Skype playing notifications"
#define COMPONENT_DLL_NAME "foo_skype"
#define COMPONENT_VERSION "0.1 beta 2"

DECLARE_COMPONENT_VERSION(COMPONENT_TITLE, COMPONENT_VERSION, "Copyright (C) 2008 Dremora");

static HWND GlobalMainWindowHandle;
static HWND GlobalSkypeAPIWindowHandle = 0;
static unsigned int GlobalSkypeControlAPIAttachMsg;
static unsigned int GlobalSkypeControlAPIDiscoverMsg;
static wchar_t WindowClassName[] = L"foobar2000_skype_class";

static const GUID guid_skype_cfg_branch = { 0x9ae6ec18, 0xcb9c, 0x4998, { 0x9c, 0x86, 0x1f, 0xa9, 0x4d, 0xf3, 0xfe, 0xd1 } };
static advconfig_branch_factory skype_cfg_branch(COMPONENT_TITLE, guid_skype_cfg_branch, advconfig_entry::guid_branch_display, 10);

static const GUID guid_skype_cfg_playing = { 0x0e646745, 0x9062, 0x4775, { 0x87, 0x81, 0xcb, 0xfd, 0x38, 0x73, 0x67, 0x49 } };
static advconfig_string_factory skype_cfg_playing("Playing", guid_skype_cfg_playing, guid_skype_cfg_branch, 1, "playing: [%artist% - ]%title%");

static const GUID guid_skype_cfg_paused = { 0x79b6576b, 0x9dc0, 0x4866, { 0xb2, 0xaf, 0x21, 0xa, 0x89, 0xd6, 0x2c, 0x53 } };
static advconfig_string_factory skype_cfg_paused("Paused", guid_skype_cfg_paused, guid_skype_cfg_branch, 2, "paused: [%artist% - ]%title%");

static const GUID guid_skype_cfg_stopped = { 0x78ac024d, 0x32a3, 0x40b2, { 0x88, 0x66, 0x9b, 0x5b, 0x7b, 0x2a, 0x9f, 0xc3 } };
static advconfig_string_factory skype_cfg_stopped("Stopped", guid_skype_cfg_stopped, guid_skype_cfg_branch, 3, COMPONENT_DLL_NAME " v" COMPONENT_VERSION);

//static const GUID guid_skype_fb2k_not_started = { 0x6575e95f, 0x4e85, 0x46bf, { 0x81, 0xa3, 0xe6, 0xb1, 0x9e, 0x5f, 0x61, 0xc2 } };
//static advconfig_string_factory skype_fb2k_not_started("foobar2000 is not started", guid_skype_fb2k_not_started, guid_skype_cfg_branch, 4, "foobar2000 is not started.");

enum {
	SKYPECONTROLAPI_ATTACH_SUCCESS,               // Client is successfully attached and API window handle can be found in wParam parameter
	SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION, // Skype has acknowledged connection request and is waiting for confirmation from the user. The client is not yet attached and should wait for SKYPECONTROLAPI_ATTACH_SUCCESS message
	SKYPECONTROLAPI_ATTACH_REFUSED,               // User has explicitly denied access to client
	SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE,         // API is not available at the moment. For example, this happens when no user is currently logged in. Client should wait for SKYPECONTROLAPI_ATTACH_API_AVAILABLE broadcast before making any further connection attempts.
	SKYPECONTROLAPI_ATTACH_API_AVAILABLE         = 0x8001
};

void skype_send(const char *title) {
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
	service_ptr_t<advconfig_entry_string_impl> stop;
	pfc::string8 str;
	skype_cfg_stopped.get_static_instance().get_state(str);
	skype_send(str);
}

void skype_playing() {
	if (!GlobalSkypeAPIWindowHandle) return;
	static_api_ptr_t<playback_control> pc;
	static_api_ptr_t<titleformat_compiler> tc;
	service_ptr_t<titleformat_object> script;
	pfc::string8 str, text;
	if (pc->is_paused()) skype_cfg_paused.get_static_instance().get_state(str);
	else skype_cfg_playing.get_static_instance().get_state(str);
	tc->compile_safe(script, str);
	if (!pc->playback_format_title(0, text, script, 0, playback_control::display_level_titles)) {
		skype_stopped();
		return;
	}
	skype_send(text);
}

void skype_connect() {
	PostMessage(HWND_BROADCAST, GlobalSkypeControlAPIDiscoverMsg, (WPARAM)GlobalMainWindowHandle, 0);
}
static LRESULT __stdcall WindowProc(HWND hWindow, UINT uiMessage, WPARAM uiParam, LPARAM ulParam) {
	if (uiMessage == WM_COPYDATA && GlobalSkypeAPIWindowHandle == (HWND)uiParam) return 1;
	else if (uiMessage == GlobalSkypeControlAPIAttachMsg) {
		switch (ulParam) {
			case SKYPECONTROLAPI_ATTACH_SUCCESS:
			{
				GlobalSkypeAPIWindowHandle = (HWND)uiParam;
				static_api_ptr_t<playback_control> pc;
				if (pc->is_playing()) skype_playing();
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
		//pfc::string8 str;
		//skype_cfg_stopped.get_static_instance().get_state(str);
		//skype_send(str);
		UnregisterClass(WindowClassName, core_api::get_my_instance());
	}
};

static initquit_factory_t<skype_initquit> foo_initquit;

class skype_play_callback_static : public play_callback_static {
	void on_playback_new_track(metadb_handle_ptr p_track) { skype_playing(); }
	void on_playback_edited(metadb_handle_ptr p_track) { skype_playing(); }
	void on_playback_dynamic_info_track(const file_info & p_info) { skype_playing(); }
	void on_playback_pause(bool p_state) { skype_playing(); }
	void on_playback_stop(play_control::t_stop_reason reason) { skype_stopped(); }

	void on_playback_starting(play_control::t_track_command p_command,bool p_paused) {}
	void on_playback_seek(double p_time) {}
	void on_playback_time(double p_time) {}
	void on_playback_dynamic_info(const file_info & p_info) {}
	void on_volume_change(float p_new_val) {}

	unsigned get_flags() { return flag_on_playback_new_track|flag_on_playback_stop|flag_on_playback_pause|flag_on_playback_edited|flag_on_playback_dynamic_info_track; }
};

static play_callback_static_factory_t<skype_play_callback_static> foo_play_callback_static;
