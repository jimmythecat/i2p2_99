#ifndef ROCKET_H_INCLUDED
#define ROCKET_H_INCLUDED

#include "../Object.h"
#include "../shapes/Point.h"
#include <allegro5/bitmap.h>
#include <string>

/**
 * @brief The rocket launched by Hero.
 * @see Hero
 */
class Rocket : public Object
{
public:
    Rocket(const Point &start_position, const Point &direction, const std::string &image_path, double speed, int damage, double range, double scale_factor = 0.2);
    void update();
    void draw();
    const double &get_remaining_range() const { return range; }
    const int &get_dmg() const { return damage; }

private:
    double vx; // x 方向速度
    double vy; // y 方向速度
    double range; // 火箭飛行的最大距離
    double speed;
    int damage; // 火箭傷害值
    ALLEGRO_BITMAP *bitmap; // 火箭的圖片
    double scale_factor; // 縮放比例
};

#endif // ROCKET_H_INCLUDED
