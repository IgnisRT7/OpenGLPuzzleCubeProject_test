/**
*	@file EnemyMoveController
*	@brief	敵の移動システムの制御用
*	@author	Takuya Yokoyama , tn-mai(講義資料の製作者)
*/

#pragma once
#include "../Engine/Entity.h"

/**
*	移動制御のパーツクラス
*	tips:	エンティティの動作とその動作にかかる時間などを管理する
*/
class MovePart {
public:

	MovePart() = default;
	MovePart(float d);
	MovePart& operator=(const MovePart&) = delete;
	virtual ~MovePart() = default;

	float Duration() const { return duration; }
	void Duration(float t) { duration = t; }

	virtual void Initialize(Entity::Entity& e) { }
	virtual void Update(Entity::Entity& e,float d) = 0;



protected:
	
	float duration;		///< 経過時間
};

using MovePartPtr = std::shared_ptr<MovePart>;

/**
*	移動制御クラス
*/
class MoveController {
public:

	MoveController() = default;

	/**
	*	コンストラクタ
	*
	*	@param p	操作する動作パーツ
	*/
	explicit MoveController(const MovePartPtr& p);
	MoveController(const MoveController&) = delete;
	MoveController& operator=(const MoveController&) = delete;

	/**
	*	更新処理
	*
	*	@param entity	制御するエンティティ
	*	@param delta	経過時間
	*/
	void Update(Entity::Entity& entity, float delta);

private:

	MovePartPtr part;	///< 制御する動作クラス
	float elapsedTime;	///< 経過時間

	bool isInitialized = false;	///< 初期化フラグ
	bool isLoop = false;		///< ループ動作フラグ
};

using MoveControllerPtr = std::shared_ptr<MoveController>;

/**
*	まっすぐに進む動作クラス
*/
class MoveStraight : public MovePart {
public:

	MoveStraight() = default;

	/**
	*	コンストラクタ
	*
	*	@param d	動作時間
	*	@param v	移動距離
	*/
	MoveStraight(float d,const glm::vec3& v);

	MoveStraight(const MoveStraight&) = delete;
	MoveStraight& operator=(const MoveStraight&) = delete;
	virtual ~MoveStraight() = default;

	/**
	*	初期化処理
	*
	*	@param entity	制御するエンティティ
	*/
	virtual void Initialize(Entity::Entity& p) override;

	/**
	*	更新処理
	*
	*	@param entity		制御するエンティティ
	*	@param elapsedTime	経過時間
	*/
	void Update(Entity::Entity& e,float d) override;

private:

	glm::vec3 start;	///< スタート地点(エンティティ初期位置)
	glm::vec3 moveVel;	///< スタートからの移動量
	float ratio;		///< 現在の進行状態 1になると完了
};

/**
*	円を描いて移動するクラス
*/
class MoveCircle : public MovePart {
public:

	MoveCircle() = default;

	/**
	*	コンストラクタ
	*
	*	@param d	動作時間
	*	@param a	移動角度
	*	@param r	半径
	*/
	MoveCircle(float d,float a,float o, float r);
	MoveCircle(const MoveCircle&) = delete;
	MoveCircle& operator=(const MoveCircle&) = delete;
	virtual ~MoveCircle() = default;

	/**
	*	初期化処理
	*
	*	@param entity	制御するエンティティ
	*/
	virtual void Initialize(Entity::Entity& p)override;

	/**
	*	更新処理
	*
	*	@param entity	制御するエンティティ
	*	@param elapsedTime	経過時間
	*/
	void Update(Entity::Entity& e, float d)override;

private:

	float angle;
	float offsetAngle;
	float range;
	glm::vec3 center;
};

/**
*	MovePartをシーケンス実行するクラス
*/
class MovePartSequencer : public MovePart{
public:

	MovePartSequencer() = default;
	MovePartSequencer(const MovePartSequencer&) = delete;
	MovePartSequencer& operator=(const MovePartSequencer&) = delete;
	virtual ~MovePartSequencer() = default;

	/**
	*	初期化処理
	*
	*	@param entity	制御対象のエンティティ
	*/
	virtual void Initialize(Entity::Entity& p) override;

	/**
	*	更新処理
	*
	*	@param entity		制御対象のエンティティ
	*	@param elapsedTime	経過時間
	*/
	virtual void Update(Entity::Entity& p, float d) override;

	/**
	*	動作リストに追加
	*
	*	@param p	追加する動作パーツ
	*/
	void Add(const MovePartPtr p);

private:

	/**
	*	次に実行させる動作パーツの設定をする
	*
	*	@param entity	制御対象のエンティティ
	*
	*	@retval true	次に動作させるパーツが存在し設定した
	*	@retval false	次に動作させるパーツが存在しない(last index)
	*/
	bool NextMove(Entity::Entity&);

	std::vector<MovePartPtr> movList;	///< 動作パーツのリスト
	int execIndex = -1;					///< 現在実行中の動作パーツ
	float currentTimeBegin;				///< 実行中の動作パーツの開始時間
	float currentTimeEnd;				///< 実行中の動作パーツの終了時間
};
