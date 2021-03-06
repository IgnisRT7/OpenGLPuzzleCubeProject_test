/**
*	@file Shader.cpp
*/
#include "Shader.h"
#include <vector>
#include <iostream>
#include <cstdint>
#include <stdio.h>
#include <sys/stat.h>

namespace Shader {

	ProgramPtr Program::Create(const char* vsFilename, const char* fsFilename) {

		struct Impl : Program { Impl() {}~Impl() {} };
		ProgramPtr p = std::make_shared<Impl>();
		if (!p) {
			std::cerr << "[Error]: Program::Create "
				"プログラム'" << fsFilename << "'の作成に失敗" << std::endl;
			return {};
		}

		std::cout << "コンパイル中 " << vsFilename << " and " << fsFilename << std::endl;

		p->program = CreateProgramFromFile(vsFilename, fsFilename);
		if (!p->program) {
			return {};
		}

		//サンプラーの数と位置を取得する
		GLint activeUniforms;
		glGetProgramiv(p->program, GL_ACTIVE_UNIFORMS, &activeUniforms);
		for (int i = 0; i < activeUniforms; ++i) {
			GLint size;
			GLenum type;
			GLchar name[128];
			glGetActiveUniform(p->program, i, sizeof(name), nullptr, &size, &type, name);
			if (type == GL_SAMPLER_2D) {
				p->samperCount = size;
				p->samplerLocation = glGetUniformLocation(p->program, name);
				if (p->samplerLocation < 0) {
					std::cerr << "ERROR: プログラム'" << vsFilename << "'の作成に失敗しました" << std::endl;
					return {};
				}
				break;
			}
		}

		//各ロケーションの取得
		p->viewIndexLocation = glGetUniformLocation(p->program, locationNameList.viewIndexSampler);
		p->depthSamplerLocation = glGetUniformLocation(p->program, locationNameList.depthSampler);
		p->viewProjMatrixLocation = glGetUniformLocation(p->program, locationNameList.matVPSampler);

		auto result = glGetError();


		//頂点シェーダファイル名の末尾から".vert"を取り除いたものをプログラム名とする
		p->name = vsFilename;
		p->name.resize(p->name.size() - 4);

		return p;
	}

	Program::~Program() {
		if (program) {
			glDeleteProgram(program);
		}
	}

	bool Program::UniformBlockBinding(const char* blockName, GLuint bindingPoint) {

		const GLuint blockIndex = glGetUniformBlockIndex(program, blockName);
		if (blockIndex == GL_INVALID_INDEX) {
			std::cerr << "Error: Program::UniformBlockBinding プログラム名" <<
				name << "の Uniformブロック'" << blockName << "'が見つかりません" << std::endl;
			return false;
		}

		glUniformBlockBinding(program, blockIndex, bindingPoint);
		const GLenum result = glGetError();
		if (result != GL_NO_ERROR) {
			return false;
		}

		return true;
	}

	void Program::UseProgram() {

		glUseProgram(program);
		for (GLint i = 0; i < samperCount; ++i) {
			glUniform1i(samplerLocation + i, i);
		}

		if (depthSamplerLocation >= 0) {
			glUniform1i(depthSamplerLocation, 2);
		}
	}

	void Program::BindTexture(GLenum unit, GLuint texture, GLenum type) {

		if (unit >= GL_TEXTURE0 && unit < static_cast<GLenum>(GL_TEXTURE0 + samperCount)) {

			glActiveTexture(unit);
			glBindTexture(type, texture);
		}
	}

