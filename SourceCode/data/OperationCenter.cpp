#include "OperationCenter.h"
#include "DataCenter.h"
#include "../monsters/Monster.h"
#include "../towers/Tower.h"
#include "../towers/Bullet.h"
#include "../Player.h"
#include "../hero/Hero.h"
#include "../hero/Rocket.h"
#include <iostream>

void OperationCenter::update() {
	// Update monsters.
	_update_monster();
	// Update towers.
	_update_tower();
	// Update tower bullets.
	_update_towerBullet();
	// If any bullet overlaps with any monster, we delete the bullet, reduce the HP of the monster, and delete the monster if necessary.
	_update_monster_towerBullet();
	// If any monster reaches the end, hurt the player and delete the monster.
	_update_monster_player();
	// hero touch monster
	_update_hero_monster();

	_update_rocket();
	_update_monster_rocket();
}

void OperationCenter::_update_monster() {
	std::vector<Monster*> &monsters = DataCenter::get_instance()->monsters;
	for(Monster *monster : monsters)
		monster->update();
}

void OperationCenter::_update_tower() {
	std::vector<Tower*> &towers = DataCenter::get_instance()->towers;
	for(Tower *tower : towers)
		tower->update();
}

void OperationCenter::_update_towerBullet() {
	std::vector<Bullet*> &towerBullets = DataCenter::get_instance()->towerBullets;
	for(Bullet *towerBullet : towerBullets)
		towerBullet->update();
	// Detect if a bullet flies too far (exceeds its fly distance limit), which means the bullet lifecycle has ended.
	for(size_t i = 0; i < towerBullets.size(); ++i) {
		if(towerBullets[i]->get_fly_dist() <= 0) {
			towerBullets.erase(towerBullets.begin()+i);
			--i;
		}
	}
}

void OperationCenter::_update_monster_towerBullet() {
	DataCenter *DC = DataCenter::get_instance();
	std::vector<Monster*> &monsters = DC->monsters;
	std::vector<Bullet*> &towerBullets = DC->towerBullets;
	for(size_t i = 0; i < monsters.size(); ++i) {
		for(size_t j = 0; j < towerBullets.size(); ++j) {
			// Check if the bullet overlaps with the monster.
			if(monsters[i]->shape->overlap(*(towerBullets[j]->shape))) {
				// Reduce the HP of the monster. Delete the bullet.
				monsters[i]->HP -= towerBullets[j]->get_dmg();
				towerBullets.erase(towerBullets.begin()+j);
				--j;
			}
		}
	}
}

void OperationCenter::_update_hero_monster() {
	DataCenter *DC = DataCenter::get_instance();
    std::vector<Monster*> &monsters = DC->monsters;
    size_t i = 0;
    while (i < monsters.size()) {
        if (monsters[i]->shape->overlap(*(DC->hero->shape))) {
            DC->player->HP--;
            // 輸出偵錯訊息
            std::cout << "!!! Hero HP: " << DC->player->HP << std::endl;
            // 刪除怪物並從容器中移除
            delete monsters[i];
            monsters.erase(monsters.begin() + i);
        } else {
            ++i;
        }
    }
}

void OperationCenter::_update_monster_player() {
	DataCenter *DC = DataCenter::get_instance();
	std::vector<Monster*> &monsters = DC->monsters;
	Player *&player = DC->player;
	for(size_t i = 0; i < monsters.size(); ++i) {
		// Check if the monster is killed.
		if(monsters[i]->HP <= 0) {
			// Monster gets killed. Player receives money.
			player->coin += monsters[i]->get_money();
			monsters.erase(monsters.begin()+i);
			--i;
			// Since the current monsster is killed, we can directly proceed to next monster.
			break;
		}
		// Check if the monster reaches the end.
		if(monsters[i]->get_path().empty()) {
			monsters.erase(monsters.begin()+i);
			player->HP--;
			--i;
		}
	}
}

void OperationCenter::_update_monster_rocket(){
	DataCenter *DC = DataCenter::get_instance();
	std::vector<Monster*> &monsters = DC -> monsters;
	std::vector<Rocket*> &rockets = DC -> rockets;
	for(size_t i = 0; i < monsters.size(); ++i) {
		for(size_t j = 0; j < rockets.size(); ++j) {
			// Check if the rockets overlaps with the monster.
			if(monsters[i] -> shape -> overlap(*(rockets[j] -> shape))) {
				// Reduce the HP of the monster. Delete the rockets.
				monsters[i]->HP -= rockets[j]->get_dmg();
				delete rockets[j];
				rockets.erase(rockets.begin() + j);
				--j;
				break;
			}
		}
	}
}

void OperationCenter::_update_rocket() {
    std::vector<Rocket*> &rockets = DataCenter::get_instance()->rockets;
	for(Rocket *rocket : rockets) rocket->update();
    for (size_t i = 0; i < rockets.size(); ++i) {
        if (rockets[i]->get_remaining_range() <= 0) {
            delete rockets[i];
            rockets.erase(rockets.begin() + i);
            --i;
        }
    }
}

void OperationCenter::draw() {
	_draw_monster();
	_draw_tower();
	_draw_towerBullet();
	_draw_rocket();
}

void OperationCenter::_draw_monster() {
	std::vector<Monster*> &monsters = DataCenter::get_instance()->monsters;
	for(Monster *monster : monsters)
		monster->draw();
}

void OperationCenter::_draw_tower() {
	std::vector<Tower*> &towers = DataCenter::get_instance()->towers;
	for(Tower *tower : towers)
		tower->draw();
}

void OperationCenter::_draw_towerBullet() {
	std::vector<Bullet*> &towerBullets = DataCenter::get_instance()->towerBullets;
	for(Bullet *towerBullet : towerBullets)
		towerBullet->draw();
}

void OperationCenter::_draw_rocket(){
	DataCenter *DC = DataCenter::get_instance();
	std::vector<Rocket*> &rockets = DC->rockets;
    for (Rocket *rocket : rockets) {
        rocket->draw();
    }
}