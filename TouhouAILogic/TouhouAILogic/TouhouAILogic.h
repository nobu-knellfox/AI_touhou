// TouhouAILogic.h
#pragma once

#include "ControlManager.h"

namespace TouhouAILogic {

	public ref class MainProc
	{
		// TODO: ���̃N���X�́A���[�U�[�̃��\�b�h�������ɒǉ����Ă��������B
		ControlManager cm;
	public:
		System::String^ Test();
		void Init(System::IntPtr);
		void ScreenShot();
		void Up();
		void Down();
		void Right();
		void Left();
		void Enter();
	};
}
