#include "foo_skype.h"

namespace foo_skype
{
	namespace Preferences
	{
		const GUID guid_enabled = { 0x444c42e8, 0x249, 0x4777, { 0x8e, 0x98, 0xae, 0x23, 0xfb, 0xb6, 0x84, 0x5a } };
		cfg_bool enabled(guid_enabled, true);

		const GUID guid_branch = { 0x9ae6ec18, 0xcb9c, 0x4998, { 0x9c, 0x86, 0x1f, 0xa9, 0x4d, 0xf3, 0xfe, 0xd1 } };
		advconfig_branch_factory branch(COMPONENT_TITLE, guid_branch, advconfig_entry::guid_branch_display, 10);

		const GUID guid_playing = { 0x0e646745, 0x9062, 0x4775, { 0x87, 0x81, 0xcb, 0xfd, 0x38, 0x73, 0x67, 0x49 } };
		advconfig_string_factory playing("Playing", guid_playing, guid_branch, 1, "playing: [%artist% - ]%title%");

		const GUID guid_paused = { 0x79b6576b, 0x9dc0, 0x4866, { 0xb2, 0xaf, 0x21, 0xa, 0x89, 0xd6, 0x2c, 0x53 } };
		advconfig_string_factory paused("Paused", guid_paused, guid_branch, 2, "paused: [%artist% - ]%title%");

		const GUID guid_stopped = { 0x78ac024d, 0x32a3, 0x40b2, { 0x88, 0x66, 0x9b, 0x5b, 0x7b, 0x2a, 0x9f, 0xc3 } };
		advconfig_string_factory stopped("Stopped", guid_stopped, guid_branch, 3, COMPONENT_DLL_NAME);

		const GUID guid_pause_on_call = { 0x6575e95f, 0x4e85, 0x46bf, { 0x81, 0xa3, 0xe6, 0xb1, 0x9e, 0x5f, 0x61, 0xc2 } };
		advconfig_checkbox_factory_t<false> pause_on_call("Pause on incoming Skype calls", guid_pause_on_call, advconfig_entry::guid_branch_playback, 10, false);
	}
}