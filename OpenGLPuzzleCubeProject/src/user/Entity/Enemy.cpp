/**
*	@file Entity.cpp
*/
#include "Enemy.h"
#include "../../GameEngine.h"
#include "../../GameState.h"
#include "../../../Res/Audio/SampleSound_acf.h"
#include "../../../Res/Audio/SampleCueSheet.h"

#include "Effect.h"
#include "Item.h"
#include "Bullet.h"
#include "Player.h"


namespace GameState {


	/**
	*	敵の初期化処理
	*/
	void Toroid::Initialize() {
		entity->CastShadow(true);
		entity->CastStencil(true);	//TODO : ステンシルマスクのテスト用
		entity->StencilColor(glm::vec4(1, 0, 1, 1));

		entity->Collision(collisionDataList[EntityGroupId_Enemy]);
		bulletManager = std::make_shared<EnemyBulletManager>(*entity);

		return;
	}

	/**
	*	敵の更新処理
	*
	*	@param delta	経過時間
	*/
	void Toroid::Update(float delta) {

		timer += delta;

		if (bulletManager)bulletManager->Update(delta);

		float rot = glm::angle(entity->Rotation());
		const glm::vec3 pos = entity->Position();

		GameEngine& game = GameEngine::Instance();

		auto e = game.FindEntityData<Player>();

		// 移動処理
		switch (enemyType) {
		case 0:	/// デフォルトの処理まっすぐ進む

			entity->Velocity(glm::vec3(0, 0, -10));


			break;

		case 1:	/// 蛇行して下方向へ
		{
			float velX = glm::cos(timer * 2) * 10 - entity->Velocity().x;
			glm::vec3 vel = glm::vec3(velX, 0, -5);
			entity->Velocity(vel);

			break;
		}
		case 2: ///プレイヤーキャラへ軽追尾




			break;

		case 3:

		default:
			break;
		}

		// 円盤を回転させる.
		rot += glm::radians(180.0f) * delta;
		if (rot > glm::pi<float>() * 2.0f) {
			rot -= glm::pi<float>() * 2.0f;
		}
		entity->Rotation(glm::angleAxis(rot, glm::vec3(0, 1, 0)));

		//画面外判定処理
		if (std::abs(pos.x) > 200.0f || std::abs(pos.z) > 200.0f) {
			GameEngine::Instance().RemoveEntity(entity);
			return;
		}
	}

	/**
	*	ダメージ処理
	*
	*	@param p	ダメージ量
	*/
	void Toroid::Damage(float p) {

		if (--hp <= 0) {

			GameEngine& game = GameEngine::Instance();

			//爆発エフェクト
			if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, entity->Position(), "Blast", "Res/Model/Toroid.dds", std::make_shared<Blast>())) {
				const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
				p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
				p->Color(glm::vec4(1.0f, 0.75f, 0.5f, 1.0f));
				game.UserVariable("score") += 100;
			}

			if (isItemDrop) {

				int itemID = rand() % 2;

				//アイテム
				std::string texName = itemID ? "Res/Model/ItemBoxSpeed.dds" : "Res/Model/ItemBoxBullet.dds";

				if (Entity::Entity* p = game.AddEntity(EntityGroupId_Item, entity->Position(), "ItemBox", texName.c_str(), std::make_shared<Item>(itemID))) {
					p->Collision(collisionDataList[EntityGroupId_Item]);
				}
			}

			//爆発音
			game.PlayAudio(1, CRI_SAMPLECUESHEET_BOMB);

			entity->Destroy();
		}
	}

	/**
	*	衝突判定処理
	*
	*	@param e	衝突してきたエンティティ
	*/
	void Toroid::CollisionEnter(Entity::Entity& e) {
	}

	/// 敵スポナーのクラス定義

	/**
	*	敵スポナーの更新処理
	*
	*	@param delta	経過時間
	*/
	void EnemyLaunchType::Update(float delta) {

		GameEngine& game = GameEngine::Instance();

		time += delta;

		if (time >= spawnMax * spawnInterval) {
			//スポーン終了

			entity->Destroy();
			return;
		}

		//敵の出撃処理
		if (launchIndex < static_cast<int>(time / spawnInterval)) {

			bool isItemDrop = spawnMax == (launchIndex + 2);	///最後に出撃する敵のみアイテムドロップする

			Entity::Entity* p = game.AddEntity(EntityGroupId_Enemy, entity->Position(),
				"Toroid", "Res/Model/Toroid.dds", "Res/Model/Toroid.Normal.bmp", std::make_shared<Toroid>(0, isItemDrop));

			p->Collision(collisionDataList[EntityGroupId_Enemy]);

			launchIndex++;
		}
	}

	/**
	*	更新処理
	*
	*	@param delta	経過時間
	*/
	void EnemyBulletManager::Update(float delta) {

		GameEngine& game = GameEngine::Instance();

		timer -= delta;

		if (timer < 0) {

			Entity::Entity* player = game.FindEntityData<Player>();

			if (Entity::Entity* p = game.AddEntity(EntityGroupId_EnemyShot, parent.Position(),
				"NormalShot", "Res/Model/Player.dds", std::make_shared<Bullet>(
					parent.Velocity(), player), "NonLighting")) {

				p->CastStencil(true);
				p->StencilColor(glm::vec4(1, 0, 1, 1));
				timer = shotInterval;

			}
		}
	}

}