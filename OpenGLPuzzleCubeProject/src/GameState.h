/**
*	@file GameState.h
*/
#pragma once
#include "Entity.h"

namespace GameState {

	///エンティティの衝突グループID
	enum EntityGroupId {

		EntityGroupId_Background,
		EntityGroupId_Player,
		EntityGroupId_PlayerShot,
		EntityGroupId_Enemy,
		EntityGroupId_EnemyShot,
		EntityGroupId_Others
	};

	///タイトル画面
	class Title {
	public:

		//explicit Title(Entity::Entity* p = nullptr) : pSpaceSphere(p) {}
		void operator()(double delta);

	private:

		//Entity::Entity* pSpaceSphere;
		bool initial = true;
		float timer = 0;

	};

	///メインゲーム画面
	class MainGame {
	public:

		MainGame() {}
		//explicit MainGame(Entity::Entity* p);
		void operator()(double delta);
		
	private:

		//Entity::Entity* pSpaceSphere = nullptr;
		//Entity::Entity* pPlayer = nullptr;
		double interval = 0;
		int stageNo = 0;
		double stageTimer = -1;
	};

	///ゲームオーバー画面
	/*
	class GameOver {	
	public:

		explicit GameOver(Entity::Entity* = nullptr) {}
		void operator()(double delta);

	private:

		

	};*/



}

