#include "foo_skype.h"

using namespace foo_skype;

DECLARE_COMPONENT_VERSION(COMPONENT_TITLE, COMPONENT_VERSION, "Copyright (C) 2008 Dremora");

static HWND GlobalMainWindowHandle;
static HWND GlobalSkypeAPIWindowHandle = 0;
static unsigned int GlobalSkypeControlAPIAttachMsg;
static unsigned int GlobalSkypeControlAPIDiscoverMsg;
static wchar_t WindowClassName[] = L"foobar2000_skype_class";

enum {
	SKYPECONTROLAPI_ATTACH_SUCCESS,               // Client is successfully attached and API window handle can be found in wParam parameter
	SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION, // Skype has acknowledged connection request and is waiting for confirmation from the user. The client is not yet attached and should wait for SKYPECONTROLAPI_ATTACH_SUCCESS message
	SKYPECONTROLAPI_ATTACH_REFUSED,               // User has explicitly denied access to client
	SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE,         // API is not available at the moment. For example, this happens when no user is currently logged in. Client should wait for SKYPECONTROLAPI_ATTACH_API_AVAILABLE broadcast before making any further connection attempts.
	SKYPECONTROLAPI_ATTACH_API_AVAILABLE         = 0x8001
};

void skype_connect() {
	PostMessage(HWND_BROADCAST, GlobalSkypeControlAPIDiscoverMsg, (WPARAM)GlobalMainWindowHandle, 0);
}

bool skype_can_send()
{
	return GlobalSkypeAPIWindowHandle && foo_skype::Preferences::enabled;
}

void skype_disconnect() {
	GlobalSkypeAPIWindowHandle = 0;
	console::printf(COMPONENT_TITLE ": Disconnected.");
}

void skype_send(const char *title) {
	if (!skype_can_send()) return;
	COPYDATASTRUCT CopyData;
	CopyData.dwData = 0;
	CopyData.cbData = 28 + strlen(title);
	CopyData.lpData = new char[CopyData.cbData];
	strcpy_s((char *)CopyData.lpData, CopyData.cbData, "SET PROFILE RICH_MOOD_TEXT ");
	strcat_s((char *)CopyData.lpData, CopyData.cbData, title);
	if (!SendMessage(GlobalSkypeAPIWindowHandle, WM_COPYDATA, (WPARAM)GlobalMainWindowHandle, (LPARAM)&CopyData)) skype_disconnect();
	delete CopyData.lpData;
}

void skype_stopped() {
	if (!skype_can_send()) return;
	skype_send(Preferences::stopped);
}

void skype_playing() {
	if (!skype_can_send()) return;
	static_api_ptr_t<playback_control> pc;
	service_ptr_t<titleformat_object> script;
	pfc::string8 str, text;
	if (pc->is_paused()) str = Preferences::paused;
	else str = Preferences::playing;
	static_api_ptr_t<titleformat_compiler>()->compile_safe(script, str);
	if (!pc->playback_format_title(0, text, script, 0, playback_control::display_level_titles)) {
		skype_stopped();
		return;
	}
	skype_send(text);
}

static LRESULT __stdcall WindowProc(HWND hWindow, UINT uiMessage, WPARAM uiParam, LPARAM ulParam) {
	if (uiMessage == WM_COPYDATA && GlobalSkypeAPIWindowHandle == (HWND)uiParam) {
		if (Preferences::pause_on_call && *((int *)ulParam + 1) >= 22 && !memcmp(*(char **)((char *)ulParam + 8), "CALL ", 5)) {
			int size = *((int *)ulParam + 1);
			char *str = *((char **)((char *)ulParam + 8));
			for (int i = 5; i < size; i++) {
				if (*(str+i) == ' ') {
					if (!strcmp(str+i+1, "STATUS RINGING")) static_api_ptr_t<playback_control>()->pause(true);
					break;
				}
			}
		} else if (!strcmp(*(char **)((char *)ulParam + 8), "USERSTATUS LOGGEDOUT")) skype_disconnect();
		return 1;
	}
	else if (uiMessage == GlobalSkypeControlAPIAttachMsg) {
		switch (ulParam) {
			case SKYPECONTROLAPI_ATTACH_SUCCESS: {
				GlobalSkypeAPIWindowHandle = (HWND)uiParam;
				if (static_api_ptr_t<playback_control>()->is_playing()) skype_playing();
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
				if (GlobalSkypeAPIWindowHandle) skype_disconnect();
				console::printf(COMPONENT_TITLE ": Skype API is now available.");
				skype_connect();
				break;
		}
		return 0;
	} else return DefWindowProc(hWindow, uiMessage, uiParam, ulParam);
}

class skype_initquit : public initquit {
	virtual void on_init() {
		if (core_api::is_quiet_mode_enabled())
			return;
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
		if (core_api::is_quiet_mode_enabled())
			return;
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
