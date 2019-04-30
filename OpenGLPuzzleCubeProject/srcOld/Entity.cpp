/**
*	@file Entity.cpp
*/
#include "Entity.h"
#include "Uniform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>

#define ENTITY_NEW_VERSION 1

/**
*	�G���e�B�e�B�Ɋւ���R�[�h���i�[���閼�O���
*/
namespace Entity {

	/**
	*	VertexData��UBO�ɓ]������
	*
	*	@param entity
	*	@param ubo
	*	@param matViewProjection
	*	@param viewFlags
	*/
	void UpdateUniformVertexData(Entity& entity, void*ubo, const glm::mat4* matVP, const glm::mat4& matDepthVP, glm::u32 viewFlags) {

		Uniform::VertexData data;

#ifdef ENTITY_NEW_VERSION
		data.matModel = entity.TransformMatrix();
#else
		data.matModel = entity.CalcModelMatrix();
#endif 

		data.matNormal = glm::mat4_cast(entity.Rotation());

		//�r���[ x �ˉe �s��̌v�Z
		for (int i = 0; i < Uniform::maxViewCount; ++i) {
			//viewFlag == false�̎��͍X�V���Ȃ�
			if (viewFlags & (1U << i)) {
				data.matMVP[i] = matVP[i] * data.matModel;
			}
		}

		data.matDepthMVP = matDepthVP * data.matModel;
		data.color = entity.Color();
		memcpy(ubo, &data, sizeof(data));
	}

	/**
	*	�ړ��E��]�E�g�k�s����v�Z����
	*
	*	@return TRS�s��
	*/
	glm::mat4 Entity::CalcModelMatrix()
	{

		//���łɌv�Z�ς݂̏ꍇ�͏��������Ȃ�
		if (isTransformUpdated == true) {
			return transformMatrix;
		}

		//�e�̃g�����X�t�H�[���s��̎擾(�����ꍇ�͒P�ʍs��)
		glm::mat4 parentMatrix = glm::mat4(1);
		if (parent) {
			parentMatrix = parent->transformMatrix;
		}

		///�g�����X�t�H�[���s��v�Z����
#ifdef ENTITY_NEW_VERSION //new code

		//���[�J����Ԃ̃g�����X�t�H�[���擾�ƌv�Z
		const glm::mat4 t = glm::translate(glm::mat4(), localTransform.position);
		const glm::mat4 r = glm::mat4_cast(localTransform.rotation);
		const glm::mat4 s = glm::scale(glm::mat4(), localTransform.scale);

		auto localMatrix = t * r * s;

		//���[���h��Ԃ̃g�����X�t�H�[���s��v�Z
		transformMatrix = localMatrix * parentMatrix;

		//�q�̃g�����X�t�H�[���X�V����
		for (auto& e : children) {


			e->CalcModelMatrix();
		}

#else	//TODO: �d�l�ύX�̂��ߌ�X�C��

			const glm::mat4 tw = glm::translate(glm::mat4(), transform.position);
			const glm::mat4 rw = glm::mat4_cast(transform.rotation);
			const glm::mat4 sw = glm::scale(glm::mat4(), transform.scale);
			transformMatrix = tw * rw * sw;
#endif
		return transformMatrix;
	}
	/**
	*	�G���e�B�e�B�̎��t������
	*/
	void Entity::AddChild(Entity* e){

		e->parent = this;
		children.push_back(e);
		CalcModelMatrix();
	}

	/**
	*	�w�肵���q�G���e�B�e�B����菜��
	*
	*	@param c ��菜���G���e�B�e�B
	*/
	void Entity::RemoveChild(Entity* c){

		//���g�̎q�Ɏ�菜���Ώ�c�����邩�ǂ������ׂ�
		auto itr = std::find_if(children.begin(), children.end(), 
			[c](const Entity* e) { return e == c; });

		if (itr != children.end()) {
			(*itr)->parent = nullptr;	//�q�G���e�B�e�B����̎Q�Ɛ؂藣��
			*itr = nullptr;				//���g�̎Q�Ƃ̐؂藣��
		}
	}

	/**
	*	�G���e�B�e�B��j������
	*
	*	���̊֐����Ăяo������́A�G���e�B�e�B�𑀍삵�Ă͂Ȃ�Ȃ�
	*/
	void Entity::Destroy() {

		if (pBuffer) {
			pBuffer->RemoveEntity(this);
		}
	}

