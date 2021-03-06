#include "Stdafx.h"
#include "ImageRecognition.h"
#include "ImageRecognitionMgr.h"
#include "ImageData.h"
#include "Player.h"

using namespace System::Threading;
using namespace TouhouAILogic;

static std::vector<cv::Rect> player_rect;
static std::vector<cv::Rect> enemy_rect;
static std::vector<cv::Rect> bullet_rect;
static std::vector<Bullet> bullets;

static std::vector<std::pair<cv::Mat, std::string>> player_image;
static std::vector<std::pair<cv::Mat, std::string>> player_image_move;
static std::vector<std::pair<cv::Mat, std::string>> bullet_image;
static std::vector<std::pair<cv::Mat, std::string>> enemy_image;

static cv::Rect bullet_search_rect;
static cv::Rect enemy_search_rect;


static cv::Mat screen_image;
static ImageRecognition recog;
static std::vector<cv::Mat> screen_planes;

static std::vector<cv::Mat> bullet_image_pyramid;
static std::vector<std::vector<cv::Mat>> bullet_planes_pyramid;

static std::vector<cv::Mat> enemy_image_pyramid;
static std::vector<std::vector<cv::Mat>> enemy_planes_pyramid;

static void PlayerRecognition(System::Object^ s);
static void EnemyRecognition(System::Object^ s);
static void BulletRecognition(System::Object^ s);

ref struct State
{
	Player& player; //最初の整数
	Mutex^ mtx; //ロック用のハンドル
	int n;

	State(int _n ,Player& _player, Mutex^ _mtx)
		: player(_player), mtx(_mtx), n(_n){}
};


ref struct State2
{
	Player& player; //最初の整数
	State2(Player& _player)
		: player(_player){}
};

TouhouAILogic::ImageRecognitionMgr::ImageRecognitionMgr()
{
}

void TouhouAILogic::ImageRecognitionMgr::Init()
{
	ImageData::Instance().ImageMap("player", player_image);
	ImageData::Instance().ImageMap("move_player", player_image_move);
	ImageData::Instance().ImageMap("bullet", bullet_image);
	ImageData::Instance().ImageMap("enemy", enemy_image);
	recog.Init();
}

void TouhouAILogic::ImageRecognitionMgr::Recognition(cv::Mat& screen ,Player& player)
{
	player_th = gcnew Thread(gcnew ParameterizedThreadStart(PlayerRecognition));
	
	screen_image = screen;

	BulletThreadSet();
	EnemyThreadSet();

	screen_planes.clear();
	cv::split(screen_image, screen_planes);
	
	player_th->Start(gcnew State2(player));

	BulletThreadStart(screen,player);
	EnemyThreadStart(screen, player);

	BulletThreadJoin();
	EnemyThreadJoin();
	player_th->Join();
}

static void BulletRecognition(System::Object^ s)
{
	
	State^ state = dynamic_cast<State^>(s); //キャストが必要

	std::vector<Bullet> b;

	state->mtx->WaitOne();
	auto img = bullet_image[state->n - 1];
	state->mtx->ReleaseMutex();

	std::vector<cv::Mat> bullet_p;
	cv::buildPyramid(img.first,bullet_p,2);

	recog.BulletRecognitionInd(screen_image, std::pair<cv::Mat,std::string>(bullet_p[2],img.second) , bullet_planes_pyramid[2], b, Vec2D(bullet_search_rect.x, bullet_search_rect.y));
	
	if (!b.empty()) {
		b.clear();
		recog.BulletRecognitionInd(screen_image, std::pair<cv::Mat, std::string>(bullet_p[1], img.second), bullet_planes_pyramid[1], b, Vec2D(bullet_search_rect.x, bullet_search_rect.y));

		if (!b.empty()) {
			b.clear();
			recog.BulletRecognitionInd(screen_image, std::pair<cv::Mat, std::string>(bullet_p[0], img.second), bullet_planes_pyramid[0], b, Vec2D(bullet_search_rect.x, bullet_search_rect.y));

		}
	}

	state->mtx->WaitOne();
	for (auto x : b) {
		bullets.push_back(x);
		bullet_rect.push_back(x.Rect());
	}
	state->mtx->ReleaseMutex();
	
}

void TouhouAILogic::ImageRecognitionMgr::PlayerRect(std::vector<cv::Rect>& p)
{
	p = player_rect;
}

