#include "Game.h"
#include "Utils.h"
#include "data/DataCenter.h"
#include "data/OperationCenter.h"
#include "data/SoundCenter.h"
#include "data/ImageCenter.h"
#include "data/FontCenter.h"
#include "Player.h"
#include "Level.h"
#include "hero/Hero.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_acodec.h>
#include <vector>
#include <cstring>
#include <string>
#include <map>



// fixed settings
constexpr char game_icon_img_path[] = "./assets/image/game_icon.png";
constexpr char startback_img_path[] = "./assets/image/startback.png";
constexpr char mainmenu_img_path[] = "./assets/image/mainmenu.png";
constexpr char roleselect_img_path[] = "./assets/image/roleselect1.png";
constexpr char about_img_path[] = "./assets/image/about.png";
//role image path
constexpr char role1_img_path[] = "./assets/image/roles/role1_left.png";
constexpr char role2_img_path[] = "./assets/image/roles/role2_left.png";
constexpr char role3_img_path[] = "./assets/image/roles/role3_left.png";
//music path
constexpr char game_start_sound_path[] = "./assets/sound/game_start.ogg";
constexpr char background_sound_path[] = "./assets/sound/BackgroundMusic.ogg";
constexpr char mainmenu_sound_path[] = "./assets/sound/menumusic.ogg";

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
			} default: {
				fprintf(stderr, "Unhandled event type: %d\n", event.type);
				break;
			}
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


void Game::game_init() {
	DataCenter *DC = DataCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();
	ImageCenter *IC = ImageCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();
	// set window icon
	game_icon = IC->get(game_icon_img_path);
	al_set_display_icon(display, game_icon);

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
	DC->hero->init(1);
	
	// game start
	current_background = nullptr;

	role1_img = IC->get(role1_img_path);
	role2_img = IC->get(role2_img_path);
	role3_img = IC->get(role3_img_path);

	debug_log("Game state: change to MAIN_MENU\n");
	state = STATE::MAIN_MENU;
	al_start_timer(timer);
}

/**
 * @brief The function processes all data update.
 * @details The behavior of the whole game body is determined by its state.
 * @return Whether the game should keep running (true) or reaches the termination criteria (false).
 * @see Game::STATE
 */
constexpr int ROLE_W = 250, ROLE_H = 250;       // 按鈕寬度和高度
constexpr int ROLE_BUTTON_SPACING = 50;        // 按鈕間距
constexpr int ROLE_AREA_WIDTH = ROLE_W * 3 + ROLE_BUTTON_SPACING * 2; // 總按鈕區域寬度
constexpr int ROLE_AREA_START_X = (1024 - ROLE_AREA_WIDTH) / 2;       // 起始 X 坐標
constexpr int ROLE_Y = 300;                    // 固定距離頂部 100px

constexpr int ROLE1_X = ROLE_AREA_START_X;
constexpr int ROLE2_X = ROLE_AREA_START_X + ROLE_W + ROLE_BUTTON_SPACING;
constexpr int ROLE3_X = ROLE_AREA_START_X + (ROLE_W + ROLE_BUTTON_SPACING) * 2;

const int button_width = 250;
const int button_height = 50;
const int button_spacing = 40;
const int total_height = (3 * button_height) + (2 * button_spacing);


