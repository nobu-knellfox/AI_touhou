#pragma once

#include "Vec2D.h"

namespace TouhouAILogic {
	class SendMove {
		bool MoveState[4];
	public:
		SendMove();
		void MovePlayer(Vec2D dif);
		void Update();
	};
}