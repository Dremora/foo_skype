#include "foo_skype.h"

namespace foo_skype
{
	class Menu : public mainmenu_commands
	{
	public:
		t_uint32 get_command_count()
		{
			return 1;
		}

		GUID get_command(t_uint32 p_index)
		{
			static const GUID menu_guid = { 0x4747ea6c, 0xc43, 0x477d, { 0xb9, 0xb7, 0x5a, 0x55, 0x62, 0x2c, 0x49, 0x39 } };
			return menu_guid;
		}

		void get_name(t_uint32 p_index,pfc::string_base & p_out)
		{
			p_out = "Send playing notifications to Skype";
		}

		bool get_description(t_uint32 p_index,pfc::string_base & p_out)
		{
			return false;
		}

		GUID get_parent()
		{
			return mainmenu_groups::playback;
		}

		bool get_display(t_uint32 p_index,pfc::string_base & p_text,t_uint32 & p_flags)
		{
			p_flags = Preferences::enabled ? flag_checked : 0;
			get_name(p_index, p_text);
			return true;
		}

		void execute(t_uint32 p_index,service_ptr_t<service_base> p_callback)
		{
			Preferences::enabled = !Preferences::enabled.get_value();
		}
	};

	static mainmenu_commands_factory_t<Menu> menu;
}