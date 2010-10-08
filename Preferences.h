namespace foo_skype
{
	namespace Preferences
	{
		extern const GUID guid_enabled;
		extern const bool default_enabled;
		extern cfg_bool enabled;

		extern const GUID guid_playing;
		extern const char *default_playing;
		extern cfg_string playing;

		extern const GUID guid_paused;
		extern const char *default_paused;
		extern cfg_string paused;

		extern const GUID guid_stopped;
		extern const char *default_stopped;
		extern cfg_string stopped;

		extern const GUID guid_pause_on_call;
		extern const bool default_pause_on_call;
		extern cfg_bool pause_on_call;
	}
}