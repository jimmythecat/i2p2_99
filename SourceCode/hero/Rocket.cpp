#include "Rocket.h"
#include <iostream>
#include "../data/DataCenter.h"
#include "../data/ImageCenter.h"

#include "../shapes/Point.h"
#include "../shapes/Circle.h"
#include <allegro5/bitmap_draw.h>
#include <algorithm>

Rocket::Rocket(const Point &start_position, const Point &direction, const std::string &image_path, double speed, int damage, double range, double scale_factor) 
    : speed(speed), scale_factor(scale_factor){
    ImageCenter *IC = ImageCenter::get_instance();
    this->range = range;
    this->damage = damage;
    bitmap = IC->get(image_path);
    double r = std::min(al_get_bitmap_width(bitmap), al_get_bitmap_height(bitmap)) * scale_factor * 0.8;
    shape.reset(new Circle{start_position.x, start_position.y, r});
    // double length = Point::dist(Point(0, 0), direction);
    vx = 0;
    vy = -200;
}

void Rocket::update() {
    if (range == 0) return;
    DataCenter *DC = DataCenter::get_instance();
    double dx = vx / DC->FPS;
    double dy = vy / DC->FPS;
    // 更新火箭位置
    shape->update_center_x(shape->center_x() + dx);
    shape->update_center_y(shape->center_y() + dy);

    // 判斷火箭是否到達螢幕最上方（y座標小於等於 0）
    if (shape->center_y() <= 0) {
        range = 0;  // 火箭射到頂部，停止更新
    }
}

void Rocket::draw() {
    /*
    al_draw_bitmap(
        bitmap,
        shape->center_x() - al_get_bitmap_width(bitmap) / 2,
        shape->center_y() - al_get_bitmap_height(bitmap)/ 2, 0);
    */
    int width = al_get_bitmap_width(bitmap);
    int height = al_get_bitmap_height(bitmap);

    al_draw_scaled_bitmap(
        bitmap,
        0, 0, // 原圖的起始點
        width, height, // 原圖的寬高
        shape->center_x() - (width * scale_factor) / 2, // 縮放後的 X 座標
        shape->center_y() - (height * scale_factor) / 2, // 縮放後的 Y 座標
        width * scale_factor, // 縮放後的寬度
        height * scale_factor, // 縮放後的高度
        0 // 無額外繪製標誌
    );
}

