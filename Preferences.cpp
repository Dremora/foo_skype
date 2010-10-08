#include "foo_skype.h"

namespace foo_skype
{
	namespace Preferences
	{
		const GUID guid_enabled = { 0x444c42e8, 0x249, 0x4777, { 0x8e, 0x98, 0xae, 0x23, 0xfb, 0xb6, 0x84, 0x5a } };
		const bool default_enabled = true;
		cfg_bool enabled(guid_enabled, default_enabled);

		const GUID guid_playing = { 0x0e646745, 0x9062, 0x4775, { 0x87, 0x81, 0xcb, 0xfd, 0x38, 0x73, 0x67, 0x49 } };
		const char *default_playing = "playing: [%artist% - ]%title%";
		cfg_string playing(guid_playing, default_playing);

		const GUID guid_paused = { 0x79b6576b, 0x9dc0, 0x4866, { 0xb2, 0xaf, 0x21, 0xa, 0x89, 0xd6, 0x2c, 0x53 } };
		const char *default_paused = "paused: [%artist% - ]%title%";
		cfg_string paused(guid_paused, default_paused);

		const GUID guid_stopped = { 0x78ac024d, 0x32a3, 0x40b2, { 0x88, 0x66, 0x9b, 0x5b, 0x7b, 0x2a, 0x9f, 0xc3 } };
		const char *default_stopped = COMPONENT_DLL_NAME;
		cfg_string stopped(guid_stopped, default_stopped);

		const GUID guid_pause_on_call = { 0x6575e95f, 0x4e85, 0x46bf, { 0x81, 0xa3, 0xe6, 0xb1, 0x9e, 0x5f, 0x61, 0xc2 } };
		const bool default_pause_on_call = false;
		cfg_bool pause_on_call(guid_pause_on_call, default_pause_on_call);
	}
}