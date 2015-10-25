#pragma once
#include "Player.h"
#include "ImageRecognition.h"
#include "PrintWindow.h"

using namespace System::Threading;

void PlayerModule();
void EnemyModule();
void BulletModule();

namespace TouhouAILogic {

	ref class ControlManager {
		Thread^ player_th;
		Thread^ enemy_th;
		Thread^ bullet_th;

	public:
		ControlManager();
		void Proc();

	private:
		void GetPrintScreenModule();
	};

}