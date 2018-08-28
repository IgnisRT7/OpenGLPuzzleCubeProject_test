/**
*	@file OffscreenBuffer.cpp
*/
#pragma once
#include "OffscreenBuffer.h"

/**
*	�I�t�X�N���[���o�b�t�@���쐬����
*
*	@param w �I�t�X�N���[���o�b�t�@�̕�(�s�N�Z���P��)	
*	@param h �I�t�X�N���[���o�b�t�@�̍���(�s�N�Z���P��)
*
*	@return �쐬�����I�t�X�N���[���o�b�t�@�ւ̃|�C���^
*/
OffscreenBufferPtr OffscreenBuffer::Create(int w, int h) {

	struct Impl : OffscreenBuffer {};
	OffscreenBufferPtr offscreen = std::make_shared<Impl>();
	if (!offscreen) {
		return offscreen;
	}


	offscreen->tex = Texture::Create(w, h, GL_RGBA8, GL_RGBA, nullptr);

	//�[�x�o�b�t�@�̍쐬
	glGenRenderbuffers(1, &offscreen->depthbuffer);						//�[�x�o�b�t�@�̃������m��
	glBindRenderbuffer(GL_RENDERBUFFER, offscreen->depthbuffer);			//�[�x�o�b�t�@�̕R�Â�
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);	//���̃o�b�t�@�̗p�r�ƃT�C�Y�����߂�
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//�t���[���o�b�t�@�̍쐬
	glGenFramebuffers(1, &offscreen->framebuffer);						//�t���[���o�b�t�@�̃������m��
	glBindFramebuffer(GL_FRAMEBUFFER, offscreen->framebuffer);			//�t���[���o�b�t�@�̕R�Â�(�ǂݏ����\)
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, offscreen->depthbuffer);	//�[�x�o�b�t�@���t���[���o�b�t�@�ɕR�Â�	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, offscreen->tex->Id(), 0);	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return offscreen;
}

/**
*	�f�X�g���N�^
*/
OffscreenBuffer::~OffscreenBuffer() {

	if (framebuffer) {
		glDeleteFramebuffers(1, &framebuffer);
	}
	if (depthbuffer) {
		glDeleteRenderbuffers(1, &depthbuffer);
	}
}