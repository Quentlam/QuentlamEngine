#include "Level.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Quentlam/Renderer/Texture.h"
#include "Quentlam/Renderer/Renderer2D.h"
#include "Random.h"

static glm::vec4 HSVtoRGB(const glm::vec3& hsv)
{
	int H = (int)(hsv.x * 360.0f);
	double S = hsv.y;
	double V = hsv.z;


	double C = S * V;
	double X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
	double m = V - C;
	double Rs, Gs, Bs;

	if (H >= 0 && H < 60)
	{
		Rs = C;
		Gs = X;
		Bs = 0;
	}
	else if (H >= 60 && H <= 120)
	{
		Rs = X;
		Gs = C;
		Bs = 0;
	}
	else if (H >= 120 && H <= 180)
	{
		Rs = 0;
		Gs = C;
		Bs = X;
	}
	else if (H >= 180 && H <= 240)
	{
		Rs = 0;
		Gs = X;
		Bs = C;
	}
	else if (H >= 240 && H <= 300)
	{
		Rs = X;
		Gs = 0;
		Bs = C;
	}
	else 
	{
		Rs = C;
		Gs = 0;
		Bs = X;
	}
	return { (Rs + m),(Gs + m),(Bs + m),1.0f };

}


static bool PointInTri(const glm::vec2& p, glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2)
{
	float s = p0.y * p2.x - p0.x * p2.y + (p2.y - p0.y) * p.x + (p0.x - p2.x) * p.y;
	float t = p0.x * p1.y - p0.y * p1.x + (p0.y - p1.y) * p.x + (p1.x - p0.x) * p.y;

	if ((s < 0) != (t > 0))
	{
		return false;
	}
	float  A = -p1.y * p2.x + p0.y * (p2.x - p1.x) + p0.x * (p1.y - p2.y) + p1.x * p2.y;
	return A < 0 ?
		(s <= 0 && s + t >= A) :
		(s >= 0 && s + t <= A);


}




void Level::Init()
{
	m_TriangleTexture = Quentlam::Texture2D::Create("assets/texture/Triangle.png");
	m_Player.LoadAssets();

	m_Pillars.resize(5);
	for (int i = 0; i < 5; i++)
		CreatePillar(i, i * 10.0f);

}

void Level::OnUpdate(Quentlam::Timestep ts)
{
	m_Player.OnUpdate(ts);
		
	if (CollisionTest())
	{
		GameOver();
		return;
	}
		
	m_PillarHSV.x += 0.1f * ts;
	if (m_PillarHSV.x > 1.0f)
		m_PillarHSV.x = 0.0f;


	if (m_Player.GetPosition().x > m_PillarsTarget)
	{
		CreatePillar(m_PillarsIndex, m_PillarsTarget + 20.0f);
		m_PillarsIndex = ++m_PillarsIndex % m_Pillars.size();
		m_PillarsTarget += 10.0f;
	}
}

void Level::OnRender()
{
	const auto& playerPos = m_Player.GetPosition();
	
	glm::vec4 color = HSVtoRGB(m_PillarHSV);

	//Background
	Quentlam::Renderer2D::DrawQuad({ playerPos.x,0.0f,-0.8f }, { 50.0f,50.0f }, { 0.3f,0.3f,0.3f,1.0f });


	//Floor and ceiling
	Quentlam::Renderer2D::DrawQuad({ playerPos.x,34.0f }, { 50.0f,50.0f }, color);
	Quentlam::Renderer2D::DrawQuad({ playerPos.x,-34.0f }, { 50.0f,50.0f }, color);
	
	for(auto& pillar : m_Pillars)
	{
		Quentlam::Renderer2D::DrawRotatedQuad(pillar.TopPosition, pillar.TopScale, m_TriangleTexture, glm::radians(180.0f), 1.0f,color);
		Quentlam::Renderer2D::DrawRotatedQuad(pillar.BottomPosition, pillar.BottomScale, m_TriangleTexture, 0.0f ,1.0f, color);
	}
	m_Player.OnRender();
}

