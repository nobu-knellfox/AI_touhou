#include "Stdafx.h"
#include "Bullet.h"
#include <algorithm>

TouhouAILogic::Bullet::Bullet(std::pair<cv::Mat,std::string> image):move_vec(0,0)
{
	img = image;
}

cv::Rect TouhouAILogic::Bullet::Rect()const
{
	return rect;
}

void TouhouAILogic::Bullet::InputRect(cv::Rect p)
{
	rect = p;
}

void TouhouAILogic::Bullet::Update(bool can_reco)
{
	if (can_reco) {
		life_time = defo_life_time;
	}
	else {
		--life_time;
	}
}

cv::Point TouhouAILogic::Bullet::MidPoint()const
{
	return cv::Point(rect.x + rect.width / 2 , rect.y + rect.height / 2);
}

int TouhouAILogic::Bullet::LifeTime()const
{
	return life_time;
}

void TouhouAILogic::Bullet::SetPoint(cv::Point p)
{
	rect.x = p.x;
	rect.y = p.y;
}

std::pair<cv::Mat,std::string> TouhouAILogic::Bullet::Image()const
{
	return img;
}

void TouhouAILogic::Bullet::SetIdentify(std::string i)
{
	identify = i;
}

void TouhouAILogic::Bullet::SetMoveVec(cv::Point v)
{
	move_vec = v;
}

cv::Point TouhouAILogic::Bullet::MoveVec()
{
	return move_vec;
}

