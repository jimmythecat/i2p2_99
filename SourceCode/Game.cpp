#include "Game.h"
#include "Utils.h"
#include "data/DataCenter.h"
#include "data/OperationCenter.h"
#include "data/SoundCenter.h"
#include "data/ImageCenter.h"
#include "data/FontCenter.h"
#include "Player.h"
#include "Level.h"
#include "Hero.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_acodec.h>
#include <vector>
#include <cstring>

// fixed settings
constexpr char game_icon_img_path[] = "./assets/image/game_icon.png";
constexpr char game_start_sound_path[] = "./assets/sound/growl.wav";
constexpr char background_img_path[] = "./assets/image/StartBackground.jpg";
constexpr char background_sound_path[] = "./assets/sound/BackgroundMusic.ogg";

//*-------*
// szc 12/11  新增Role Select STATE 
constexpr char role_select_img_path[] = "./assets/image/RoleSelect.jpg";
constexpr char role1_img_path[] = "./assets/image/roles/role1.jpg";
constexpr char role2_img_path[] = "./assets/image/roles/role2.jpg";
constexpr char role3_img_path[] = "./assets/image/roles/role3.png";
//*-------*


/**
 * @brief Game entry.
 * @details The function processes all allegro events and update the event state to a generic data storage (i.e. DataCenter).
 * For timer event, the game_update and game_draw function will be called if and only if the current is timer.
 */
void
Game::execute() {
	DataCenter *DC = DataCenter::get_instance();
	// main game loop
	bool run = true;
	while(run) {
		// process all events here
		al_wait_for_event(event_queue, &event);
		switch(event.type) {
			case ALLEGRO_EVENT_TIMER: {
				run &= game_update();
				game_draw();
				break;
			} case ALLEGRO_EVENT_DISPLAY_CLOSE: { // stop game
				run = false;
				break;
			} case ALLEGRO_EVENT_KEY_DOWN: {
				DC->key_state[event.keyboard.keycode] = true;
				break;
			} case ALLEGRO_EVENT_KEY_UP: {
				DC->key_state[event.keyboard.keycode] = false;
				break;
			} case ALLEGRO_EVENT_MOUSE_AXES: {
				DC->mouse.x = event.mouse.x;
				DC->mouse.y = event.mouse.y;
				break;
			} case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN: {
				DC->mouse_state[event.mouse.button] = true;
				break;
			} case ALLEGRO_EVENT_MOUSE_BUTTON_UP: {
				DC->mouse_state[event.mouse.button] = false;
				break;
			} default: break;
		}
	}
}

/**
 * @brief Initialize all allegro addons and the game body.
 * @details Only one timer is created since a game and all its data should be processed synchronously.
 */
Game::Game() {
	DataCenter *DC = DataCenter::get_instance();
	GAME_ASSERT(al_init(), "failed to initialize allegro.");

	// initialize allegro addons
	bool addon_init = true;
	addon_init &= al_init_primitives_addon();
	addon_init &= al_init_font_addon();
	addon_init &= al_init_ttf_addon();
	addon_init &= al_init_image_addon();
	addon_init &= al_init_acodec_addon();
	GAME_ASSERT(addon_init, "failed to initialize allegro addons.");

	// initialize events
	bool event_init = true;
	event_init &= al_install_keyboard();
	event_init &= al_install_mouse();
	event_init &= al_install_audio();
	GAME_ASSERT(event_init, "failed to initialize allegro events.");

	// initialize game body
	GAME_ASSERT(
		display = al_create_display(DC->window_width, DC->window_height),
		"failed to create display.");
	GAME_ASSERT(
		timer = al_create_timer(1.0 / DC->FPS),
		"failed to create timer.");
	GAME_ASSERT(
		event_queue = al_create_event_queue(),
		"failed to create event queue.");

	debug_log("Game initialized.\n");
	game_init();
}

/**
 * @brief Initialize all auxiliary resources.
 */
