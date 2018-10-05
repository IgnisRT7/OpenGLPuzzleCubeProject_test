/**
*	@file MainGameState.cpp
*/
#include "GameState.h"
#include "GameEngine.h"
#include "../Res/Audio/SampleCueSheet.h"
#include <algorithm>

namespace GameState {
	///衝突形状リスト
	static const Entity::CollisionData collisionDataList[] = {
		{ glm::vec3(-1.0f,-1.0f,-1.0f), glm::vec3(1.0f,1.0f,1.0f) },
	{ glm::vec3(-0.5f,-0.5f,-1.0f), glm::vec3(0.5f,0.5f,1.0f) },
	{ glm::vec3(-1.0f,-1.0f,-1.0f), glm::vec3(1.0f,1.0f,1.0f) },
	{ glm::vec3(-0.25f,-0.25f,-0.25f), glm::vec3(0.25f,0.25f,0.25f) },
	};

	/**
	*	敵の円盤の状態を更新する
	*/
	struct UpdateToroid {

		//explicit UpdateToroid(const Entity::BufferPtr& buffer) : entityBuffer(buffer) {}
		void operator()(Entity::Entity& entity, double delta) {

			// 範囲外に出たら削除する.
			const glm::vec3 pos = entity.Position();
			if (std::abs(pos.x) > 40.0f || std::abs(pos.z) > 40.0f) {
				GameEngine::Instance().RemoveEntity(&entity);
				return;
			}

			//TODO : デバッグ用、敵を透明化させる
			auto colorValue = entity.Color();
			colorValue.a = 0;
			entity.Color(colorValue);

			entity.Scale({ 2,5,2 });
			entity.Color({ 1,1,1,0.4f });

			// 円盤を回転させる.
			float rot = glm::angle(entity.Rotation());
			rot += glm::radians(15.0f) * static_cast<float>(delta);
			if (rot > glm::pi<float>() * 2.0f) {
				rot -= glm::pi<float>() * 2.0f;
			}
			entity.Rotation(glm::angleAxis(rot, glm::vec3(0, 1, 0)));

		}

	};

	/**
	*	自機の弾の更新
	*/
	struct UpdatePlayerShot {
		void operator()(Entity::Entity& entity, double delta) {
			const glm::vec3 pos = entity.Position();
			if (std::abs(pos.x) > 40 || pos.z < -4 || pos.z > 40) {
				entity.Destroy();
				return;
			}
		}
	};

	/**
	*	爆発の更新
	*/
	struct UpdateBlast {

		void operator()(Entity::Entity& entity, double delta) {
			timer += delta;

			if (timer >= 0.5) {
				entity.Destroy();
				return;
			}
			const float variation = static_cast<float>(timer * 4);	//変化量
			entity.Scale(glm::vec3(static_cast<float>(1 + variation)));	//徐々に拡大していく

																		//時間経過で色と透明度を変化させる
			static const glm::vec4 color[] = {
				glm::vec4(1.0f,1.0f,0.75f,1),
				glm::vec4(1.0f,0.5f,0.1f,1),
				glm::vec4(0.25f,0.1f,0.1f,0),
			};
			const glm::vec4 col0 = color[static_cast<int>(variation)];
			const glm::vec4 col1 = color[static_cast<int>(variation) + 1];
			const glm::vec4 newColor = glm::mix(col0, col1, std::fmod(variation, 1));
			entity.Color(newColor);

			//Y軸回転させる
			glm::vec3 euler = glm::eulerAngles(entity.Rotation());
			euler.y += glm::radians(60.0f) + static_cast<float>(delta);
			entity.Rotation(glm::quat(euler));
		}

		double timer = 0;
	};


	/**
	*	自機の更新
	*/
	struct UpdatePlayer {
		void operator()(Entity::Entity& entity, double delta) {
			
			static float timer = 0;
			timer += delta * 100;

			/*TODO : プレイヤーのダメージ時の点滅アクションテスト
			auto color = entity.Color();
			color.a = (glm::sin(timer) + 1) * 0.5;
			entity.Color(color);
			*/

			GameEngine& game = GameEngine::Instance();

			const GamePad gamepad = game.GetGamePad();
			glm::vec3 vec;

			float rotZ = 0;
			if (gamepad.buttons & GamePad::DPAD_LEFT) {
				vec.x = 1;
				rotZ = -glm::radians(30.0f);
			}
			else if (gamepad.buttons & GamePad::DPAD_RIGHT) {
				vec.x = -1;
				rotZ = glm::radians(30.0f);
			}
			if (gamepad.buttons & GamePad::DPAD_UP) {
				vec.z = 1;
			}
			else if (gamepad.buttons & GamePad::DPAD_DOWN) {
				vec.z = -1;
			}
			if (vec.x || vec.z) {
				vec = glm::normalize(vec) * 5.0f;
			}
			
			glm::vec3 rt = glm::vec3(-25, -100, 1);
			glm::vec3 ld = glm::vec3(25, 100, 80)
				;
			entity.Velocity(vec);
			entity.Rotation(glm::quat(glm::vec3(0, 0, rotZ)));
			glm::vec3 pos = entity.Position();
			pos = glm::min(ld, glm::max(pos, rt));
			entity.Position(pos);

			if (gamepad.buttons & GamePad::A) {
				shotInterval -= delta;
				glm::vec3 pos = entity.Position();
				pos.x -= 0.3f;

				game.PlayAudio(0, 0);
				for (int i = 0; i < 2; ++i) {
					if (Entity::Entity* p = game.AddEntity(EntityGroupId_PlayerShot, pos, "NormalShot", "Res/Player.bmp", UpdatePlayerShot())) {
						p->Velocity(glm::vec3(0, 0, 80));
						p->Collision(collisionDataList[EntityGroupId_PlayerShot]);
					}
					pos.x += 0.25f;
				}
			}
			else {
				shotInterval = 0;
			}

		}
	private:
		double shotInterval = 0;
	};

