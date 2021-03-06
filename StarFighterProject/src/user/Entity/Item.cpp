/**
*	@file	Item.cpp
*/

#include "Item.h"
#include "../../Engine/GameEngine.h"

namespace Application {

	void Item::Initialize() {
		entity->Scale({ 0.8f,0.8f,0.8f });
		entity->Velocity({ 0,0,-5 });
		entity->Color({ 1,1,1,1 });
	}

	void Item::Update(float delta) {

		//くるくる回転させる
		glm::quat addR = glm::angleAxis(5 * delta, glm::vec3(-1, 0, 0));
		entity->Rotation(entity->Rotation() * addR);

		glm::vec3 pos = entity->Position();

		//画面外判定処理
		if (std::abs(pos.x) > 200.0f || std::abs(pos.z) > 200.0f) {
			GameEngine::Instance().RemoveEntity(entity);
			return;
		}
	}

	void Item::CollisionEnter(Entity::Entity& e) {

		entity->Destroy();

	}

}