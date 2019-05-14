/**
*	@file player.h
*/
#pragma once

#include "../../Entity.h"

namespace GameState {
	/// プレイヤー
	class Player : public Entity::EntityDataBase {
	public:

		void Initialize() override;

		void Update(double delta) override;

		void CollisionEnter(Entity::Entity& entity) override;

		void Damage(float p) override;

		void StartMove(double delta);

		void ShotBullet();

		int RemainingPlayer()const { return remainingPlayer; }

	private:

		bool isStartingMove = true;		/// スタート直後の移動処理
		float startMovValue = 20;		/// スタート直後の移動量
		float moveSpeed = 5.0f;

		float timer = 0;
		float damageTimer;				/// 無敵時間

		glm::vec3 moveBox[2] =
		{ {-25, -120, -1},{25, 100, 80} };		/// プレイヤーの可動域

		double shotInterval = 0;	/// 発射されるまでのクールタイム
		int multiShotNum = 1;			/// 一度に発射できる弾数

		int remainingPlayer = 3;		/// プレイヤー残機
	};
}