void TouhouAILogic::ImageRecognitionMgr::EnemyRect(std::vector<cv::Rect>& e)
{
	e = enemy_rect;
}

void TouhouAILogic::ImageRecognitionMgr::BulletRect(std::vector<cv::Rect>& b)
{
	b = bullet_rect;
}

void TouhouAILogic::ImageRecognitionMgr::Bullets(std::vector<Bullet>& b)
{
	b = bullets;
}


void TouhouAILogic::ImageRecognitionMgr::BulletThreadSet()
{
	bullet_th1 = gcnew Thread(gcnew ParameterizedThreadStart(BulletRecognition));
	
	bullet_th2 = gcnew Thread(gcnew ParameterizedThreadStart(BulletRecognition));
	bullet_th3 = gcnew Thread(gcnew ParameterizedThreadStart(BulletRecognition));
	bullet_th4 = gcnew Thread(gcnew ParameterizedThreadStart(BulletRecognition));
	bullet_th5 = gcnew Thread(gcnew ParameterizedThreadStart(BulletRecognition));
	bullet_th6 = gcnew Thread(gcnew ParameterizedThreadStart(BulletRecognition));
	bullet_th7 = gcnew Thread(gcnew ParameterizedThreadStart(BulletRecognition));
	bullet_th8 = gcnew Thread(gcnew ParameterizedThreadStart(BulletRecognition));
	bullet_th9 = gcnew Thread(gcnew ParameterizedThreadStart(BulletRecognition));
	bullet_th10 = gcnew Thread(gcnew ParameterizedThreadStart(BulletRecognition));
	bullet_th11 = gcnew Thread(gcnew ParameterizedThreadStart(BulletRecognition));
	bullet_th12 = gcnew Thread(gcnew ParameterizedThreadStart(BulletRecognition));
	bullet_th13 = gcnew Thread(gcnew ParameterizedThreadStart(BulletRecognition));
	
}

static int player_count = 50;

void PlayerRecognition(System::Object^ s)
{
	State2^ state = dynamic_cast<State2^>(s); //キャストが必要
	auto p_p = state->player.Point();

	const int h = 200;
	const int w = 200;

	p_p.Set(std::max(p_p.X() - (w / 2 - 20), 0), std::max(p_p.Y() - h / 2, 0));
	p_p.Set(std::min(p_p.X(), screen_image.cols - w), std::min(p_p.Y(), screen_image.rows - h));

	cv::Rect roi_rect(p_p.X(), p_p.Y(), w, h);

	//プレイヤー探索範囲
	//cv::rectangle(screen_image, roi_rect, cv::Scalar(100, 100, 0), 2, 8, 0);
	

	std::vector<cv::Mat> player_planes;

	for (auto x : screen_planes) {
		player_planes.push_back(x(roi_rect));
	}


	recog.PlayerRecognition(screen_image, player_planes, player_rect, Vec2D(p_p.X(), p_p.Y()));

	if (!player_rect.empty()) {
		cv::rectangle(screen_image, player_rect[0], cv::Scalar(255, 0, 255), 2, 8, 0);
	}

	if (player_rect.empty()) {
		if (--player_count < 0) {
			player_rect.push_back(cv::Rect(357, 754, 14, 44));
		}
	}
	else {
		player_count = 50;
	}

}

void TouhouAILogic::ImageRecognitionMgr::BulletThreadStart(cv::Mat& screen_image,Player& player)
{
	Mutex^ mtx = gcnew Mutex;

	const int hai = 200;
	const int wid = 200;

	auto p_p = player.MidPoint();
	p_p.Set(std::max(p_p.X() - (wid / 2 ), 0), std::max(p_p.Y() - (hai /2), 0));

	bullet_search_rect = cv::Rect(p_p.X(), p_p.Y(), std::min(screen_image.cols - p_p.X(), wid), std::min(screen_image.rows - p_p.Y(), hai));

	//cv::rectangle(screen_image, bullet_search_rect, cv::Scalar(0, 0, 255), 2, 8, 0);

	bullets.clear();
	bullet_rect.clear();
	bullet_planes_pyramid.clear();
	bullet_image_pyramid.clear();

	cv::Mat bullet_image = screen_image(bullet_search_rect);

	cv::buildPyramid(bullet_image, bullet_image_pyramid, 2);

	for (auto image : bullet_image_pyramid) {
		cv::vector<cv::Mat> bullet_p;
		cv::split(image,bullet_p);
		bullet_planes_pyramid.push_back(bullet_p);
	}


	bullet_th1->Start(gcnew State(1,player, mtx));
	bullet_th2->Start(gcnew State(2,player, mtx));
	bullet_th3->Start(gcnew State(3,player, mtx));
	bullet_th4->Start(gcnew State(4,player, mtx));
	bullet_th5->Start(gcnew State(5,player, mtx));
	bullet_th6->Start(gcnew State(6,player, mtx));
	bullet_th7->Start(gcnew State(7,player, mtx));
	bullet_th8->Start(gcnew State(8,player, mtx));
	bullet_th9->Start(gcnew State(9, player, mtx));
	bullet_th10->Start(gcnew State(10, player, mtx));
	bullet_th11->Start(gcnew State(11, player, mtx));
	bullet_th12->Start(gcnew State(12, player, mtx));
	bullet_th13->Start(gcnew State(13, player, mtx));
}

