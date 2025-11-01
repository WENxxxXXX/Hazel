#pragma once

#include <glm/glm.hpp>

#include "Hazel/Renderer/Texture.h"

namespace Hazel
{

	class SubTexture2D
	{
	public:
		// cellSize: 单个sprite的宽高（像素级）
		// spritePos: sprite的位置，x方向第几个，y方向第几个（决定左下角位置）
		// spriteSize: sprite的宽高分别占几个单位（决定宽度、高度）
		SubTexture2D(
			const Ref<Texture2D>& texture, const glm::vec2& cellSize,
			const glm::vec2& spritePos, const glm::vec2& spriteSize
		);

		static Ref<SubTexture2D> Create(const Ref<Texture2D>& texture, const glm::vec2& cellSize, const glm::vec2& spritePos, const glm::vec2& spriteSize = { 1, 1 });

		const Ref<Texture2D>& GetTexture() const { return m_Texture; }
		const glm::vec2* GetCoords() const { return m_TexCoords; }
	private:
		Ref<Texture2D> m_Texture;

		glm::vec2 m_TexCoords[4];
	};

}