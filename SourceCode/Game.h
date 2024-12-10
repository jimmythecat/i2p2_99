#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <allegro5/allegro.h>
#include "UI.h"

/**
 * @brief Main class that runs the whole game.
 * @details All game procedures must be processed through this class.
 */
class Game
{
public:
	void execute();
public:
	Game();
	~Game();
	void game_init();
	bool game_update();
	void game_draw();
private:
	/**
	 * @brief States of the game process in game_update.
	 * @see Game::game_update()
	 */
	enum class STATE {
		START, // -> LEVEL
		//*-------*
		// szc 12/11  新增Role Select
		ROLE_SELECT,
		//*-------*
		LEVEL, // -> PAUSE, END
		PAUSE, // -> LEVEL
		END
	};
	STATE state;
	ALLEGRO_EVENT event;
	ALLEGRO_BITMAP *game_icon;
	ALLEGRO_BITMAP *background;
	//*-------*
	// szc 12/11  新增Role Select
	ALLEGRO_BITMAP *role_select_background;  // 角色選擇畫面背景
    ALLEGRO_BITMAP *role1_img;              // 角色1的按鈕圖片
    ALLEGRO_BITMAP *role2_img;              // 角色2的按鈕圖片
    ALLEGRO_BITMAP *role3_img;              // 角色3的按鈕圖片
	int selected_role = -1;  //角色選擇
	//*-------*
private:
	ALLEGRO_DISPLAY *display;
	ALLEGRO_TIMER *timer;
	ALLEGRO_EVENT_QUEUE *event_queue;
	UI *ui;
};

#endif
