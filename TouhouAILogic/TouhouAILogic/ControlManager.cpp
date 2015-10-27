#include "Stdafx.h"
#include "ControlManager.h"
#include "Debug.h"
#include "Bullets.h"

using namespace TouhouAILogic;

static WindowPrint printer;
static ImageRecognition recog;
static Player player;
static cv::Mat screen_image;
static std::vector<cv::Mat> screen_planes;
static Bullets bullets;
	
std::vector<cv::Rect> bullet_recog_rect;
std::vector<cv::Rect> bullet_new_rect;

static int search_rect_i = 0;
static const int wdiv = 386; 
static const int hdiv = 450;
static cv::Rect BulletSearchRect[4] = 
{
	cv::Rect(0,0,wdiv,hdiv) , cv::Rect(wdiv,0,wdiv,hdiv),cv::Rect(0,hdiv,wdiv,hdiv),cv::Rect(wdiv,hdiv,wdiv,hdiv),
};

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
	recog.DrawRectangle(screen_image, recog.BulletRect(), cv::Scalar(255, 0, 255));
	
	cv::imshow("matching", screen_image);
}

void TouhouAILogic::ControlManager::GetPrintScreenModule()
{
	printer.HBITMAPToMat(screen_image);

	cv::split(screen_image, screen_planes);
}

void PlayerModule()
{
	auto p_p = player.Point();

	const int h = 200;
	const int w = 200;

	p_p.Set(std::max(p_p.X() - w/2, 0), std::max(p_p.Y() - h/2,0));
	p_p.Set(std::min(p_p.X(), screen_image.cols - w), std::min(p_p.Y(),screen_image.rows - h));

	cv::Rect roi_rect(p_p.X(),p_p.Y(), w , h);

	//�v���C���[�T���͈�
	cv::rectangle(screen_image, roi_rect, cv::Scalar(0,0,0), 2, 8, 0);

	std::vector<cv::Mat> player_planes;

	for (auto x : screen_planes) {
		player_planes.push_back(x(roi_rect));
	}

	recog.PlayerRecognition(screen_image, player_planes , Vec2D(p_p.X(), p_p.Y()));

	player.InputPoint(recog.PlayerRect());
}

void BulletModule()
{
	

	//�S�͈̔͒T��
	auto sr = BulletSearchRect[search_rect_i];
	search_rect_i = ++search_rect_i % 4;

	std::vector<cv::Mat> bullet_planes;

	for (auto x : screen_planes) {
		bullet_planes.push_back(x(sr));
	}

	recog.BulletRecognition(screen_image, bullet_planes, Vec2D(sr.x, sr.y));

	bullets.InputRecoBullets(recog.Bullets());
	bullet_new_rect = recog.BulletRect();

	//���łɌ����Ă�e�T��
	auto bull = bullets.OutRecoBullets();
	const int sp_w = 20;
	const int sp_h = 20;

	for (auto x : bull) {
		bullet_planes.clear();

		auto b_p = x.Rect();
		x.SetPoint(cv::Point(std::max(b_p.x - sp_w / 2, 0), std::max(b_p.y - sp_h / 2, 0)));
		x.SetPoint(cv::Point(std::min(b_p.x, screen_image.cols - sp_w), std::min(b_p.y, screen_image.rows - sp_h)));


		for (auto x : screen_planes) {
			bullet_planes.push_back(x(b_p));
		}

		recog.BulletRecognitionInd(screen_image, x.Image(), bullet_planes, Vec2D(b_p.x, b_p.y));
	}


	bullets.ClearBullets();
	bullets.InputRecoBullets(recog.Bullets());
	bullet_recog_rect = recog.BulletRect();
}


void EnemyModule()
{
	recog.EnemyRecognition(screen_image, screen_planes, Vec2D(0, 0));
}
