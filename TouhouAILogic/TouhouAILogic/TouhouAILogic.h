// TouhouAILogic.h
#pragma once

#include "ControlManager.h"

namespace TouhouAILogic {

	public ref class MainProc
	{
		// TODO: このクラスの、ユーザーのメソッドをここに追加してください。
		ControlManager cm;
	public:
		System::String^ Test();
		void Init(System::IntPtr);
		void ScreenShot();
		void DebugSave();
		void Up();
	};
}
