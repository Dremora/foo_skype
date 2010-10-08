#include "foo_skype.h"

namespace foo_skype
{
	class PreferencesPageInstance : public CDialogImpl<PreferencesPageInstance>, public preferences_page_instance
	{
	private:
		CEdit playing;
		CEdit paused;
		CEdit stopped;
		CCheckBox pause_on_call;
		const preferences_page_callback::ptr callback;

	public:
		enum { IDD = IDD_PREFERENCES };

		BEGIN_MSG_MAP(PreferencesPageInstance)
			MSG_WM_INITDIALOG(OnInitDialog)
			COMMAND_HANDLER_EX(IDC_PREFERENCES_PLAYING, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_PREFERENCES_PAUSED, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_PREFERENCES_STOPPED, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_PREFERENCES_PAUSE_ON_CALL, BN_CLICKED, OnEditChange)
		END_MSG_MAP()

		PreferencesPageInstance(preferences_page_callback::ptr callback) : callback(callback) {}

		BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
		{
			playing = GetDlgItem(IDC_PREFERENCES_PLAYING);
			paused = GetDlgItem(IDC_PREFERENCES_PAUSED);
			stopped = GetDlgItem(IDC_PREFERENCES_STOPPED);
			pause_on_call = GetDlgItem(IDC_PREFERENCES_PAUSE_ON_CALL);

			uSetWindowText(playing, Preferences::playing);
			uSetWindowText(paused, Preferences::paused);
			uSetWindowText(stopped, Preferences::stopped);
			pause_on_call.SetCheck(Preferences::pause_on_call.get_value());
			return 0;
		}

		void OnEditChange(UINT, int, CWindow)
		{
			callback->on_state_changed();
		}

		t_uint32 get_state()
		{
			bool changed =
				strcmp(Preferences::playing.get_ptr(), uGetWindowText(playing).ptr()) != 0 ||
				strcmp(Preferences::paused.get_ptr(), uGetWindowText(paused).ptr()) != 0 ||
				strcmp(Preferences::stopped.get_ptr(), uGetWindowText(stopped).ptr()) != 0 ||
				Preferences::pause_on_call != pause_on_call.IsChecked();

			return preferences_state::resettable | (changed & preferences_state::changed);
		}

		void apply()
		{
			Preferences::playing = uGetWindowText(playing).ptr();
			Preferences::paused = uGetWindowText(paused).ptr();
			Preferences::stopped = uGetWindowText(stopped).ptr();
			Preferences::pause_on_call = pause_on_call.IsChecked();
		}

		void reset()
		{
			uSetWindowText(playing, Preferences::default_playing);
			uSetWindowText(paused, Preferences::default_paused);
			uSetWindowText(stopped, Preferences::default_stopped);
			pause_on_call.SetCheck(Preferences::default_pause_on_call);
		}
	};

	class PreferencesPage : public preferences_page_impl<PreferencesPageInstance>
	{
	public:
		const char *get_name()
		{
			return "Skype playing notifications";
		}

		GUID get_guid()
		{
			static const GUID guid = { 0xfe45c953, 0x8fdc, 0x49fc, { 0x9d, 0xc6, 0x4f, 0xc5, 0xd0, 0xa3, 0xa6, 0xef } };
			return guid;
		}

		GUID get_parent_guid()
		{
			return preferences_page_v3::guid_tools;
		}
	};

	preferences_page_factory_t<PreferencesPage> preferences_page;
}