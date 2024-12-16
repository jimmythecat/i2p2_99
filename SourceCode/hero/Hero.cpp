#include "Hero.h"
#include <cstdio> // sprintf
#include <string> // std::string
#include "../data/DataCenter.h"
#include "../data/ImageCenter.h"
#include "../shapes/Rectangle.h"
#include <allegro5/allegro.h> // ALLEGRO_BITMAP, al_draw_scaled_bitmap, 等 Allegro 函數
#include <allegro5/allegro_image.h> // 加載和處理圖片的 Allegro 擴展
#include "Rocket.h"

// ./表示當前目錄 ../表示上一層
namespace HeroSetting {
	static constexpr char gif_root_path[40] = {
		"./assets/image/roles",
	};
	static constexpr char gif_postfix[][10] = {
		"left", "right", //"attack"
	};
}

void Hero::init(int role_id){
    
    this->current_role_id = role_id; // 設置當前角色 ID

    // 設定角色屬性
    if (role_id == 1) {
        speed = 5;
        HP = 100;
        //attack = 3;
    } else if (role_id == 2) {
        speed = 7;
        HP = 80;
        //attack = 5;
    } else if (role_id == 3) {
        speed = 3;
        HP = 150;
        //attack = 10;
    }
    //*-------*
    //把圖片讀進來
    for(size_t type = 0; type < static_cast<size_t>(HeroState::ROLESTATE_MAX); ++type){
        //static_cast:強制轉型 size_t是本程式定義的 unsigned ll
        char buffer[50];
        //sprintf 將東西輸出到字串裡而非螢幕上
        sprintf(
            buffer, "%s/role%d_%s.png",
            HeroSetting::gif_root_path,
            role_id,
            HeroSetting::gif_postfix[static_cast<int>(type)]);
        gifPath[static_cast<HeroState>(type)] = std::string(buffer);
    }
    // 設定 hitbox
    DataCenter *DC = DataCenter::get_instance();
    ImageCenter *IC = ImageCenter::get_instance();

    // 獲取角色左側圖片的位圖 (假設這是基準)
    ALLEGRO_BITMAP *image = IC->get(gifPath[HeroState::LEFT]);
    if (image) {
        int mapWidth = DC->game_field_length; // 獲取地圖的寬度
        int mapHeight = DC->game_field_length; // 假設地圖寬高一致

        // 計算 Hero 的目標大小，寬高為地圖的 1/5
        targetWidth = mapWidth / 5;
        targetHeight = mapHeight / 5;

        // 使用圖片的縮放後尺寸設置 hitbox
        shape.reset(new Rectangle(
            DC->window_width / 2 - targetWidth / 2,   // 左上角 X
            DC->window_height / 2 - targetHeight / 2, // 左上角 Y
            DC->window_width / 2 + targetWidth / 2,   // 右下角 X
            DC->window_height / 2 + targetHeight / 2  // 右下角 Y
        ));
    }
}

void Hero::update(){
    // allegro 座標系: 左上角是(0, 0)
    DataCenter *DC = DataCenter::get_instance();
    if(DC -> key_state[ALLEGRO_KEY_W]){
        shape -> update_center_y(shape -> center_y() - speed);
        state = HeroState::LEFT;
    }
    else if(DC -> key_state[ALLEGRO_KEY_A]){
        shape -> update_center_x(shape -> center_x() - speed);
        state = HeroState::LEFT;
    }
    else if(DC -> key_state[ALLEGRO_KEY_S]){
        shape -> update_center_y(shape -> center_y() + speed);
        state = HeroState::RIGHT;
    }
    else if(DC -> key_state[ALLEGRO_KEY_D]){
        shape -> update_center_x(shape -> center_x() + speed);
        state = HeroState::RIGHT;
    }

    if (DC->key_state[ALLEGRO_KEY_SPACE] && !(DC -> prev_key_state[ALLEGRO_KEY_SPACE])) {
        // 設定火箭發射的目標位置為當前位置向上移動一格
        launch_rocket();
    }
}

void Hero::draw(){
    ImageCenter *IC = ImageCenter::get_instance();
    ALLEGRO_BITMAP *image = IC->get(gifPath[state]);
    
    //畫image
    if (image) {
        // 使用初始化時設置的大小進行繪製
        al_draw_scaled_bitmap(
            image,
            0, 0, al_get_bitmap_width(image), al_get_bitmap_height(image), // 原始圖片大小
            shape->center_x() - targetWidth / 2,                          // 左上角 X
            shape->center_y() - targetHeight / 2,                         // 左上角 Y
            targetWidth, targetHeight,                                    // 目標大小
            0 // 無翻轉
        );
    }
}

void Hero::launch_rocket() {
    Point start_position(shape->center_x(), shape->center_y());
    Point direction(0, -1);
    DataCenter *DC = DataCenter::get_instance();
    // 將火箭加入到 DataCenter 的火箭列表
    DC->rockets.emplace_back(new Rocket(start_position, direction, "./assets/image/rocket.png", 10.0, 8, 5));
}

