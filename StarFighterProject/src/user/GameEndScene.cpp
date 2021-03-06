/**
*	@file	GameEndScene.cpp
*/

#include "GameEndScene.h"
#include "../Engine/GameEngine.h"
#include "../GameState.h"
#include "TitleScene.h"

#include "../../Res/Audio/testProject_acf.h"
#include "../../Res/Audio/CueSheet_0.h"
#include "../../Res/Resource.h"

#include <iostream>
#include <fstream>

namespace Application{

	bool GameEnd::Initialize(){

		GameEngine& game = GameEngine::Instance();

		game.LoadMeshFromFile(Resource::fbx_spaceSphere);
		game.LoadTextureFromFile(Resource::tex_spaceSphere);

		//ハイスコア読み込み処理
		int highScore = 100000;
		std::ifstream ifs(Resource::misc_highScore);
		if (!ifs.fail()) {
			char tmp[64];
			ifs.getline(tmp, 64);
			highScore = atoi(tmp);
		}
		
		//スコア読み込み処理
		int score = static_cast<int>(game.UserVariable("score"));

		if (score > highScore) {
			//ハイスコアを超えた

			highScore = score;
			std::ofstream ofs(Resource::misc_highScore);
			if (!ofs.fail()) {
				ofs << score << std::endl;
			}
		}

		//画面描画用文字列の初期化処理

		scoreStrInfo.color = glm::vec4(1);
		scoreStrInfo.size = glm::vec2(4.5f);
		scoreStrInfo.pos = glm::vec2(0, -0.05f);
		scoreStrInfo.str = std::string("SCORE: ") + std::to_string(score);
		scoreStrInfo.isCenter = true;

		highScoreStrInfo.color = glm::vec4(1);
		highScoreStrInfo.size = glm::vec2(4.5f);
		highScoreStrInfo.pos = glm::vec2(0, 0.1f);
		highScoreStrInfo.str = std::string("HIGH SCORE: ") + std::to_string(highScore);
		highScoreStrInfo.isCenter = true;

		gameoverStrInfo.color = glm::vec4(1, 0, 0, 1);
		gameoverStrInfo.size = glm::vec2(8);
		gameoverStrInfo.pos = glm::vec2(0, 0.5f);
		gameoverStrInfo.str = isClear ? "GAME CLEAR!!" : "GAME OVER...";
		gameoverStrInfo.isCenter = true;

		pressButtonStrInfo.color = glm::vec4(1, 0, 0, 1);
		pressButtonStrInfo.size = glm::vec2(2);
		pressButtonStrInfo.pos = glm::vec2(0,-0.5f);
		pressButtonStrInfo.str = "Pressed enter to title...";
		pressButtonStrInfo.isCenter = true;

		return true;
	}

	void GameEnd::Finalize() {

	}

	void GameEnd::Play() {

		GameEngine& game = GameEngine::Instance();

		auto e = game.AddEntity(GameState::EntityGroupId_Background, glm::vec3(0, 0, 0),
			"SpaceSphere", Resource::fbx_spaceSphere, std::make_shared<SpaceSphereMain>(), "NonLighting");
		game.KeyValue(0.1f);

		game.MainCamera(std::make_shared<CameraComponent>());
		game.MainCamera()->LookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));

		game.PlayAudio(0, CRI_CUESHEET_0_GAMEOVER);
	}

	void GameEnd::Stop() {
	}

	void GameEnd::Update(float delta){

		GameEngine& game = GameEngine::Instance();

		strFadeTimer += delta * (timer > 0 ? 20 : 2);

		float fadeAlpha = (glm::cos(strFadeTimer) + 1) * 0.25f;// 0 <= fadeAlpha <= 0.5
		pressButtonStrInfo.color.a = 0.5f + fadeAlpha;

		FontDrawInfo infoArray[] = {
			scoreStrInfo,highScoreStrInfo,gameoverStrInfo,pressButtonStrInfo
		};

		for (auto& info : infoArray) {
			game.FontColor(info.color);
			game.FontScale(info.size);
			game.AddString(info.pos, info.str.c_str(),info.isCenter);
		}

		auto gamepad = game.GetGamePad();

		if (timer > 0) {
			timer -= delta;
			if (timer <= 0) {

				game.PopScene();
				return;
			}
		}
		else if (game.GetGamePad().buttonDown & GamePad::START) {
			game.PlayAudio(1, CRI_CUESHEET_0_SELECT2);
			timer = 2;
		}
	}
}