void
Game::game_init() {
	DataCenter *DC = DataCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();
	ImageCenter *IC = ImageCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();

	//*-------*
	// szc 12/11  新增Role Select STATE 

	// 設置遊戲視窗大小
    DC->window_width = 1024;  // 設定適合的視窗寬度
    DC->window_height = 1024;  // 設定適合的視窗高度

    // 建立遊戲視窗
    display = al_create_display(DC->window_width, DC->window_height);
    GAME_ASSERT(display, "failed to create display");

	//*-------*

	// set window icon
	game_icon = IC->get(game_icon_img_path);
	al_set_display_icon(display, game_icon);

	//*-------*
	// szc 12/11  新增Role Select STATE 
	// 初始化角色選擇畫面背景和角色圖片
    role_select_background = IC->get(role_select_img_path);
    role1_img = IC->get(role1_img_path);
    role2_img = IC->get(role2_img_path);
    role3_img = IC->get(role3_img_path);
	//*-------*

	// register events to event_queue
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

	// init sound setting
	SC->init();

	// init font setting
	FC->init();

	ui = new UI();
	ui->init();

	DC->level->init();
	//*-------*
	// szc 12/11  新增Role Select STATE 
	//*-------*
	DC->hero->init(selected_role);

	// game start
	background = IC->get(background_img_path);
	debug_log("Game state: change to START\n");
	//*-------*
	// szc 12/11  新增Role Select STATE 
	state = STATE::ROLE_SELECT;
	//*-------*
	al_start_timer(timer);
}

/**
 * @brief The function processes all data update.
 * @details The behavior of the whole game body is determined by its state.
 * @return Whether the game should keep running (true) or reaches the termination criteria (false).
 * @see Game::STATE
 */
bool
Game::game_update() {
	DataCenter *DC = DataCenter::get_instance();
	OperationCenter *OC = OperationCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();
	static ALLEGRO_SAMPLE_INSTANCE *background = nullptr;

	switch(state) {
		case STATE::START: {
			static bool is_played = false;
			static ALLEGRO_SAMPLE_INSTANCE *instance = nullptr;
			if(!is_played) {
				instance = SC->play(game_start_sound_path, ALLEGRO_PLAYMODE_ONCE);
				DC->level->load_level(1);
				is_played = true;
			}

			if(!SC->is_playing(instance)) {
				debug_log("<Game> state: change to LEVEL\n");
				state = STATE::LEVEL;
			}
			break;
		}

		//*-------*
		// szc 12/11  新增Role Select STATE 
		case STATE::ROLE_SELECT: {
            // 檢查是否有點擊角色選擇按鈕
            if(DC->mouse_state[1] && !DC->prev_mouse_state[1]) {
                if(DC->mouse.x >= 100 && DC->mouse.x <= 200 && DC->mouse.y >= 300 && DC->mouse.y <= 400) {
                    selected_role = 1; // 選擇角色 1
                } else if(DC->mouse.x >= 300 && DC->mouse.x <= 400 && DC->mouse.y >= 300 && DC->mouse.y <= 400) {
                    selected_role = 2; // 選擇角色 2
                } else if(DC->mouse.x >= 500 && DC->mouse.x <= 600 && DC->mouse.y >= 300 && DC->mouse.y <= 400) {
                    selected_role = 3; // 選擇角色 3
                }

                // 初始化角色
                if(selected_role != -1) {
                    DC->hero->init(selected_role); // 根據選擇的角色進行初始化
                    state = STATE::START; // 進入遊戲
					//*----更新後應改為進入level 狀態
                }
            }
            break;
        }
		//*-------*
		
		
		case STATE::LEVEL: {
			static bool BGM_played = false;
			if(!BGM_played) {
				background = SC->play(background_sound_path, ALLEGRO_PLAYMODE_LOOP);
				BGM_played = true;
			}

			if(DC->key_state[ALLEGRO_KEY_P] && !DC->prev_key_state[ALLEGRO_KEY_P]) {
				SC->toggle_playing(background);
				debug_log("<Game> state: change to PAUSE\n");
				state = STATE::PAUSE;
			}
			if(DC->level->remain_monsters() == 0 && DC->monsters.size() == 0) {
				debug_log("<Game> state: change to END\n");
				state = STATE::END;
			}
			if(DC->player->HP == 0) {
				debug_log("<Game> state: change to END\n");
				state = STATE::END;
			}
			break;
		} case STATE::PAUSE: {
			if(DC->key_state[ALLEGRO_KEY_P] && !DC->prev_key_state[ALLEGRO_KEY_P]) {
				SC->toggle_playing(background);
				debug_log("<Game> state: change to LEVEL\n");
				state = STATE::LEVEL;
			}
			break;
		} case STATE::END: {
			return false;
		}
	}
	// If the game is not paused, we should progress update.
	if(state != STATE::PAUSE) {
		DC->player->update();
		SC->update();
		ui->update();
		DC->hero->update();
		if(state != STATE::START) {
			DC->level->update();
			OC->update();
		}
	}
	// game_update is finished. The states of current frame will be previous states of the next frame.
	memcpy(DC->prev_key_state, DC->key_state, sizeof(DC->key_state));
	memcpy(DC->prev_mouse_state, DC->mouse_state, sizeof(DC->mouse_state));
	return true;
}

