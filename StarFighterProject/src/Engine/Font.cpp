/**
*	@file Font.cpp
*	@brief	シーン内で描画するためのフォント
*	@author	takuya Yokoyama , tn-mai(講義資料製作者)
*/

#include "Font.h"
#include <memory>
#include <iostream>
#include <stdio.h>
#include "GameEngine.h"
#include "../../Res/Resource.h"

#pragma warning(disable: 4996)

/**
*	フォント描画機能を格納する名前空間
*/
namespace Font {

	/**
	*	フォント用頂点データ型
	*/
	struct Vertex {
		glm::vec2 position;	/// 座標
		glm::u16vec2 uv;	/// UV座標
		glm::u8vec4 color;	/// 色
	};

	bool Renderer::Init(size_t maxChar, const glm::vec2& screen) {

		if (maxChar > (USHRT_MAX + 1) / 4) {
			std::cerr << "[Warning]: FontRenderer::Init " << maxChar << "は設定可能な最大文字数を超えています" << std::endl;
			maxChar = (USHRT_MAX + 1) / 4;
		}

		//1文字4頂点を文字数分確保する
		vboCapacity = static_cast<GLsizei>(4 * maxChar);
		vbo.Init(GL_ARRAY_BUFFER, sizeof(Vertex)*vboCapacity, nullptr, GL_STREAM_DRAW);

		{	///頂点バッファの初期化

			std::vector<GLushort> tmp;
			tmp.resize(maxChar * 6);
			GLushort* p = tmp.data();
			for (GLushort i = 0; i < maxChar * 4; i += 4) {
				for (GLushort n : {0, 1, 2, 2, 3, 0}) {
					*(p++) = i + n;
				}
			}

			///インデックスバッファの初期化
			ibo.Init(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6 * maxChar, tmp.data(), GL_STATIC_DRAW);
		}

		vao.Init(vbo.Id(), ibo.Id());
		vao.Bind();
		vao.VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, position));
		vao.VertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(Vertex), offsetof(Vertex, uv));
		vao.VertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), offsetof(Vertex, color));
		vao.UnBind();

		progFont = GameEngine::Instance().Shader("Font");

		reciprocalScreenSize = 1.0f / screen;
		return true;
	}

	bool Renderer::LoadFromFile(const char* filename) {

		const std::unique_ptr<FILE, decltype(&fclose)> fp(fopen(filename, "r"), fclose);
		if (!fp) {
			std::cerr << "ERROR: '" << filename << "'のオープンに失敗" << std::endl;
			return false;
		}

		int line = 1; ///< フォントファイルの処理中の行.
		int ret = fscanf(fp.get(),
			"info face=%*s size=%*d bold=%*d italic=%*d charset=%*s unicode=%*d"
			" stretchH=%*d smooth=%*d aa=%*d padding=%*d,%*d,%*d,%*d spacing=%*d,%*d");
		++line;

		glm::vec2 scale;
		ret = fscanf(fp.get(),
			" common lineHeight=%*d base=%*d scaleW=%f scaleH=%f pages=%*d packed=%*d",
			&scale.x, &scale.y);
		if (ret < 2) {
			std::cerr << "ERROR: '" << filename << "'の読み込みに失敗(line=" << line <<
				")" << std::endl;
			return false;
		}
		const glm::vec2 reciprocalScale(1.0f / scale);
		++line;

		char tex[128];
		ret = fscanf(fp.get(), " page id=%*d file=%127s", tex);
		if (ret < 1) {
			std::cerr << "ERROR: '" << filename << "'の読み込みに失敗(line=" << line <<
				")" << std::endl;
			return false;
		}
		std::string texName;
		std::string res("Res/Texture/");
		texName.assign(tex + 1, tex + strlen(tex) - 1);

		texFilename = res + texName;

		texFilename.assign(res + texName);
		++line;

		int charCount;
		ret = fscanf(fp.get(), " chars count=%d", &charCount);
		if (ret < 1) {
			std::cerr << "ERROR: '" << filename << "'の読み込みに失敗(line=" << line <<
				")" << std::endl;
			return false;
		}
		++line;

		//
		fontList.resize(128);
		for (int i = 0; i < charCount; ++i) {
			FontInfo font;
			glm::vec2 uv;
			ret = fscanf(fp.get(),
				" char id=%d x=%f y=%f width=%f height=%f xoffset=%f yoffset=%f xadvance=%f"
				" page=%*d chnl=%*d", &font.id, &uv.x, &uv.y, &font.size.x, &font.size.y,
				&font.offset.x, &font.offset.y, &font.xadvance);
			if (ret < 8) {
				std::cerr << "ERROR: '" << filename << "'の読み込みに失敗(line=" << line <<
					")" << std::endl;
				return false;
			}
			font.offset.y *= -1;
			uv.y = scale.y - uv.y - font.size.y;
			font.uv[0] = uv * reciprocalScale * 65535.0f;
			font.uv[1] = (uv + font.size) * reciprocalScale * 65535.0f;
			if (font.id < 128) {
				fontList[font.id] = font;
			}
			++line;
		}

		// テクスチャを読み込む.
		if (!GameEngine::Instance().LoadTextureFromFile(texFilename.c_str())) {
			return false;
		}
		return true;
	}

	void Renderer::Color(const glm::vec4& c) {
		color = glm::clamp(c, 0.0f, 1.0f) * 255.0f;
	}

	glm::vec4 Renderer::Color() const {
		
		return glm::vec4(color) * (1.0f / 255.0f);
	}

	bool Renderer::AddString(const glm::vec2& position, const char* str, bool isCenter) {

		Vertex* p = pVBO + vboSize;
		glm::vec2 pos = position;

		//文字列の中心位置を特定する
		float stringSizeX = 0;
		glm::vec2 temp = position;
		if (isCenter) {
			for (const char* itr = str; *itr; ++itr) {

				const FontInfo& font = fontList[*itr];
				stringSizeX += font.xadvance * reciprocalScreenSize.x * scale.x;
			}
		}

		for (const char* itr = str; *itr; ++itr) {

			if (vboSize + 4 > vboCapacity) {
				break;
			}

			//itrに入っている文字に対応するフォントデータを取り出す
			const FontInfo& font = fontList[*itr];

			//
			if (font.id >= 0 && font.size.x && font.size.y) {

				const glm::vec2 size = font.size * reciprocalScreenSize * scale;
				glm::vec2 offsetedPos = pos + font.offset * reciprocalScreenSize * scale;
				offsetedPos.x -= stringSizeX * 0.5f;
				p[0].position = offsetedPos + glm::vec2(0, -size.y);
				p[0].uv = font.uv[0];
				p[0].color = color;
				p[1].position = offsetedPos + glm::vec2(size.x, -size.y);
				p[1].uv = glm::u16vec2(font.uv[1].x, font.uv[0].y);
				p[1].color = color;
				p[2].position = offsetedPos + glm::vec2(size.x, 0);
				p[2].uv = font.uv[1];
				p[2].color = color;
				p[3].position = offsetedPos;
				p[3].uv = glm::u16vec2(font.uv[0].x, font.uv[1].y);
				p[3].color = color;
				p += 4;
				vboSize += 4;
			}
			pos.x += font.xadvance * reciprocalScreenSize.x * scale.x;
		}


		return true;
	}

	void Renderer::MapBuffer() {

		if (pVBO) {
			return;
		}

		glBindBuffer(GL_ARRAY_BUFFER, vbo.Id());
		pVBO = static_cast<Vertex*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(Vertex)* vboCapacity, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		vboSize = 0;
	}

	void Renderer::UnmapBuffer() {

		if (!pVBO || vboSize == 0) {
			return;
		}

		glBindBuffer(GL_ARRAY_BUFFER, vbo.Id());
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		pVBO = nullptr;
	}

	void Renderer::Draw() const {

		if (vboSize == 0) {
			return;
		}

		vao.Bind();
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		progFont->UseProgram();
		progFont->BindTexture(GL_TEXTURE0,GameEngine::Instance().GetTexture(texFilename.c_str())->Id());
		glDrawElements(GL_TRIANGLES, (vboSize / 4) * 6, GL_UNSIGNED_SHORT, 0);
		vao.UnBind();
	}
}