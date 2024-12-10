#include "Hero.h"
#include <cstdio> //sprintf
#include "./data/DataCenter.h"
#include "./data/GIFCenter.h"
#include "./shapes/Rectangle.h"
// ./表示當前目錄 ../表示上一層
namespace HeroSetting {
	static constexpr char gif_root_path[40] = {
		"./assets/gif/Hero",
	};
	static constexpr char gif_postfix[][10] = {
		"left", "right", "front", "back"
	};
}

void Hero::init(){
    //把圖片讀進來
    for(size_t type = 0; type < static_cast<size_t>(HeroState::HEROSTATE_MAX); ++type){
        //static_cast:強制轉型 size_t是本程式定義的 unsigned ll
        char buffer[50];
        //sprintf 將東西輸出到字串裡而非螢幕上
        sprintf(
            buffer, "%s/dragonite_%s.gif",
            HeroSetting::gif_root_path,
            HeroSetting::gif_postfix[static_cast<int>(type)]);
        gifPath[static_cast<HeroState>(type)] = std::string(buffer);
    }
    //設定hitbox
    DataCenter *DC = DataCenter::get_instance();
    GIFCenter *GIFC = GIFCenter::get_instance();
    //參考 monsters\Monster.cpp 148
    ALGIF_ANIMATION *gif = GIFC->get(gifPath[HeroState::FRONT]);
    shape.reset(new Rectangle(
        DC -> window_width / 2,
        DC -> window_height / 2,
        DC -> window_height / 2 + gif -> width,
        DC -> window_height / 2 + gif -> height
    ));
}

void Hero::update(){
    // allegro 座標系: 左上角是(0, 0)
    DataCenter *DC = DataCenter::get_instance();
    if(DC -> key_state[ALLEGRO_KEY_W]){
        shape -> update_center_y(shape -> center_y() - speed);
        state = HeroState::BACK;
    }
    else if(DC -> key_state[ALLEGRO_KEY_A]){
        shape -> update_center_x(shape -> center_x() - speed);
        state = HeroState::LEFT;
    }
    else if(DC -> key_state[ALLEGRO_KEY_S]){
        shape -> update_center_y(shape -> center_y() + speed);
        state = HeroState::FRONT;
    }
    else if(DC -> key_state[ALLEGRO_KEY_D]){
        shape -> update_center_x(shape -> center_x() + speed);
        state = HeroState::RIGHT;
    }
}

void Hero::draw(){
    GIFCenter *GIFC = GIFCenter::get_instance();
	ALGIF_ANIMATION *gif = GIFC->get(gifPath[state]);
    //畫gif
	algif_draw_gif(
		gif,
		shape->center_x() - gif->width / 2,
		shape->center_y() - gif->height / 2, 0);
}