	void Program::BindShadowTexture(GLenum type, GLuint texture) {
		if (depthSamplerLocation >= 0) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(type, texture);
		}
	}

	void Program::SetViewIndex(int index) {

		if (viewIndexLocation >= 0) {
			glUniform1i(viewIndexLocation, index);
		}
	}

	void Program::SetBoolParameter(bool b, std::string name) {

		GLint bLocation = glGetUniformLocation(program, name.c_str());
		if (bLocation > 0) {
			glUniform1i(bLocation, b);
		}
	}

	void Program::SetFloatParameter(float f, std::string name){

		GLint fLocation = glGetUniformLocation(program, name.c_str());
		if (fLocation > 0) {
			glUniform1f(fLocation, f);
		}
	}

	void Program::SetVectorParameter(glm::vec3 p, std::string name) {
		GLint vecLocation = glGetUniformLocation(program, name.c_str());
		if (vecLocation > 0) {
			glUniform4fv(vecLocation, 1, glm::value_ptr(glm::vec4(p.x, p.y, p.z, 1.0f)));
		}
	}

	void Program::SetMatViewProjection(glm::mat4& matVP){

		this->matVP = matVP;

		if (viewProjMatrixLocation>=0) {
			glUniformMatrix4fv(viewProjMatrixLocation, 1, GL_FALSE, &matVP[0][0]);
		}
	}

	/**
	*	シェーダコードをコンパイルする
	*
	*	@param type シェーダの種類
	*	@param string シェーダコードへのポインタ
	*
	*	@return 作成したシェーダオブジェクト
	*/
	GLuint CompileShader(GLenum type, const GLchar* string) {

		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, &string, nullptr);
		glCompileShader(shader);
		GLint compiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
			GLint infoLen = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen) {
				std::vector<char> buf;
				buf.resize(infoLen);
				if (static_cast<int>(buf.size()) >= infoLen) {
					glGetShaderInfoLog(shader, infoLen, NULL, buf.data());
					std::cerr << "ERROR: シェーダのコンパイルに失敗\n" << buf.data() << std::endl;
				}

			}
			glDeleteShader(shader);
			return 0;
		}
		else {
			std::cout << "Successful." << std::endl;
		}
		return shader;
	}

	/**
	*	プログラムオブジェクトを作成する
	*
	*	@param vsCode 頂点シェーダコードへのポインタ
	*	@param fsCode フラグメントシェーダコードへのポインタ
	*
	*	@return 作成したプログラムオブジェクト
	*/
	GLuint CreateShaderProgram(const GLchar* vsCode, const GLchar* fsCode) {

		std::cout << "	vertex shader... ";
		GLuint vs = CompileShader(GL_VERTEX_SHADER, vsCode);

		std::cout << "	fragment shader... ";
		GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsCode);


		if (!vs || !fs) {
			return 0;
		}

		GLuint program = glCreateProgram();
		glAttachShader(program, fs);
		glDeleteShader(fs);
		glAttachShader(program, vs);
		glDeleteShader(vs);
		glLinkProgram(program);
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE) {
			GLint infoLen = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen) {
				std::vector<char> buf;
				buf.resize(infoLen);
				if (static_cast<int>(buf.size()) >= infoLen) {
					glGetProgramInfoLog(program, infoLen, NULL, buf.data());
					std::cerr << "ERROR: シェーダのリンクに失敗\n" << buf.data() << std::endl;
				}
			}
			glDeleteProgram(program);
			return 0;
		}

		std::cout << std::endl;

		return program;
	}


	/**
	*	ファイルを読み込む
	*
	*	@param filename 読み込むファイル名
	*	@param buf		読み込み先バッファ
	*
	*	@retval true	読み込み成功
	*	@retval false	読み込み失敗
	*/
	bool ReadFile(const char* filename, std::vector<char>&buf) {

		struct stat st;
		if (stat(filename, &st)) {
			return false;
		}

		FILE* fp = NULL;
		fopen_s(&fp, filename, "rb");
		if (!fp) {
			return false;
		}

		buf.resize(st.st_size + 1);
		const size_t readSize = fread(buf.data(), 1, st.st_size, fp);
		fclose(fp);
		if (readSize != st.st_size) {
			return false;
		}

		buf.back() = '\0';
		return true;

	}

	/**
	*	ファイルからシェーダプログラムを作成する
	*
	*	@param vsCode 頂点シェーダファイル名
	*	@param fsCode フラグメントシェーダファイル名
	*
	*	@return 作成したプログラムオブジェクト
	*/
	GLuint CreateProgramFromFile(const char* vsFilename, const char* fsFilename) {

		std::vector<char> vsBuf;
		if (!ReadFile(vsFilename, vsBuf)) {
			std::cerr << "ERROR in Shader::CreateProgramFromFile:\n" << vsFilename << "を読み込めません" << std::endl;
			return 0;
		}

		std::vector<char> fsBuf;
		if (!ReadFile(fsFilename, fsBuf)) {
			std::cerr << "ERROR in Shader::CreateProgramFromFile:\n" << fsFilename << "を読み込めません" << std::endl;
			return 0;
		}

		return CreateShaderProgram(vsBuf.data(), fsBuf.data());
	}

}


