#include <string.h>
#include "render_sdl.h"
#include "controller_info.h"
#include "config.h"

typedef struct {
	char const      *name;
	controller_info info;
} heuristic;

static heuristic heuristics[] = {
	//TODO: Add more heuristic rules
	{"DualShock 4", {.type = TYPE_PSX, .subtype = SUBTYPE_PS4}},
	{"PS4", {.type = TYPE_PSX, .subtype = SUBTYPE_PS4}},
	{"PS3", {.type = TYPE_PSX, .subtype = SUBTYPE_PS3}},
	{"X360", {.type = TYPE_XBOX, .subtype = SUBTYPE_X360}},
	{"Xbox 360", {.type = TYPE_XBOX, .subtype = SUBTYPE_X360}},
	{"X-box 360", {.type = TYPE_XBOX, .subtype = SUBTYPE_X360}},
	{"Xbox One", {.type = TYPE_XBOX, .subtype = SUBTYPE_XBONE}},
	{"X-box One", {.type = TYPE_XBOX, .subtype = SUBTYPE_XBONE}},
	{"WiiU", {.type = TYPE_NINTENDO, .subtype = SUBTYPE_WIIU}},
	{"Wii U", {.type = TYPE_NINTENDO, .subtype = SUBTYPE_WIIU}},
	{"Nintendo Switch", {.type = TYPE_NINTENDO, .subtype = SUBTYPE_SWITCH}},
	{"Saturn", {.type = TYPE_SEGA, .subtype = SUBTYPE_SATURN}}
};
const uint32_t num_heuristics = sizeof(heuristics)/sizeof(*heuristics);

static tern_node *info_config;
static uint8_t loaded;
static const char *subtype_names[] = {
	"unknown",
	"xbox",
	"xbox 360",
	"xbone",
	"ps2",
	"ps3",
	"ps4",
	"wiiu",
	"switch",
	"genesis",
	"saturn"
};
static const char *variant_names[] = {
	"normal",
	"6b bumpers",
	"6b right"
};
controller_info get_controller_info(int joystick)
{
	if (!loaded) {
		info_config = load_overrideable_config("controller_types.cfg", "controller_types.cfg");
		loaded = 1;
	}
	char guid_string[33];
	SDL_Joystick *stick = render_get_joystick(joystick);
	SDL_GameController *control = render_get_controller(joystick);
	SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(stick), guid_string, sizeof(guid_string));
	tern_node *info = tern_find_node(info_config, guid_string);
	if (info) {
		controller_info res;
		char *subtype = tern_find_ptr(info, "subtype");
		res.subtype = SUBTYPE_UNKNOWN;
		if (subtype) {
			for (int i = 0; i < SUBTYPE_NUM; i++)
			{
				if (!strcmp(subtype_names[i], subtype)) {
					res.subtype = i;
					break;
				}
			}
		}
		switch (res.subtype)
		{
		case SUBTYPE_XBOX:
		case SUBTYPE_X360:
		case SUBTYPE_XBONE:
			res.type = TYPE_XBOX;
			break;
		case SUBTYPE_PS2:
		case SUBTYPE_PS3:
		case SUBTYPE_PS4:
			res.type = TYPE_PSX;
			break;
		case SUBTYPE_WIIU:
		case SUBTYPE_SWITCH:
			res.type = TYPE_NINTENDO;
			break;
		case SUBTYPE_GENESIS:
		case SUBTYPE_SATURN:
			res.type = TYPE_SEGA;
			break;
		default:
			res.type = TYPE_UNKNOWN;
			break;
		}
		char *variant = tern_find_ptr(info, "variant");
		res.variant = VARIANT_NORMAL;
		if (variant) {
			for (int i = 0; i < VARIANT_NUM; i++)
			{
				if (!strcmp(variant_names[i], variant)) {
					res.variant = i;
					break;
				}
			}
		}
		res.name = control ? SDL_GameControllerName(control) : SDL_JoystickName(stick);
		if (control) {
			SDL_GameControllerClose(control);
		}
		return res;
	}
	if (!control) {
		return (controller_info) {
			.type = TYPE_UNKNOWN,
			.subtype = SUBTYPE_UNKNOWN,
			.variant = VARIANT_NORMAL,
			.name = SDL_JoystickName(stick)
		};
	}
	const char *name = SDL_GameControllerName(control);
	SDL_GameControllerClose(control);
	for (uint32_t i = 0; i < num_heuristics; i++)
	{
		if (strstr(name, heuristics[i].name)) {
			controller_info res = heuristics[i].info;
			res.name = name;
			return res;
		}
	}
	//default to a 360
	return (controller_info){
		.type = TYPE_GENERIC_MAPPING,
		.subtype = SUBTYPE_UNKNOWN,
		.variant = VARIANT_NORMAL,
		.name = name
	};
}