	/**
	*	���[�U�[��`�p�̍X�V����
	*
	*	@param dt �o�ߎ���
	*/
	void Entity::Update(float dt){

		//�f�t�H���g�ł͉������Ȃ�
	}
	
	/*
	*	�G���e�B�e�B�̃V�X�e���X�V����
	*/
	void Entity::UpdateRecursive(float dt){

		//�����ŃG���e�B�e�B�̃V�X�e���X�V����������


		std::cout << "EntityName:" << (name.empty()? "root" : name) << std::endl;
		std::cout << "	Pos:" << transform.position.x << ","<< transform.position.y <<","<< transform.position.z<<"," <<std::endl;

		//�ړ��ʂɂ����W�X�V
		transform.position += velocity * static_cast<float>(dt);

		//���[�U�[��`�X�V�����Ăяo��(�������̂��ߏ]���̊֐��g�p)
		//Update(dt);
		if (updateFunc)updateFunc(*this,dt);

		if (children.size()) {
			std::cout << "child {" << std::endl;
			//�q�̍X�V�����Ăяo��
			for (auto e : children) {

				e->UpdateRecursive(dt);
			}
			std::cout << "}" << std::endl;
		}
	}

	/**
	*	�������g�������N���X�g����؂藣��
	*
	*	�����͂ǂ��ɂ��ڑ�����Ă��Ȃ���ԂɂȂ�
	*/
	void Buffer::Link::Remove(){
		next->prev = prev;
		prev->next = next;
		prev = this;
		next = this;
	}

	/**
	*	�����N�I�u�W�F�N�g�������̎�O�ɒǉ�����
	*
	*	@param p	�ǉ����郊���N�I�u�W�F�N�g�ւ̃|�C���^
	*
	*	p���������̃����N���X�g����؂藣���A�����̎�O�ɒǉ�����
	*/
	void Buffer::Link::Insert(Link* p)
	{
		p->Remove();
		p->prev = prev;
		p->next = this;
		prev->next = p;
		prev = p;
	}

	/**
	*	�G���e�B�e�B�o�b�t�@���쐬����
	*
	*	@param maxEntityCount	������G���e�B�e�B�̍ő吔
	*	@param ubSizePerEntity	�G���e�B�e�B���Ƃ�Uniform Buffer�̃o�C�g��
	*	@param bindingPoint		�G���e�B�e�B�pUBO�̃o�C���f�B���O�|�C���g
	*	@param ubName			�G���e�B�e�B�pUniform Buffer�̖��O
	*
	*	@return �쐬�����G���e�B�e�B�o�b�t�@�ւ̃|�C���^
	*/
	BufferPtr Buffer::Create(size_t maxEntityCount, GLsizeiptr ubSizePerEntity, int bindingPoint, const char* ubName) {


		struct Impl : Buffer { Impl() {} ~Impl() {} };
		BufferPtr p = std::make_shared<Impl>();
		if (!p) {
			std::cerr << "WARNING in Entity::Buffer::Create: �o�b�t�@�̍쐬�Ɏ��s." << std::endl;
			return {};
		}

		p->ubo = UniformBuffer::Create(maxEntityCount * ubSizePerEntity, bindingPoint, ubName);
		p->buffer.reset(new LinkEntity[maxEntityCount]);
		if (!p->ubo || !p->buffer) {
			std::cerr << "WARNING in Entity::Buffer::Create: �o�b�t�@�̍쐬�Ɏ��s." << std::endl;
			return {};
		}

		p->bufferSize = maxEntityCount;
		p->ubSizePerEntity = ubSizePerEntity;
		GLintptr offset = 0;
		const LinkEntity* const end = &p->buffer[maxEntityCount];
		for (LinkEntity* itr = &p->buffer[0]; itr != end; ++itr) {
			itr->uboOffset = offset;
			itr->pBuffer = p.get();
			p->freeList.Insert(itr);
			offset += ubSizePerEntity;
		}
		p->collisionHandlerList.reserve(maxGroupId);

		for (auto& e : p->visibilityFlags) {
			e = 1;	
		}

		return p;
	}

