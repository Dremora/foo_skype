#include "foo_skype.h"

namespace foo_skype
{
	namespace Preferences
	{
		const GUID guid_enabled = { 0x444c42e8, 0x249, 0x4777, { 0x8e, 0x98, 0xae, 0x23, 0xfb, 0xb6, 0x84, 0x5a } };
		cfg_bool enabled(guid_enabled, true);
	}
}