/**
 * @brief Draw the whole game and objects.
 */
void
Game::game_draw() {
	DataCenter *DC = DataCenter::get_instance();
	OperationCenter *OC = OperationCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();
	//szc 12/11 
	ImageCenter *IC = ImageCenter::get_instance();


	// Flush the screen first.
	al_clear_to_color(al_map_rgb(100, 100, 100));

	/* 修改中 請稍後
	if(state != STATE::END) {
		// background
		al_draw_bitmap(background, 0, 0, 0);
		if(DC->game_field_length < DC->window_width)
			al_draw_filled_rectangle(
				DC->game_field_length, 0,
				DC->window_width, DC->window_height,
				al_map_rgb(100, 100, 100));
		if(DC->game_field_length < DC->window_height)
			al_draw_filled_rectangle(
				0, DC->game_field_length,
				DC->window_width, DC->window_height,
				al_map_rgb(100, 100, 100));
		// user interface
		if(state != STATE::START) {
			DC->level->draw();
			DC->hero->draw();
			ui->draw();
			OC->draw();
		}
	}

	switch(state) {
		case STATE::START: {
		}
		//*-------*
		// szc 12/11  新增Role Select STATE 
		case STATE::ROLE_SELECT: {
            // 顯示角色選擇畫面的背景
            al_draw_bitmap(role_select_background, 0, 0, 0);

            // 顯示三個角色的選擇按鈕
            al_draw_scaled_bitmap(role1_img, 0, 0, al_get_bitmap_width(role1_img), al_get_bitmap_height(role1_img),
                                  100, 300, 100, 100, 0);  // 角色1按鈕縮放
            al_draw_scaled_bitmap(role2_img, 0, 0, al_get_bitmap_width(role2_img), al_get_bitmap_height(role2_img),
                                  300, 300, 100, 100, 0);  // 角色2按鈕縮放
            al_draw_scaled_bitmap(role3_img, 0, 0, al_get_bitmap_width(role3_img), al_get_bitmap_height(role3_img),
                                  500, 300, 100, 100, 0);  // 角色3按鈕縮放

            break;
        }
		//*-------*
		case STATE::LEVEL: {
			break;
		} case STATE::PAUSE: {
			// game layout cover
			al_draw_filled_rectangle(0, 0, DC->window_width, DC->window_height, al_map_rgba(50, 50, 50, 64));
			al_draw_text(
				FC->caviar_dreams[FontSize::LARGE], al_map_rgb(255, 255, 255),
				DC->window_width/2., DC->window_height/2.,
				ALLEGRO_ALIGN_CENTRE, "GAME PAUSED");
			break;
		} case STATE::END: {
		}
	}
	*/

	switch(state) {
        case STATE::START: {
			// 顯示遊戲的背景圖片或標誌
			al_draw_bitmap(background, 0, 0, 0);  // 顯示背景圖片
			
			// 顯示遊戲標誌（如果有的話）
			ALLEGRO_BITMAP *game_logo = IC->get("./assets/image/game_logo.png");  // 假設有遊戲標誌圖片
			al_draw_bitmap(game_logo, DC->window_width / 4, DC->window_height / 8, 0);  // 顯示遊戲標誌在螢幕上方

			// 顯示開始遊戲按鈕
			// 假設有一個開始遊戲的按鈕圖片
			ALLEGRO_BITMAP *start_button = IC->get("./assets/image/start_button.png");  // 假設有開始遊戲的按鈕
			int button_width = al_get_bitmap_width(start_button);
			int button_height = al_get_bitmap_height(start_button);
			al_draw_bitmap(start_button, DC->window_width / 2 - button_width / 2, DC->window_height / 2 - button_height / 2, 0);  // 顯示開始按鈕在畫面中間

			// 顯示遊戲簡介或規則
			al_draw_text(FC->courier_new[FontSize::MEDIUM], al_map_rgb(255, 255, 255),
						DC->window_width / 2, DC->window_height / 4 + 100, ALLEGRO_ALIGN_CENTER, "Welcome to the Game!");
			al_draw_text(FC->courier_new[FontSize::SMALL], al_map_rgb(255, 255, 255),
						DC->window_width / 2, DC->window_height / 4 + 150, ALLEGRO_ALIGN_CENTER, "Press Enter to Start");

			// 檢查鼠標是否點擊開始遊戲按鈕
			if(DC->mouse_state[1] && !DC->prev_mouse_state[1]) {  // 檢查左鍵點擊
				int mouse_x = DC->mouse.x;
				int mouse_y = DC->mouse.y;

				// 判斷鼠標是否在開始遊戲按鈕範圍內
				if(mouse_x >= DC->window_width / 2 - button_width / 2 &&
				mouse_x <= DC->window_width / 2 + button_width / 2 &&
				mouse_y >= DC->window_height / 2 - button_height / 2 &&
				mouse_y <= DC->window_height / 2 + button_height / 2) {
					// 按鈕被點擊，切換到角色選擇畫面
					state = STATE::ROLE_SELECT;  // 進入角色選擇畫面
				}
			}

			break;
		}

        case STATE::ROLE_SELECT: {
            // 當遊戲在角色選擇狀態下，繪製角色選擇畫面
            al_draw_bitmap(role_select_background, 0, 0, 0); // 顯示背景圖片

            // 顯示角色選擇按鈕（縮放過後的圖片）
            al_draw_scaled_bitmap(role1_img, 0, 0, al_get_bitmap_width(role1_img), al_get_bitmap_height(role1_img),
                                  100, 300, 100, 100, 0);  // 角色 1 按鈕
            al_draw_scaled_bitmap(role2_img, 0, 0, al_get_bitmap_width(role2_img), al_get_bitmap_height(role2_img),
                                  300, 300, 100, 100, 0);  // 角色 2 按鈕
            al_draw_scaled_bitmap(role3_img, 0, 0, al_get_bitmap_width(role3_img), al_get_bitmap_height(role3_img),
                                  500, 300, 100, 100, 0);  // 角色 3 按鈕

            break;
        }
        case STATE::LEVEL: {
            // 當遊戲進入 LEVEL 階段時，顯示背景和遊戲元素
            al_draw_bitmap(background, 0, 0, 0);

            // 繪製其他遊戲元素
            if(DC->game_field_length < DC->window_width)
                al_draw_filled_rectangle(
                    DC->game_field_length, 0,
                    DC->window_width, DC->window_height,
                    al_map_rgb(100, 100, 100));
            if(DC->game_field_length < DC->window_height)
                al_draw_filled_rectangle(
                    0, DC->game_field_length,
                    DC->window_width, DC->window_height,
                    al_map_rgb(100, 100, 100));

            // 繪製 UI 和遊戲內容
            if(state != STATE::START) {
                DC->level->draw();
                DC->hero->draw();
                ui->draw();
                OC->draw();
            }
            break;
        }
        case STATE::PAUSE: {
            // 當遊戲暫停時，顯示暫停界面
            al_draw_filled_rectangle(0, 0, DC->window_width, DC->window_height, al_map_rgba(50, 50, 50, 64));
            al_draw_text(
                FC->caviar_dreams[FontSize::LARGE], al_map_rgb(255, 255, 255),
                DC->window_width / 2., DC->window_height / 2.,
                ALLEGRO_ALIGN_CENTRE, "GAME PAUSED");
            break;
        }
        case STATE::END: {
            // 當遊戲結束時，可以在此顯示結算畫面
            break;
        }
    }

	al_flip_display();
}

Game::~Game() {
	al_destroy_display(display);
	al_destroy_timer(timer);
	al_destroy_event_queue(event_queue);
}