	/**
	*	�G���e�B�e�B��ǉ�����
	*
	*	@param position	�G���e�B�e�B�̍��W
	*	@param mesh		�G���e�B�e�B�̕\���Ɏg�p���郁�b�V��
	*	@param texture	�G���e�B�e�B�̕\���Ɏg���e�N�X�`��
	*	@param program	�G���e�B�e�B�̕\���Ɏg�p����V�F�[�_�v���O����
	*	@param func		�G���e�B�e�B�̏�Ԃ��X�V����֐�(�܂��͊֐��I�u�W�F�N�g)
	*
	*	@return �ǉ������G���e�B�e�B�ւ̃|�C���^
	*			���ꂱ��ȏ�G���e�B�e�B��ǉ��ł��Ȃ��ꍇ��nullptr���Ԃ����
	*			��]��g�嗦��ݒ肷��ꍇ�͌̃|�C���^�o�R�ōs��
	*			���̃|�C���^���A�v���P�[�V�������ŕێ�����K�v�͂Ȃ�
	*/
	Entity* Buffer::AddEntity(int groupId, const glm::vec3& position, const Mesh::MeshPtr& mesh, const TexturePtr texture[2], const Shader::ProgramPtr& program, Entity::UpdateFuncType func) {

		if (freeList.prev == freeList.next) {
			std::cerr << "WARNING in Entity::Buffer::AddEntity: "
				"�󂫃G���e�B�e�B������܂���." << std::endl;
			return nullptr;
		}

		if (groupId <0 || groupId > maxGroupId) {

			std::cerr << "ERROR in Entity::Buffer::AddEntity: �͈͊O�̃O���[�vID(" << groupId << ")���n����܂����B\n�O���[�vID��0�`" << maxGroupId << "�łȂ���΂Ȃ�܂���B" << std::endl;
			return nullptr;
		}

		LinkEntity* entity = static_cast<LinkEntity*>(freeList.prev);
		activeList[groupId].Insert(entity);

		//�G���e�B�e�B�f�[�^�̏���������
		entity->name = mesh->Name();
		entity->groupId = groupId;
		entity->transform = TransformData({ position,glm::vec3(1,1,1),glm::quat() });
		entity->localTransform = entity->transform;
		entity->velocity = glm::vec3();
		entity->color = glm::vec4(1);
		entity->mesh = mesh;
		entity->texture[0] = texture[0];
		entity->texture[1] = texture[1];
		entity->program = program;
		entity->updateFunc = func;
		entity->isActive = true;

		//entity->Init();
		rootNode.AddChild(entity);

		return entity;
	}

	/**
	*	�G���e�B�e�B���폜����
	*
	*	@param �폜����G���e�B�e�B�̃|�C���^
	*	TODO: �G���e�B�e�B�̐e�q�֌W�����������ō폜������
	*			���Ȃ��Ƃ����Ȃ����ߗv�C��
	*/
	void Buffer::RemoveEntity(Entity* entity) {

		if (!entity || !entity->isActive) {
			std::cerr << "WARNING in Entity::Buffer::RemoveEntity: "
				"��A�N�e�B�u�ȃG���e�B�e�B���폜���悤�Ƃ��܂���." << std::endl;
			return;
		}
		LinkEntity* p = static_cast<LinkEntity*>(entity);
		if (p < &buffer[0] || p >= &buffer[bufferSize]) {
			std::cerr << "WARNING in Entity::Buffer::RemoveEntity: "
				"�قȂ�o�b�t�@����擾�����G���e�B�e�B���폜���悤�Ƃ��܂���." << std::endl;
			return;
		}
		if (p == itrUpdate) {
			itrUpdate = p->prev;
		}
		if (p == itrUpdateRhs) {
			itrUpdateRhs = p->prev;
		}

		freeList.Insert(p);
		p->mesh.reset();
		for (auto& e : p->texture) {
			e.reset();
		}
		p->program.reset();
		p->updateFunc = nullptr;
		p->isActive = false;
	}

	/**
	*	�S�ẴG���e�B�e�B���폜����
	*/
	void Buffer::RemoveAllEntity() {

		for (int groupId = 0; groupId <= maxGroupId; ++groupId) {
			while (activeList[groupId].next != &activeList[groupId]) {
				RemoveEntity(static_cast<LinkEntity*>(activeList[groupId].next));
			}
		}
	}

	/**
	*	��`���m�̏Փ˔���
	*/
	bool HasCollision(const CollisionData& lhs, const CollisionData& rhs) {
		if (lhs.max.x < rhs.min.x || lhs.min.x > rhs.max.x)return false;
		if (lhs.max.y < rhs.min.y || lhs.min.y > rhs.max.y)return false;
		if (lhs.max.z < rhs.min.z || lhs.min.z > rhs.max.z)return false;
		return true;
	}