	/**
	*	自機の弾と敵の衝突判定
	*/
	void PlayerShotAndEnemyCollisionHandler(Entity::Entity& lhs, Entity::Entity& rhs) {

		GameEngine& game = GameEngine::Instance();

		//爆発エフェクト
		if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, rhs.Position(), "Blast", "Res/Toroid.dds", UpdateBlast())) {
			const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
			p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
			p->Color(glm::vec4(1.0f, 0.75f, 0.5f, 1.0f));
			game.UserVariable("score") += 100;
		}

		//爆発音
		game.PlayAudio(1, CRI_SAMPLECUESHEET_BOMB);
		lhs.Destroy();
		rhs.Destroy();
	}

	/**
	*	自機と敵の衝突判定
	*/
	void PlayyerAndEnemyCollisionHandler(Entity::Entity& lhs, Entity::Entity& rhs) {

		GameEngine& game = GameEngine::Instance();

		//爆発エフェクト
		if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, rhs.Position(), "Blast", "Res/Toroid.bmp", UpdateBlast())) {
			const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
			p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
			game.UserVariable("score") += 100;
		}

		//爆発音
		game.PlayAudio(1, CRI_SAMPLECUESHEET_BOMB);

		if (lhs.GroupId() == EntityGroupId_Player) {
			rhs.Destroy();			
		}
		else {
			lhs.Destroy();	
		}

		game.UserVariable("player") -= 1;

	}

	/**
	*	メインゲーム画面のコンストラクタ
	*/
	MainGame::MainGame(Entity::Entity* p) : pSpaceSphere(p) {

		GameEngine& game = GameEngine::Instance();
		game.CollisionHandler(EntityGroupId_PlayerShot, EntityGroupId_Enemy, &PlayerShotAndEnemyCollisionHandler);
		game.CollisionHandler(EntityGroupId_Player, EntityGroupId_Enemy, &PlayyerAndEnemyCollisionHandler);

		game.UserVariable("player") = 3;	//プレイヤー残機数
	}

	/**
	*	メインゲーム画面の更新
	*/
	void MainGame::operator()(double delta) {
		
		GameEngine& game = GameEngine::Instance();

		if (game.UserVariable("player") <= 0) {
			//プレイヤー死亡

			//TODO : ゲームオーバーの処理へ
			//このときにゲーム上のエンティティをすべて削除しなければならない
			game.UpdateFunc(GameOver());
		}

		if (!pPlayer) {
			pPlayer = game.AddEntity(EntityGroupId_Player, glm::vec3(0, 0, 2), "Aircraft", "Res/Player.bmp", UpdatePlayer());
			pPlayer->Collision(collisionDataList[EntityGroupId_Player]);
		}

		game.Camera({ glm::vec4(0, 50 ,-8,1) ,glm::vec3(0,0,12),glm::vec3(0,1,0) });
		game.AmbientLight(glm::vec4(0.05f, 0.1f, 0.2f, 1));
		game.Light(0, { glm::vec4(4.,20,10,1),glm::vec4(1200,1200,1200,1) });

		//スポーン座標範囲
		std::uniform_int_distribution<> distributerX(-12, 12);
		std::uniform_int_distribution<> distributerZ(40, 44);

		interval -= delta;

		if (interval <= 0) {

			const std::uniform_real_distribution<> rndInterval(0.5f, 1);
			const std::uniform_int_distribution<> rndAddingCount(1, 5);

			for (int i = rndAddingCount(game.Rand()); i > 0; --i) {
				const glm::vec3 pos(distributerX(game.Rand()), 0, distributerZ(game.Rand()));

				if (Entity::Entity* p = game.AddEntity(EntityGroupId_Enemy, pos, "Toroid", "Res/Toroid.dds","Res/Toroid.Normal.bmp", UpdateToroid())) {

					p->Velocity(glm::vec3(pos.x < 0 ? 0.5f : -0.5f, 0, -1.0f));
					p->Collision(collisionDataList[EntityGroupId_Enemy]);
				}
			}

			interval = rndInterval(game.Rand());

		}


		char str[16];
		snprintf(str, 16, "%08.0f", game.UserVariable("score"));
		game.FontScale(glm::vec2(1.5));
		game.AddString(glm::vec2(-0.3f, 1.0f), str);

		snprintf(str, 16, "%02.0f", game.UserVariable("player"));
		std::string player = std::string("p - 0") + std::to_string((int)game.UserVariable("player"));

		game.AddString(glm::vec2(-0.9, 1.0f), player.c_str());

	}

}
