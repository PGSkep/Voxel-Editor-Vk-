#ifndef	ENGINE_H
#define ENGINE_H

#include "Scene.h"
#include "Renderer.h"

class Engine
{
public:
	// Global
	static bool done;
	static Scene scene;
	static Renderer renderer;

	// Time
	static Timer globalTimer;
	static double lastTime;
	static double deltaTime;

	// Input
	enum INPUT_STATE
	{
		IS_IDLE,
		IS_PRESSED,
		IS_DOWN,
		IS_RELEASED,
	};
	enum INPUT_CODE
	{
		// Mouse
		IC_MOUSE_LEFT = 1,
		IC_MOUSE_RIGHT = 2,
		IC_MOUSE_MIDDLE = 4,

		// ARROW
		IC_ARROW_UP = 38,
		IC_ARROW_DOWN = 40,
		IC_ARROW_LEFT = 37,
		IC_ARROW_RIGHT = 39,

		// NUMBERS
		IC_0 = 48,
		IC_1 = 49,
		IC_2 = 50,
		IC_3 = 51,
		IC_4 = 52,
		IC_5 = 53,
		IC_6 = 54,
		IC_7 = 55,
		IC_8 = 56,
		IC_9 = 57,

		// LETTERS
		IC_A = 65,
		IC_B = 66,
		IC_C = 67,
		IC_D = 68,
		IC_E = 69,
		IC_F = 70,
		IC_G = 71,
		IC_H = 72,
		IC_I = 73,
		IC_J = 74,
		IC_K = 75,
		IC_L = 76,
		IC_M = 77,
		IC_N = 78,
		IC_O = 79,
		IC_P = 80,
		IC_Q = 81,
		IC_R = 82,
		IC_S = 83,
		IC_T = 84,
		IC_U = 85,
		IC_V = 86,
		IC_W = 87,
		IC_X = 88,
		IC_Y = 89,
		IC_Z = 90,

		// General
		IC_ESCAPE = 27,
		IC_CONTROL = 17,
		IC_CONTROL_LEFT = 162,
		IC_CONTROL_RIGHT = 163,
		IC_SHIFT = 16,
		IC_SHIFT_LEFT = 160,
		IC_SHIFT_RIGHT = 161,
		IC_ALT = 18,
		IC_ALT_LEFT = 164,
		IC_ALT_RIGHT = 165,
		IC_BACKSPACE = 8,
		IC_DELETE = 46,
		IC_TAB = 9,
		IC_SPACE = 32,

		// PAD
		IC_PAD_0 = 96,
		IC_PAD_1 = 97,
		IC_PAD_2 = 98,
		IC_PAD_3 = 99,
		IC_PAD_4 = 100,
		IC_PAD_5 = 101,
		IC_PAD_6 = 102,
		IC_PAD_7 = 103,
		IC_PAD_8 = 104,
		IC_PAD_9 = 105,
		IC_PAD_DIVIDE = 111,
		IC_PAD_MULTIPLY = 106,
		IC_PAD_MINUS = 109,
		IC_PAD_PLUS = 107,
		IC_PAD_NUM_LOCK = 144,
		IC_PAD_DOT = 110,
		IC_PAD_ENTER = 13,

		// F*
		IC_F1 = 112,
		IC_F2 = 113,
		IC_F3 = 114,
		IC_F4 = 115,
		IC_F5 = 116,
		IC_F6 = 117,
		IC_F7 = 118,
		IC_F8 = 119,
		IC_F9 = 120,
		IC_F10 = 121,
		IC_F11 = 122,
		IC_F12 = 123,

		//Other
		IC_TILDE = 192,
		IC_UNDERSCORE = 189,
		IC_EQUAL = 187,
		IC_BRACKET_OPEN = 219,
		IC_BRACKET_CLOSE = 221,
		IC_BAR = 220,
		IC_SEMICOLON = 186,
		IC_QUOTE = 222,
		IC_ENTER = 13,
		IC_COMMA = 188,
		IC_PERIOD = 190,
		IC_QUESTION_MARK = 191,

		// Misc
		IC_PRINT_SCREEN = 44,
		IC_INSERT = 45,
		IC_SCROLL_LOCK = 145,
		IC_HOME = 36,
		IC_END = 35,
		IC_PAUSE_BREAK = 19,
		IC_PAGE_UP = 33,
		IC_PAGE_DOWN = 34,
		IC_CAPS_LOCK = 20,
		IC_OS_LEFT = 91,
		IC_OS_RIGHT = 92,
		IC_PAGE = 93,
	};

	static bool inputs[512];
	static bool* lastInput;
	static bool* currentInput;
	static INPUT_STATE GetInputState(uint8_t _input);

	// Engine
	static void Init();
	static void Loop();
	static void UpdateTime();
	static void UpdateInput();
	static void UpdateScene();
	static void UpdateRenderer();
	static void ShutDown();
};

#endif