	/**
	*	�A�N�e�B�u�ȃG���e�B�e�B�̏�Ԃ��X�V����
	*
	*	@param delta	�O��̍X�V����̌o�ߎ���
	*	@param matView	View�s��
	*	@param matProj	Projection�s��
	*	@param matDepthVP �e�`��p�s��
	*/
	void Buffer::Update(double delta, const glm::mat4* matView, const glm::mat4& matProj, const glm::mat4& matDepthVP) {
		
		rootNode.UpdateRecursive(delta);
		rootNode.CalcModelMatrix();



		//�G���e�B�e�B�̍X�V����
		for (int groupId = 0; groupId <= maxGroupId; ++groupId) {
			for (itrUpdate = activeList[groupId].next; itrUpdate != &activeList[groupId]; itrUpdate = itrUpdate->next) {

				LinkEntity& e = *static_cast<LinkEntity*>(itrUpdate);

				//���[���h��Ԃ̏Փ˔���p���W�̍X�V����
				e.colWorld.min = e.colLocal.min + e.transform.position;
				e.colWorld.max = e.colLocal.max + e.transform.position;
			}
		}

		//�Փ˔��菈��
		for (const auto& e : collisionHandlerList) {
			if (!e.handler) {
				continue;
			}
			Link* listL = &activeList[e.groupId[0]];
			Link* listR = &activeList[e.groupId[1]];
			for (itrUpdate = listL->next; itrUpdate != listL; itrUpdate = itrUpdate->next) {
				LinkEntity* entityL = static_cast<LinkEntity*>(itrUpdate);
				for (itrUpdateRhs = listR->next; itrUpdateRhs != listR; itrUpdateRhs = itrUpdateRhs->next) {
					LinkEntity* entityR = static_cast<LinkEntity*>(itrUpdateRhs);
					if (!HasCollision(entityL->colWorld, entityR->colWorld)) {
						continue;
					}

					e.handler(*entityL, *entityR);
					if (entityL != itrUpdate) {
						break;
					}
				}
			}
		}
		itrUpdate = nullptr;
		itrUpdateRhs = nullptr;

		//�J�����̕ϊ��s��(�r���[�E�ˉe)�̌v�Z����
		uint8_t* p = static_cast<uint8_t*>(ubo->MapBuffer());
		glm::mat4 matVP[Uniform::maxViewCount];
		for (int i = 0; i < Uniform::maxViewCount; ++i) {
			//�J�������Ƃ̍s��v�Z����
			matVP[i] = matProj * matView[i];
		}

		//groupId���Ƃ�VertexData�X�V����
		for (int groupId = 0; groupId <= maxGroupId; ++groupId) {
			for (itrUpdate = activeList[groupId].next; itrUpdate != &activeList[groupId]; itrUpdate = itrUpdate->next) {
				LinkEntity& e = *static_cast<LinkEntity*>(itrUpdate);
				UpdateUniformVertexData(e, p + e.uboOffset, matVP, matDepthVP, visibilityFlags[groupId]);
			}
		}
		ubo->UnmapBuffer();
	}

	/**
	*	�A�N�e�B�u�ȃG���e�B�e�B��`�悷��
	*
	*	@param meshBuffer �`��Ɏg�p���郁�b�V���o�b�t�@�ւ̃|�C���^
	*/
	void Buffer::Draw(const Mesh::BufferPtr& meshBuffer) const {

		meshBuffer->BindVAO();
		for (int viewIndex = 0; viewIndex < Uniform::maxViewCount; ++viewIndex) {
			for (int groupId = 0; groupId <= maxGroupId; ++groupId) {
				//std::cout << "groupID: " << groupId << std::endl;

				if (!(visibilityFlags[groupId] & (1 << viewIndex))) {
					//�����Ȃ��G���e�B�e�B�͕`�悵�Ȃ�
					continue;
				}

				for (const Link* itr = activeList[groupId].next; itr != &activeList[groupId]; itr = itr->next) {

					const LinkEntity& e = *static_cast<const LinkEntity*>(itr);
					if (e.mesh && e.texture && e.program) {

						//std::cout << "DrawEntity name: " << e.name << std::endl;

						e.program->UseProgram();
						//e.program->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, e.texture->Id());
						for (size_t i = 0; i < sizeof(e.texture) / sizeof(e.texture[0]); ++i) {

							e.program->BindTexture(GL_TEXTURE0 + i, GL_TEXTURE_2D, e.texture[i]->Id());
						}
						e.program->SetViewIndex(viewIndex);
						ubo->BindBufferRange(e.uboOffset, ubSizePerEntity);
						e.mesh->Draw(meshBuffer);
					}
				}
			}
		}
	}