bool Game::game_update() {
	DataCenter *DC = DataCenter::get_instance();
	OperationCenter *OC = OperationCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();
	ImageCenter *IC = ImageCenter::get_instance();
	static ALLEGRO_SAMPLE_INSTANCE *backmusic = nullptr;
	

	int start_y = (DC->window_height - total_height) / 2;
	int start_button_x = (DC->window_width - button_width) / 2;
	int start_button_y = start_y;
	int role_button_y = start_y + button_height + button_spacing;
	int about_button_y = role_button_y + button_height + button_spacing;

	switch(state) {
		case STATE::MAIN_MENU: {
			//增加背景音樂
			current_background = IC->get(mainmenu_img_path);

			if (!SC->is_playing(backmusic)) {  // 确保之前的音乐停止
                backmusic = SC->play(mainmenu_sound_path, ALLEGRO_PLAYMODE_ONCE);
            }

            // 在主選單中檢測滑鼠點擊或按鍵
            if (DC->mouse_state[1] && !DC->prev_mouse_state[1]) {
                int mouse_x = DC->mouse.x;
                int mouse_y = DC->mouse.y;

                // 假設三個按鈕的位置
                if (mouse_x >= start_button_x && mouse_x <= start_button_x + button_width &&
    				mouse_y >= start_button_y && mouse_y <= start_button_y + button_height) {
                    	state = STATE::START; // 進入開始遊戲
						debug_log("<Game> state changed to: %d\n", static_cast<int>(state));

                }  
				if (mouse_x >= start_button_x && mouse_x <= start_button_x + button_width &&
    				mouse_y >= role_button_y && mouse_y <= role_button_y + button_height) {
                    	state = STATE::ROLE_SELECT; // 進入角色選擇
						debug_log("<Game> state changed to: %d\n", static_cast<int>(state));

                } 
				if (mouse_x >= start_button_x && mouse_x <= start_button_x + button_width &&
    				mouse_y >= about_button_y && mouse_y <= about_button_y + button_height) {
                    	state = STATE::ABOUT; // 進入遊戲介紹
						debug_log("<Game> state changed to: %d\n", static_cast<int>(state));

                }
            }
			if (state == STATE::START) {
				if (backmusic) {
					al_stop_sample_instance(backmusic);  // 暂停音乐
					backmusic = nullptr;           // 标记为已停止
				}		
			}
			break;
        } case STATE::ABOUT: {
			
			if (current_background != IC->get(about_img_path)) {
				current_background = IC->get(about_img_path); // 只在圖片不相同時重新加載
			}

			if (DC->key_state[ALLEGRO_KEY_ESCAPE]) {
				debug_log("<Game> state: change to MAIN_MENU\n");
				state = STATE::MAIN_MENU;
			}
			break;
		} case STATE::ROLE_SELECT: {
			
			
			current_background = IC->get(roleselect_img_path);
			
			// 檢查是否有點擊角色選擇按鈕
			if (DC->mouse_state[1] && !DC->prev_mouse_state[1]) {
				int selected_role = -1;
				// 檢查角色1按鈕是否被點擊
				if (DC->mouse.x >= ROLE1_X && DC->mouse.x <= ROLE1_X + ROLE_W &&
					DC->mouse.y >= ROLE_Y && DC->mouse.y <= ROLE_Y + ROLE_H) {
					selected_role = 1; // 選擇角色 1
				} 
				// 檢查角色2按鈕是否被點擊
				else if (DC->mouse.x >= ROLE2_X && DC->mouse.x <= ROLE2_X + ROLE_W &&
						DC->mouse.y >= ROLE_Y && DC->mouse.y <= ROLE_Y + ROLE_H) {
					selected_role = 2; // 選擇角色 2
				} 
				// 檢查角色3按鈕是否被點擊
				else if (DC->mouse.x >= ROLE3_X && DC->mouse.x <= ROLE3_X + ROLE_W &&
						DC->mouse.y >= ROLE_Y && DC->mouse.y <= ROLE_Y + ROLE_H) {
					selected_role = 3;// 選擇角色 3
				}

				if (selected_role != -1) {
					DC->hero->init(selected_role); // 初始化選中的角色
					
					state = STATE::MAIN_MENU; // 返回主選單
				} else {
					debug_log("<Game> No valid role selected.\n");
				}
			}	
			break;
		} case STATE::START: {

			static bool is_load = false;
			static ALLEGRO_SAMPLE_INSTANCE *instance = nullptr;
			static ALLEGRO_SAMPLE_INSTANCE *startmusic = nullptr;
			
			
			current_background = IC->get(startback_img_path);
			/*if(!is_played) {
				instance = SC->play(game_start_sound_path, ALLEGRO_PLAYMODE_ONCE);
				DC->level->load_level(1);
				is_played = true;
			}*/
			if (!is_load) {
				backmusic = SC->play(game_start_sound_path, ALLEGRO_PLAYMODE_ONCE);	
				DC->level->load_level(1);
				backmusic = nullptr;
				is_load = true;
			}
			if(!SC->is_playing(instance)) {
				static bool BGM_played = false;
				if (!BGM_played && SC->is_playing(instance) == false) {
					startmusic = SC->play(background_sound_path, ALLEGRO_PLAYMODE_LOOP);
					BGM_played = true;
				}

				if(DC->key_state[ALLEGRO_KEY_P] && !DC->prev_key_state[ALLEGRO_KEY_P]) {
					SC->toggle_playing(startmusic);
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
			}	
			break;
		} case STATE::PAUSE: {
			if(DC->key_state[ALLEGRO_KEY_P] && !DC->prev_key_state[ALLEGRO_KEY_P]) {
				SC->toggle_playing(backmusic);
				debug_log("<Game> state: change to LEVEL\n");
				state = STATE::START;
			}
			break;
		} case STATE::END: {
			debug_log("<Game> Exiting game loop. State: END\n");
			return false;
		} default:
        debug_log("<Game> Unknown state detected: %d\n", static_cast<int>(state));
        return false; // 防止進入未知狀態
	}
	// If the game is not paused, we should progress update.
	if (state != STATE::PAUSE) {
        DC->player->update();
        DC->hero->update();
		SC->update();
        if (state != STATE::MAIN_MENU && state != STATE::ABOUT && state != STATE::ROLE_SELECT) {
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
	

	int start_y = (DC->window_height - total_height) / 2;
	int start_button_x = (DC->window_width - button_width) / 2;
	int start_button_y = start_y;
	int role_button_y = start_y + button_height + button_spacing;
	int about_button_y = role_button_y + button_height + button_spacing;

	// Flush the screen first.
	al_clear_to_color(al_map_rgb(100, 100, 100));
	
	//draw background
	
 	al_draw_scaled_bitmap(current_background,
				0, 0, al_get_bitmap_width(current_background), al_get_bitmap_height(current_background),
				0, 0, DC->window_width, DC->window_height, 0);
	

	switch(state) {

		case STATE::MAIN_MENU: {
			
			// 繪製主選單標題
			al_draw_text(FC->caviar_dreams[FontSize::LARGE], al_map_rgb(255, 255, 255),
					DC->window_width / 2, 150, ALLEGRO_ALIGN_CENTRE, "99 Percent Can't Win");

			// 開始遊戲按鈕
			al_draw_filled_rectangle(start_button_x, start_button_y, start_button_x + button_width,
								start_button_y + button_height, al_map_rgb(0, 128, 255));
			al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 255),
					DC->window_width / 2, start_button_y + 15, ALLEGRO_ALIGN_CENTRE, "Start");

			// 角色選擇按鈕
			al_draw_filled_rectangle(start_button_x, role_button_y, start_button_x + button_width,
								role_button_y + button_height, al_map_rgb(0, 128, 255));
			al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 255),
					DC->window_width / 2, role_button_y + 15, ALLEGRO_ALIGN_CENTRE, "Role Select");

			// 遊戲介紹按鈕
			al_draw_filled_rectangle(start_button_x, about_button_y, start_button_x + button_width,
								about_button_y + button_height, al_map_rgb(0, 128, 255));
			al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 255),
					DC->window_width / 2, about_button_y + 15, ALLEGRO_ALIGN_CENTRE, "About");
			break;
		} 

		case STATE::ABOUT: {
			// 繪製介紹畫面背景
			
			al_draw_text(
				FC->caviar_dreams[FontSize::LARGE], al_map_rgb(255, 255, 255),
				DC->window_width/2., DC->window_height/2.,
				ALLEGRO_ALIGN_CENTRE, "Press ESC to return");
			break;
		} 

		case STATE::ROLE_SELECT: {
			
			// 繪製標題
			constexpr int BUTTON_MARGIN = 20;

			// 繪製角色按鈕
			al_draw_scaled_bitmap(role1_img, 0, 0, al_get_bitmap_width(role1_img), al_get_bitmap_height(role1_img),
								ROLE1_X, ROLE_Y, ROLE_W, ROLE_H, 0);
			al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 255),
					ROLE1_X + ROLE_W / 2, ROLE_Y + ROLE_H + BUTTON_MARGIN, ALLEGRO_ALIGN_CENTRE, "BD Master");

			al_draw_scaled_bitmap(role2_img, 0, 0, al_get_bitmap_width(role2_img), al_get_bitmap_height(role2_img),
								ROLE2_X, ROLE_Y, ROLE_W, ROLE_H, 0);
			al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 255),
					ROLE2_X + ROLE_W / 2, ROLE_Y + ROLE_H + BUTTON_MARGIN, ALLEGRO_ALIGN_CENTRE, "3CM");

			al_draw_scaled_bitmap(role3_img, 0, 0, al_get_bitmap_width(role3_img), al_get_bitmap_height(role3_img),
								ROLE3_X, ROLE_Y, ROLE_W, ROLE_H, 0);
			al_draw_text(FC->caviar_dreams[FontSize::MEDIUM], al_map_rgb(255, 255, 255),
					ROLE3_X + ROLE_W / 2, ROLE_Y + ROLE_H + BUTTON_MARGIN, ALLEGRO_ALIGN_CENTRE, "Weed Warrior");
			break;
		} 

		case STATE::START: {
			
			DC->hero->draw();
			OC->draw();
			break;
		}

		case STATE::PAUSE: {
			// 繪製暫停畫面
			al_draw_filled_rectangle(0, 0, DC->window_width, DC->window_height, al_map_rgba(50, 50, 50, 64));
			al_draw_text(FC->caviar_dreams[FontSize::LARGE], al_map_rgb(255, 255, 255),
					DC->window_width / 2, DC->window_height / 2,
					ALLEGRO_ALIGN_CENTRE, "GAME PAUSED");
			break;
		}

		case STATE::END: {
			// 繪製遊戲結束畫面
			al_draw_text(FC->caviar_dreams[FontSize::LARGE], al_map_rgb(255, 255, 255),
					DC->window_width / 2, DC->window_height / 2,
					ALLEGRO_ALIGN_CENTRE, "GAME OVER");
			break;
		}
	}

	// 顯示畫面
	al_flip_display();
}


Game::~Game() {
    al_destroy_bitmap(current_background); 
	al_destroy_display(display);
	al_destroy_timer(timer);
	al_destroy_event_queue(event_queue);
}
