/**
*	@file Effect.h
*/
#pragma once
#include "../../Entity.h"


namespace GameState {


	/**
	*	爆発の更新
	*/
	class Blast : public Entity::EntityDataBase {
	public:

		void Initialize() override;

		void Update(double delta) override;

	private:

		double timer = 0;			/// 経過時間

	};
}