	/**
	*	�A�N�e�B�u�ȃG���e�B�e�B��`�悷��
	*
	*	@param meshBuffer �`��Ɏg�p���郁�b�V���o�b�t�@�ւ̃|�C���^
	*	tips �V�F�[�_�̐ݒ�͊O���ōs������
	*/
	void Buffer::DrawDepth(const Mesh::BufferPtr& meshBuffer) const {

		meshBuffer->BindVAO();
		for (int viewIndex = 0; viewIndex < Uniform::maxViewCount; ++viewIndex) {
			for (int groupId = 0; groupId <= maxGroupId; ++groupId) {
				if (!(visibilityFlags[groupId] & (1 << viewIndex))) {
					continue;
				}
				for (const Link* itr = activeList[groupId].next; itr != &activeList[groupId];
					itr = itr->next) {
					const LinkEntity& e = *static_cast<const LinkEntity*>(itr);
					if (e.mesh && e.texture && e.program) {
						for (size_t i = 0; i < sizeof(e.texture) / sizeof(e.texture[0]); ++i) {
							e.program->BindTexture(GL_TEXTURE0 + i, GL_TEXTURE_2D,
								e.texture[i]->Id());
						}
						ubo->BindBufferRange(e.uboOffset, ubSizePerEntity);
						e.mesh->Draw(meshBuffer);
					}
				}
			}
		}
	}

	/**
	*	�Փˉ����n���h����ݒ肷��
	*
	*	@param gid0		�ՓˑΏۂ̃O���[�vID
	*	@param gid1		�ՓˑΏۂ̃O���[�vID
	*	@param handler	�Փˉ����n���h��
	*
	*	�Փ˂��������Փ˃n���h�����Ăяo�����Ƃ��A
	*	��菬�����O���[�vID�����G���e�B�e�B�����ɓn�����B
	*	�R�R�Ŏw�肵���O���[�vID�̏����Ƃ͖��֌W�ł��邱�Ƃɒ���
	*
	*	CollisionHandler(10,1,Func)
	*	�Ƃ����R�[�h�Ńn���h����o�^�����Ƃ���A�Փ˂���������ƁA
	*	Func(�O���[�vID=1�̃G���e�B�e�B,�O���[�vID=10�̃G���e�B�e�B)
	*	�̂悤�ɌĂяo�����
	*/
	void Buffer::CollisionHandler(int gid0, int gid1, CollisionHandlerType handler) {

		if (gid0 > gid1) {
			std::swap(gid0, gid1);
		}
		auto itr = std::find_if(collisionHandlerList.begin(), collisionHandlerList.end(),
			[&](const CollisionHandlerInfo& e) {
			return e.groupId[0] == gid0 && e.groupId[1] == gid1; });
		if (itr == collisionHandlerList.end()) {
			collisionHandlerList.push_back({ { gid0,gid1 },handler });
		}
		else {
			itr->handler = handler;
		}
	}

	/**
	*	�Փˉ����n���h�����擾����
	*
	*	@param gid0	�ՓˑΏۂ̃O���[�vID
	*	@param gid1 �ՓˑΏۂ̃O���[�vID
	*
	*	@return �Փˉ����n���h��
	*/
	const CollisionHandlerType& Buffer::CollisionHandler(int gid0, int gid1) const {

		if (gid0 > gid1) {
			std::swap(gid0, gid1);
		}
		auto itr = std::find_if(collisionHandlerList.begin(), collisionHandlerList.end(),
			[&](const CollisionHandlerInfo& e) {
			return e.groupId[0] == gid0 && e.groupId[1] == gid1; });
		if (itr == collisionHandlerList.end()) {
			static const CollisionHandlerType dummy;
			return dummy;
		}
		return itr->handler;
	}

	/**
	*	�S�Ă̏Փˉ����n���h�����폜����
	*/
	void Buffer::ClearCollisionHanderList() {
		collisionHandlerList.clear();
	}
}