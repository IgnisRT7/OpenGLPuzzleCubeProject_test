/**
*	@file BulletManager.h
*/
#pragma once
#include "../Entity.h"


namespace GameState {

	///弾の管理システム
	class BulletManager {
	public:

		BulletManager(Entity::Entity& p,int groupId, Entity::Entity* t = nullptr);
		

		virtual void Update(float delta);
		virtual void CalcVelocity();
		void Target(Entity::Entity* t) { target = t; CalcVelocity(); }
		void GroupId(int i) { groupId = i; }
		void ShotInterval(float i) { shotInterval = i; }
		float ShotInterval() const { return shotInterval; }
		void BulletSpeed(float s) { initBulletSpeed = s; }
		float BulletSpeed() const { return initBulletSpeed; }
		void Color(glm::vec4 c) { color = c; }

	protected:
		float timer;
		float shotInterval = 2;		/// 弾の発射間隔
		float initBulletSpeed = 1;
		int groupId = -1;
		glm::vec4 color = glm::vec4(1);

		Entity::Entity& parent;		/// 弾の発射元
		Entity::Entity* target;		/// 弾の追尾対象
	};

	using BulletManagerPtr = std::shared_ptr<BulletManager>;

	///扇状に5方向に弾を発射する
	class MultiWayShot : public BulletManager {
	public:
		MultiWayShot(Entity::Entity& p,int i, Entity::Entity* t = nullptr, float a = 15.f,int m = 3);

		void Update(float delta) override;

	private:

		float angleInterval;
		int maxBulletNum;
	};

	///回転しながら連続で弾を発射する
	class CircleShot : public BulletManager {
	public:

		CircleShot(Entity::Entity& parent, int i, float angleInterval = 15.f, float shotAngle = 0.f);

		void Update(float delta) override;

	private:

		float angleInterval;
		float shotAngle;

	};

}