void save_controller_info(int joystick, controller_info *info)
{
	char guid_string[33];
	SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(render_get_joystick(joystick)), guid_string, sizeof(guid_string));
	tern_node *existing = tern_find_node(info_config, guid_string);
	existing = tern_insert_ptr(existing, "subtype", (void *)subtype_names[info->subtype]);
	existing = tern_insert_ptr(existing, "variant",  (void *)variant_names[info->variant]);
	info_config = tern_insert_node(info_config, guid_string, existing);
	persist_config_at(info_config, "controller_types.cfg");
	
}

void save_controller_mapping(int joystick, char *mapping_string)
{
	char guid_string[33];
	SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(render_get_joystick(joystick)), guid_string, sizeof(guid_string));
	tern_node *existing = tern_find_node(info_config, guid_string);
	existing = tern_insert_ptr(existing, "mapping", mapping_string);
	info_config = tern_insert_node(info_config, guid_string, existing);
	persist_config_at(info_config, "controller_types.cfg");
}

char const *labels_xbox[] = {
	"A", "B", "X", "Y", "Back", NULL, "Start", "Click", "Click", "White", "Black", "LT", "RT"
};
char const *labels_360[] = {
	"A", "B", "X", "Y", "Back", "Xbox", "Start", "Click", "Click", "LB", "RB", "LT", "RT"
};
static char const *labels_xbone[] = {
	"A", "B", "X", "Y", "View", "Xbox", "Menu", "Click", "Click", "LB", "RB", "LT", "RT"
};
static char const *labels_ps3[] = {
	"cross", "circle", "square", "triangle", "Select", "PS", "Start", "L3", "R3", "L1", "R1", "L2", "R2"
};
static char const *labels_ps4[] = {
	"cross", "circle", "square", "triangle", "Share", "PS", "Options", "L3", "R3", "L1", "R1", "L2", "R2"
};
static char const *labels_nintendo[] = {
	"B", "A", "Y", "X", "-", "Home", "+", "Click", "Click", "L", "R", "ZL", "ZR"
};
static char const *labels_genesis[] = {
	"A", "B", "X", "Y", NULL, NULL, "Start", NULL, NULL, "Z", "C", NULL, "Mode"
};
static char const *labels_saturn[] = {
	"A", "B", "X", "Y", NULL, NULL, "Start", NULL, NULL, "Z", "C", "LT", "RT"
};

static const char** label_source(controller_info *info)
{
	if (info->type == TYPE_UNKNOWN || info->type == TYPE_GENERIC_MAPPING || info->subtype ==SUBTYPE_X360) {
		return labels_360;
	} else if (info->type == TYPE_NINTENDO) {
		return labels_nintendo;
	} else if (info->type == TYPE_PSX) {
		if (info->subtype == SUBTYPE_PS4) {
			return labels_ps4;
		} else {
			return labels_ps3;
		}
	} else if (info->type == TYPE_XBOX) {
		if (info->subtype == SUBTYPE_XBONE) {
			return labels_xbone;
		} else {
			return labels_xbox;
		}
	} else {
		if (info->subtype == SUBTYPE_GENESIS) {
			return labels_genesis;
		} else {
			return labels_saturn;
		}
	}
}

const char *get_button_label(controller_info *info, int button)
{
	if (button >= SDL_CONTROLLER_BUTTON_DPAD_UP) {
		static char const * dirs[] = {"Up", "Down", "Left", "Right"};
		return dirs[button - SDL_CONTROLLER_BUTTON_DPAD_UP];
	}
	return label_source(info)[button];
}

static char const *axis_labels[] = {
	"Left X", "Left Y", "Right X", "Right Y"
};
const char *get_axis_label(controller_info *info, int axis)
{
	if (axis < SDL_CONTROLLER_AXIS_TRIGGERLEFT) {
		return axis_labels[axis];
	} else {
		return label_source(info)[axis - SDL_CONTROLLER_AXIS_TRIGGERLEFT + SDL_CONTROLLER_BUTTON_RIGHTSHOULDER + 1];
	}
}