void TouhouAILogic::ImageRecognitionMgr::BulletThreadJoin()
{
	bullet_th1->Join();
	bullet_th2->Join();
	bullet_th3->Join();
	bullet_th4->Join();
	bullet_th5->Join();
	bullet_th6->Join();
	bullet_th7->Join();
	bullet_th8->Join();
	bullet_th9->Join();
	bullet_th10->Join();
	bullet_th11->Join();
	bullet_th12->Join();
	bullet_th13->Join();
}


static void EnemyRecognition(System::Object^ s)
{
	State^ state = dynamic_cast<State^>(s); //キャストが必要

	std::vector<cv::Rect> b;

	state->mtx->WaitOne();
	auto img = enemy_image[state->n - 1];
	state->mtx->ReleaseMutex();

	std::vector<cv::Mat> enemy_p;
	cv::buildPyramid(img.first, enemy_p, 2);

	recog.EnemyRecognition(screen_image, std::pair<cv::Mat, std::string>(enemy_p[2], img.second), b ,bullet_planes_pyramid[2], Vec2D(bullet_search_rect.x, bullet_search_rect.y));

	if (!b.empty()) {
		b.clear();
		recog.EnemyRecognition(screen_image, std::pair<cv::Mat, std::string>(enemy_p[1], img.second),b, bullet_planes_pyramid[1], Vec2D(bullet_search_rect.x, bullet_search_rect.y));

		if (!b.empty()) {
			b.clear();
			recog.EnemyRecognition(screen_image, std::pair<cv::Mat, std::string>(enemy_p[0], img.second),b, bullet_planes_pyramid[0], Vec2D(bullet_search_rect.x, bullet_search_rect.y));

		}
	}

	state->mtx->WaitOne();
	for (auto x : b) {
		enemy_rect.push_back(x);
	}
	state->mtx->ReleaseMutex();
}

void TouhouAILogic::ImageRecognitionMgr::EnemyThreadSet()
{
	enemy_th1 = gcnew Thread(gcnew ParameterizedThreadStart(EnemyRecognition));
}

void TouhouAILogic::ImageRecognitionMgr::EnemyThreadStart(cv::Mat& screen_image, Player& player)
{
	Mutex^ mtx = gcnew Mutex;

	const int hai = 200;
	const int wid = 200;

	auto p_p = player.MidPoint();
	p_p.Set(std::max(p_p.X() - (wid / 2), 0), std::max(p_p.Y() - (hai / 2), 0));

	enemy_search_rect = cv::Rect(p_p.X(), p_p.Y(), std::min(screen_image.cols - p_p.X(), wid), std::min(screen_image.rows - p_p.Y(), hai));

	//cv::rectangle(screen_image, bullet_search_rect, cv::Scalar(0, 0, 255), 2, 8, 0);

	enemy_rect.clear();
	enemy_planes_pyramid.clear();
	enemy_image_pyramid.clear();

	cv::Mat enemy_image = screen_image(enemy_search_rect);

	cv::buildPyramid(enemy_image, enemy_image_pyramid, 2);

	for (auto image : enemy_image_pyramid) {
		cv::vector<cv::Mat> enemy_p;
		cv::split(image, enemy_p);
		enemy_planes_pyramid.push_back(enemy_p);
	}


	enemy_th1->Start(gcnew State(1, player, mtx));
//	enemy_th2->Start(gcnew State(2, player, mtx));

}

void TouhouAILogic::ImageRecognitionMgr::EnemyThreadJoin()
{
	enemy_th1->Join();
//	enemy_th2->Join();

}
