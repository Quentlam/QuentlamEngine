#include "qlpch.h"
#include "SubTexture2D.h"

namespace Quentlam
{
	SubTexture2D::SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max)
		:m_Texture(texture)
	{
		m_TexCoords[0] = { min.x,min.y };
		m_TexCoords[1] = { max.x,min.y };
		m_TexCoords[2] = { max.x,max.y };
		m_TexCoords[3] = { min.x,max.y };


	}

	Ref<SubTexture2D> SubTexture2D::CreateFromCoords(const Ref<Texture2D>& texture,const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& spriteSize)
	{
		//寻找第x + 1行，第y + 1列的精灵(因为是从0开始索引的，并且从左下角开始的)
		//当前精灵图集的大小(通过定义最小点和最大点)
		//单个子贴图的宽高（spriteSize）(比例)
		//单个单元格的宽高（cellSize）(像素大小)
		glm::vec2 min = { (coords.x * cellSize.x / texture->GetWidth()),(coords.y * cellSize.y / texture->GetHeight()) };
		glm::vec2 max = { ((coords.x + spriteSize.x) * cellSize.x / texture->GetWidth()),((coords.y + spriteSize.y) * cellSize.y / texture->GetHeight()) };
		return CreateRef<SubTexture2D>(texture,min,max);
	}

}
