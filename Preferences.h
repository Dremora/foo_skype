namespace foo_skype
{
	namespace Preferences
	{
		extern const GUID guid_enabled;
		extern cfg_bool enabled;

		extern const GUID guid_branch;
		extern advconfig_branch_factory branch;

		extern const GUID guid_playing;
		extern advconfig_string_factory playing;

		extern const GUID guid_paused;
		extern advconfig_string_factory paused;

		extern const GUID guid_stopped;
		extern advconfig_string_factory stopped;

		extern const GUID guid_pause_on_call;
		extern advconfig_checkbox_factory_t<false> pause_on_call;
	}
}