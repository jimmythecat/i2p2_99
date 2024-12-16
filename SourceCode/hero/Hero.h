#ifndef HERO_H_INCLUDED
#define HERO_H_INCLUDED 
//標頭檔保護 ifndef define endif

#include"../Object.h"
#include<map>
#include<string>
//include一些需要的標頭檔

enum class HeroState{
    LEFT,
    RIGHT,
    //ATTACK, 待實作
    ROLESTATE_MAX
    //Hero的各種狀態
};

class Hero : public Object
{
public:
    //*-------*
    // szc 12/11  新增Role Select STATE 
    void init(int role_id);
    void update();
    void draw();
    void launch_rocket();
private:
    HeroState state = HeroState::LEFT;
    double speed = 5;
    double HP = 100;
    double attack = 10;
    int current_role_id = 1; // 當前選擇的角色 ID，預設為角色 1
    
    
    std::map<HeroState, std::string> gifPath;
    int targetWidth;  // 繪製的目標寬度
    int targetHeight; // 繪製的目標高度
};

#endif
