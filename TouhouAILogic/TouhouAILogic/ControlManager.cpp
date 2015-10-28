#include "Stdafx.h"
#include "ControlManager.h"
#include "Bullets.h"
#include "PlayerAlgorithm.h"

using namespace TouhouAILogic;

static WindowPrint printer;
static ImageRecognition recog;
static Player player;
static cv::Mat screen_image;
static std::vector<cv::Mat> screen_planes;
static Bullets bullets;
static PlayerAlgorithm player_alg;

void PlayerModule();
void EnemyModule();
void BulletModule();

TouhouAILogic::ControlManager::ControlManager()
{

}

void TouhouAILogic::ControlManager::Proc()
{
	GetPrintScreenModule();

	player_th = gcnew Thread(gcnew ThreadStart(PlayerModule));
	bullet_th = gcnew Thread(gcnew ThreadStart(BulletModule));
	
	if (!player_th->IsAlive) {
		player_th->Start();
	}

	if (!bullet_th->IsAlive) {
		bullet_th->Start();
	}
		
	player_th->Join();
	bullet_th->Join();

	recog.DrawRectangle(screen_image, recog.PlayerRect(), cv::Scalar(0, 255, 0));
	recog.DrawRectangle(screen_image, bullets.BulletsRect() , cv::Scalar(255, 0, 255));
	
	cv::imshow("matching", screen_image);

	auto p = player.Point();
	player_alg.PlayerUpdate(cv::Point(p.X(),p.Y()),bullets.OutRecoBullets(),player);
}

void TouhouAILogic::ControlManager::GetPrintScreenModule()
{
	printer.HBITMAPToMat(screen_image);

	cv::split(screen_image, screen_planes);
}

void PlayerModule()
{
	auto p_p = player.Point();

	const int h = 250;
	const int w = 250;

	p_p.Set(std::max(p_p.X() - (w/2 - 20), 0), std::max(p_p.Y() - h/2,0));
	p_p.Set(std::min(p_p.X(), screen_image.cols - w), std::min(p_p.Y(),screen_image.rows - h));

	cv::Rect roi_rect(p_p.X(),p_p.Y(), w , h);

	//プレイヤー探索範囲
	cv::rectangle(screen_image, roi_rect, cv::Scalar(0,0,0), 2, 8, 0);

	std::vector<cv::Mat> player_planes;

	for (auto x : screen_planes) {
		player_planes.push_back(x(roi_rect));
	}

	recog.PlayerRecognition(screen_image, player_planes , Vec2D(p_p.X(), p_p.Y()));

	player.InputPoint(recog.PlayerRect());


}

static int time_i = 0;

void BulletModule()
{
	//全体範囲探索

	const int hai = 500;
	const int wid = 400;

	std::vector<cv::Mat> bullet_planes;

	auto p_p = player.Point();
	p_p.Set(std::max(p_p.X() - (wid / 2 - 20), 0), std::max(p_p.Y() - (hai * 4 / 5), 0));

	cv::Rect rect(p_p.X(), p_p.Y(), std::min(screen_image.cols - p_p.X(), wid), std::min(screen_image.rows - p_p.Y(), hai));

	cv::rectangle(screen_image, rect, cv::Scalar(0, 0, 255), 2, 8, 0);

	for (auto x : screen_planes) {
		bullet_planes.push_back(x(rect));
	}

	if (time_i == 0) {
		recog.BulletRecognition(screen_image, bullet_planes, Vec2D(rect.x, rect.y));

		bullets.InputRecoBullets(recog.Bullets());
	}

	time_i = ++ time_i % 2;

	//すでに見つけてる弾探索

	auto bull = bullets.OutRecoBullets();
	const int sp_w = 60;
	const int sp_h = 60;

	for (auto x : bull) {
		bullet_planes.clear();

		auto b_p = x.Rect();
		b_p.x = std::min(std::max(b_p.x - sp_w / 2, 0), screen_image.cols - (sp_w + b_p.width));
		b_p.y = std::min(std::max(b_p.y - sp_h / 2, 0), screen_image.rows - (sp_h + b_p.height));
		b_p.width += sp_w;
		b_p.height += sp_h;

		cv::rectangle(screen_image, b_p, cv::Scalar(0, 0, 0), 2, 8, 0);
		
		auto p = x.Rect(); 
		cv::Point pp(p.x, p.y);
		
		cv::line(screen_image, pp,pp + 5 * x.MoveVec(), cv::Scalar(0, 255, 255),5);
		
		for (auto xy : screen_planes) {
			bullet_planes.push_back(xy(b_p));
		}

		recog.BulletRecognitionInd(screen_image, x.Image(), bullet_planes, Vec2D(b_p.x, b_p.y));
	}

	bullets.InputRecoBullets(recog.Bullets());
}


void EnemyModule()
{
	recog.EnemyRecognition(screen_image, screen_planes, Vec2D(0, 0));
}