void Level::OnImGuiRender()
{
	m_Player.OnImGuiRenderer();
}

void Level::CreatePillar(int index, float offset)
{
	Pillar& pillar = m_Pillars[index];
	pillar.TopPosition.x = offset;
	pillar.BottomPosition.x = offset;
	pillar.TopPosition.z = index * 0.1f - 0.5f;
	pillar.BottomPosition.z = index * 0.1f - 0.5f + 0.05f;
	
	float center = Random::Float() * 35.0f - 17.5f;

	//计算三角形的上下的缝隙
	float gap = 2.0f + Random::Float() * 5.0f;
	float minClearance = 0.75f;
	float maxGap = (pillar.TopScale.y + pillar.BottomScale.y) * 0.5f - 16.0f - minClearance;
	if (maxGap < 0.0f) maxGap = 0.0f;
	if (gap > maxGap) gap = maxGap;

	pillar.TopPosition.y = 10.0f - ((10.0f - center) * 0.2f) + gap * 0.5f;
	pillar.BottomPosition.y = -10.0f - ((-10.0f - center) * 0.2f) - gap * 0.5f;
}


bool Level::CollisionTest()
{
	if (glm::abs(m_Player.GetPosition().y) > 8.5f)
		return true;

	glm::vec4 playerVertices[4] =
	{
		{-0.5f,-0.5f,0.0f,1.0f},
		{0.5f, -0.5f,0.0f,1.0f},
		{0.5f, 0.5f, 0.0f,1.0f},
		{-0.5f,0.5f, 0.0f,1.0f},
	};

	const auto& pos = m_Player.GetPosition();
	glm::vec4 playerTransformedVerts[4];
	for (int i = 0; i < 4; i++)
	{
		playerTransformedVerts[i] = glm::translate(glm::mat4(1.0f), { pos.x,pos.y,0.0f })
			* glm::rotate(glm::mat4(1.0f), glm::radians(m_Player.GetRotation()), { 0.0f,0.0f,1.0f })
			* glm::scale(glm::mat4(1.0f), { 1.0f,1.3f,1.0f })
			* playerVertices[i];
	}

	glm::vec2 pv[4] =
	{
		{ playerTransformedVerts[0].x, playerTransformedVerts[0].y },
		{ playerTransformedVerts[1].x, playerTransformedVerts[1].y },
		{ playerTransformedVerts[2].x, playerTransformedVerts[2].y },
		{ playerTransformedVerts[3].x, playerTransformedVerts[3].y },
	};

	glm::vec4 pillarVertices[3] =
	{
		{-0.5f + 0.1f , -0.5f + 0.1f ,0.0f ,1.0f},
		{ 0.5f - 0.1f , -0.5f + 0.1f ,0.0f ,1.0f},
		{ 0.0f + 0.0f ,  0.5f - 0.1f ,0.0f ,1.0f},
	};

	for (auto& p : m_Pillars)
	{
		glm::vec2 tri[3];

		//Top pillars
		for (int i = 0; i < 3; i++)
		{
			tri[i] = glm::translate(glm::mat4(1.0f), { p.TopPosition.x,p.TopPosition.y,0.0f })
				* glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), { 0.0f,0.0f,1.0f })
				* glm::scale(glm::mat4(1.0f), { p.TopScale.x,p.TopScale.y,1.0f })
				* pillarVertices[i];
		}

		float eps = 1e-6f;
		for (int ei = 0; ei < 4; ++ei)
		{
			int ej = (ei + 1) % 4;
			for (int ti = 0; ti < 3; ++ti)
			{
				int tj = (ti + 1) % 3;
				glm::vec2 a = pv[ei];
				glm::vec2 b = pv[ej];
				glm::vec2 c = tri[ti];
				glm::vec2 d = tri[tj];
				glm::vec2 ab = b - a;
				glm::vec2 cd = d - c;
				float c1 = ab.x * (c.y - a.y) - ab.y * (c.x - a.x);
				float c2 = ab.x * (d.y - a.y) - ab.y * (d.x - a.x);
				float c3 = cd.x * (a.y - c.y) - cd.y * (a.x - c.x);
				float c4 = cd.x * (b.y - c.y) - cd.y * (b.x - c.x);
				bool inter = false;
				if (glm::abs(c1) <= eps)
				{
					float minx = glm::min(a.x, b.x), maxx = glm::max(a.x, b.x);
					float miny = glm::min(a.y, b.y), maxy = glm::max(a.y, b.y);
					inter = (c.x >= minx - eps && c.x <= maxx + eps && c.y >= miny - eps && c.y <= maxy + eps);
				}
				if (!inter && glm::abs(c2) <= eps)
				{
					float minx = glm::min(a.x, b.x), maxx = glm::max(a.x, b.x);
					float miny = glm::min(a.y, b.y), maxy = glm::max(a.y, b.y);
					inter = (d.x >= minx - eps && d.x <= maxx + eps && d.y >= miny - eps && d.y <= maxy + eps);
				}
				if (!inter && glm::abs(c3) <= eps)
				{
					float minx = glm::min(c.x, d.x), maxx = glm::max(c.x, d.x);
					float miny = glm::min(c.y, d.y), maxy = glm::max(c.y, d.y);
					inter = (a.x >= minx - eps && a.x <= maxx + eps && a.y >= miny - eps && a.y <= maxy + eps);
				}
				if (!inter && glm::abs(c4) <= eps)
				{
					float minx = glm::min(c.x, d.x), maxx = glm::max(c.x, d.x);
					float miny = glm::min(c.y, d.y), maxy = glm::max(c.y, d.y);
					inter = (b.x >= minx - eps && b.x <= maxx + eps && b.y >= miny - eps && b.y <= maxy + eps);
				}
				if (!inter)
					inter = (c1 * c2 < 0 && c3 * c4 < 0);
				if (inter)
					return true;
			}
		}

		for (int ei = 0; ei < 4; ++ei)
		{
			int ej = (ei + 1) % 4;
			for (int ti = 0; ti < 3; ++ti)
			{
				int tj = (ti + 1) % 3;
				glm::vec2 a = pv[ei];
				glm::vec2 b = pv[ej];
				glm::vec2 c = tri[ti];
				glm::vec2 d = tri[tj];
				glm::vec2 ab = b - a;
				glm::vec2 cd = d - c;
				float c1 = ab.x * (c.y - a.y) - ab.y * (c.x - a.x);
				float c2 = ab.x * (d.y - a.y) - ab.y * (d.x - a.x);
				float c3 = cd.x * (a.y - c.y) - cd.y * (a.x - c.x);
				float c4 = cd.x * (b.y - c.y) - cd.y * (b.x - c.x);
				bool inter = false;
				if (glm::abs(c1) <= eps)
				{
					float minx = glm::min(a.x, b.x), maxx = glm::max(a.x, b.x);
					float miny = glm::min(a.y, b.y), maxy = glm::max(a.y, b.y);
					inter = (c.x >= minx - eps && c.x <= maxx + eps && c.y >= miny - eps && c.y <= maxy + eps);
				}
				if (!inter && glm::abs(c2) <= eps)
				{
					float minx = glm::min(a.x, b.x), maxx = glm::max(a.x, b.x);
					float miny = glm::min(a.y, b.y), maxy = glm::max(a.y, b.y);
					inter = (d.x >= minx - eps && d.x <= maxx + eps && d.y >= miny - eps && d.y <= maxy + eps);
				}
				if (!inter && glm::abs(c3) <= eps)
				{
					float minx = glm::min(c.x, d.x), maxx = glm::max(c.x, d.x);
					float miny = glm::min(c.y, d.y), maxy = glm::max(c.y, d.y);
					inter = (a.x >= minx - eps && a.x <= maxx + eps && a.y >= miny - eps && a.y <= maxy + eps);
				}
				if (!inter && glm::abs(c4) <= eps)
				{
					float minx = glm::min(c.x, d.x), maxx = glm::max(c.x, d.x);
					float miny = glm::min(c.y, d.y), maxy = glm::max(c.y, d.y);
					inter = (b.x >= minx - eps && b.x <= maxx + eps && b.y >= miny - eps && b.y <= maxy + eps);
				}
				if (!inter)
					inter = (c1 * c2 < 0 && c3 * c4 < 0);
				if (inter)
					return true;
			}
		}

		for (auto& vert : playerTransformedVerts)
		{
			if (PointInTri({ vert.x,vert.y }, tri[0], tri[1], tri[2]))
				return true;
		}
		
		//Bottom pillars
		for (int i = 0; i < 3; i++)
		{
			tri[i] = glm::translate(glm::mat4(1.0f), { p.BottomPosition.x,p.BottomPosition.y,0.0f })
				* glm::scale(glm::mat4(1.0f), { p.BottomScale.x,p.BottomScale.y,1.0f })
				* pillarVertices[i];
		}	

		for (int ei = 0; ei < 4; ++ei)
		{
			int ej = (ei + 1) % 4;
			for (int ti = 0; ti < 3; ++ti)
			{
				int tj = (ti + 1) % 3;
				glm::vec2 a = pv[ei];
				glm::vec2 b = pv[ej];
				glm::vec2 c = tri[ti];
				glm::vec2 d = tri[tj];
				glm::vec2 ab = b - a;
				glm::vec2 cd = d - c;
				float c1 = ab.x * (c.y - a.y) - ab.y * (c.x - a.x);
				float c2 = ab.x * (d.y - a.y) - ab.y * (d.x - a.x);
				float c3 = cd.x * (a.y - c.y) - cd.y * (a.x - c.x);
				float c4 = cd.x * (b.y - c.y) - cd.y * (b.x - c.x);
				bool inter = false;
				if (glm::abs(c1) <= eps)
				{
					float minx = glm::min(a.x, b.x), maxx = glm::max(a.x, b.x);
					float miny = glm::min(a.y, b.y), maxy = glm::max(a.y, b.y);
					inter = (c.x >= minx - eps && c.x <= maxx + eps && c.y >= miny - eps && c.y <= maxy + eps);
				}
				if (!inter && glm::abs(c2) <= eps)
				{
					float minx = glm::min(a.x, b.x), maxx = glm::max(a.x, b.x);
					float miny = glm::min(a.y, b.y), maxy = glm::max(a.y, b.y);
					inter = (d.x >= minx - eps && d.x <= maxx + eps && d.y >= miny - eps && d.y <= maxy + eps);
				}
				if (!inter && glm::abs(c3) <= eps)
				{
					float minx = glm::min(c.x, d.x), maxx = glm::max(c.x, d.x);
					float miny = glm::min(c.y, d.y), maxy = glm::max(c.y, d.y);
					inter = (a.x >= minx - eps && a.x <= maxx + eps && a.y >= miny - eps && a.y <= maxy + eps);
				}
				if (!inter && glm::abs(c4) <= eps)
				{
					float minx = glm::min(c.x, d.x), maxx = glm::max(c.x, d.x);
					float miny = glm::min(c.y, d.y), maxy = glm::max(c.y, d.y);
					inter = (b.x >= minx - eps && b.x <= maxx + eps && b.y >= miny - eps && b.y <= maxy + eps);
				}
				if (!inter)
					inter = (c1 * c2 < 0 && c3 * c4 < 0);
				if (inter)
					return true;
			}
		}

		for (auto& vert : playerTransformedVerts)
		{
			if (PointInTri({ vert.x,vert.y }, tri[0], tri[1], tri[2]))
				return true;
		}

	}
	return false;
}

void Level::GameOver()
{
	m_Gameover = true;
}

void Level::Reset()
{
	m_Gameover = false;

	m_Player.Reset();

	m_PillarsTarget = 30.0f;
	m_PillarsIndex = 0;
	for (int i = 0; i < 5; i++)
		CreatePillar(i, i * 10.0f);
}
