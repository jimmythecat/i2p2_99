#ifndef HERO_H_INCLUDED
#define HERO_H_INCLUDED 
//標頭檔保護 ifndef define endif

#include"Object.h"
#include<map>
#include<string>
//include一些需要的標頭檔

enum class HeroState{
    LEFT,
    RIGHT,
    FRONT,
    BACK,
    HEROSTATE_MAX
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
private:
    HeroState state = HeroState::FRONT;
    double speed = 5;
    double HP = 100;
    std::map<HeroState, std::string> gifPath;
};

#endif
