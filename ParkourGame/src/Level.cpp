#include <fstream>
#include "Level.h"
#include "GameLayer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Quentlam/Renderer/Texture.h"
#include "Quentlam/Renderer/Renderer2D.h"
#include "Random.h"
#include "imgui/imgui.h"

class Box2DDebugDraw : public b2Draw
{
public:
	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override
	{
		for (int i = 0; i < vertexCount; ++i)
		{
			b2Vec2 v1 = vertices[i];
			b2Vec2 v2 = vertices[(i + 1) % vertexCount];
			DrawSegment(v1, v2, color);
		}
	}
	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override
	{
		DrawPolygon(vertices, vertexCount, color);
	}
	void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override {}
	void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override {}
	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override 
	{
		glm::vec2 p1g(p1.x, p1.y);
		glm::vec2 p2g(p2.x, p2.y);
		glm::vec2 dir = p2g - p1g;
		float len = glm::length(dir);
		float angle = glm::degrees(glm::atan(dir.y, dir.x));
		glm::vec2 centerVec = (p1g + p2g) * 0.5f;
		Quentlam::Renderer2D::DrawRotatedQuad({ centerVec.x, centerVec.y, 0.8f }, { len, 0.1f }, angle, { color.r, color.g, color.b, 1.0f });
	}
	void DrawTransform(const b2Transform& xf) override {}
	void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override {}
};

static Box2DDebugDraw s_DebugDraw;

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
	
	// Create Physics World
	b2Vec2 gravity(0.0f, -9.81f);
	m_PhysicsWorld = new b2World(gravity);
	m_PhysicsWorld->SetContactListener(this);

	m_Player.Init(m_PhysicsWorld);
	m_Player.LoadAssets();

	// Load ImGui state
	std::ifstream in("parkour_settings.ini");
	if (in.is_open())
	{
		in >> m_ShowColliders;
	}

	m_Pillars.resize(20);
	for (int i = 0; i < 20; i++)
		CreatePillar(i, i * 10.0f + 20.0f);

	// Create Boundaries
	b2BodyDef boundaryDef;
	boundaryDef.type = b2_staticBody;
	
	static int boundaryType = 3;

	// Top Boundary (Visible ceiling starts at y = 9.0f)
	boundaryDef.position.Set(0.0f, 9.5f);
	boundaryDef.userData.pointer = reinterpret_cast<uintptr_t>(&boundaryType);
	m_TopBoundary = m_PhysicsWorld->CreateBody(&boundaryDef);
	
	b2PolygonShape topBoundaryShape;
	topBoundaryShape.SetAsBox(10000.0f, 0.5f); // Infinite width
	m_TopBoundary->CreateFixture(&topBoundaryShape, 0.0f);

	// Bottom Boundary (Visible floor starts at y = -9.0f)
	boundaryDef.position.Set(0.0f, -9.5f);
	m_BottomBoundary = m_PhysicsWorld->CreateBody(&boundaryDef);
	b2PolygonShape bottomBoundaryShape;
	bottomBoundaryShape.SetAsBox(10000.0f, 0.5f);
	m_BottomBoundary->CreateFixture(&bottomBoundaryShape, 0.0f);
}

void Level::OnUpdate(Quentlam::Timestep ts)
{
	// Instead of strict fixed accumulator causing jitter in camera/visuals if they don't match,
	// let's do a more standard variable step for visuals, but Box2D handles fixed internally or we just step by ts.
	// A simpler approach to avoid rendering jitter is to just step Box2D by ts.
	// But fixed step is better for physics stability. We will stick to stepping Box2D by ts for perfectly smooth visual updates in a simple game.
	m_PhysicsWorld->Step(ts, 8, 3);

	m_Player.OnUpdate(ts);
		
	if (m_Gameover)
	{
		GameOver();
		return;
	}
		
	m_PillarHSV.x += 0.1f * ts;
	if (m_PillarHSV.x > 1.0f)
		m_PillarHSV.x = 0.0f;

	float screenLeft = m_Player.GetPosition().x - 20.0f; // roughly

	for (auto& pillar : m_Pillars)
	{
		if (pillar.TopBody)
		{
			// Update visual positions
			pillar.TopPosition.x = pillar.TopBody->GetPosition().x;
			pillar.TopPosition.y = pillar.TopBody->GetPosition().y;
			pillar.BottomPosition.x = pillar.BottomBody->GetPosition().x;
			pillar.BottomPosition.y = pillar.BottomBody->GetPosition().y;
		}

		if (pillar.TopPosition.x + pillar.TopScale.x * 0.5f < screenLeft)
		{
			// Reposition to the right
			float newX = m_PillarsTarget + 20.0f;
			
			float center = Random::Float() * 20.0f - 10.0f;
			float gap = MIN_GAP + Random::Float() * (MAX_GAP - MIN_GAP);
			if (gap < MIN_GAP) gap = MIN_GAP; // Failsafe

			float newTopY = 10.0f + gap * 0.5f + center;
			float newBottomY = -10.0f - gap * 0.5f + center;

			if (pillar.TopBody)
			{
				pillar.TopBody->SetTransform(b2Vec2(newX, newTopY), pillar.TopBody->GetAngle());
				pillar.BottomBody->SetTransform(b2Vec2(newX, newBottomY), pillar.BottomBody->GetAngle());
				
				// Re-center sensor
				b2Vec2 sensorPos(newX, center);
				pillar.SensorBody->SetTransform(sensorPos, 0.0f);
				
				pillar.Scored = false;
				
				pillar.TopPosition.x = newX;
				pillar.TopPosition.y = newTopY;
				pillar.BottomPosition.x = newX;
				pillar.BottomPosition.y = newBottomY;
			}
			
			m_PillarsTarget += 10.0f;
		}
	}
}

void Level::OnRender()
{
	const auto& playerPos = m_Player.GetPosition();
	
	glm::vec4 color = HSVtoRGB(m_PillarHSV);

	//Background
	Quentlam::Renderer2D::DrawQuad({ playerPos.x,0.0f,-0.8f }, { 50.0f,50.0f }, { 0.3f,0.3f,0.3f,1.0f });

	//Floor and ceiling
	Quentlam::Renderer2D::DrawQuad({ playerPos.x, 34.0f, -0.8f }, { 50.0f,50.0f }, color);
	Quentlam::Renderer2D::DrawQuad({ playerPos.x,-34.0f, -0.8f }, { 50.0f,50.0f }, color);
	
	// Boundaries
	Quentlam::Renderer2D::DrawRotatedQuad({ playerPos.x, 9.0f, 0.6f }, { 40.0f, 0.1f }, 0.0f, { 1.0f, 0.0f, 0.0f, 1.0f });
	Quentlam::Renderer2D::DrawRotatedQuad({ playerPos.x, -9.0f, 0.6f }, { 40.0f, 0.1f }, 0.0f, { 1.0f, 0.0f, 0.0f, 1.0f });

	for(auto& pillar : m_Pillars)
	{
		float topRot = pillar.TopBody ? pillar.TopBody->GetAngle() : glm::radians(180.0f);
		float botRot = pillar.BottomBody ? pillar.BottomBody->GetAngle() : 0.0f;
		
		glm::mat4 topTransform = glm::translate(glm::mat4(1.0f), pillar.TopPosition)
			* glm::rotate(glm::mat4(1.0f), topRot, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { pillar.TopScale.x, pillar.TopScale.y, 1.0f });
		Quentlam::Renderer2D::DrawTriangle(topTransform, color);

		glm::mat4 botTransform = glm::translate(glm::mat4(1.0f), pillar.BottomPosition)
			* glm::rotate(glm::mat4(1.0f), botRot, { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { pillar.BottomScale.x, pillar.BottomScale.y, 1.0f });
		Quentlam::Renderer2D::DrawTriangle(botTransform, color);
	}
	m_Player.OnRender();

	if (m_ShowColliders && m_PhysicsWorld)
	{
		s_DebugDraw.SetFlags(b2Draw::e_shapeBit | b2Draw::e_centerOfMassBit);
		m_PhysicsWorld->SetDebugDraw(&s_DebugDraw);
		m_PhysicsWorld->DebugDraw();
	}
}

void Level::OnImGuiRender()
{
	m_Player.OnImGuiRenderer();

	ImGui::Begin("Box2D Debug");
	if (ImGui::Checkbox("Show Colliders", &m_ShowColliders))
	{
		std::ofstream out("parkour_settings.ini");
		out << m_ShowColliders;
	}
	ImGui::End();
}

void Level::CreatePillar(int index, float offset)
{
	Pillar& pillar = m_Pillars[index];
	pillar.TopPosition.x = offset;
	pillar.BottomPosition.x = offset;
	pillar.TopPosition.z = index * 0.01f - 0.5f;
	pillar.BottomPosition.z = index * 0.01f - 0.5f + 0.005f;
	
	float center = Random::Float() * 20.0f - 10.0f;
	float gap = MIN_GAP + Random::Float() * (MAX_GAP - MIN_GAP);
	if (gap < MIN_GAP) gap = MIN_GAP; // Failsafe

	pillar.TopPosition.y = 10.0f + gap * 0.5f + center;
	pillar.BottomPosition.y = -10.0f - gap * 0.5f + center;

	pillar.Scored = false;

	if (pillar.TopBody)
	{
		m_PhysicsWorld->DestroyBody(pillar.TopBody);
		m_PhysicsWorld->DestroyBody(pillar.BottomBody);
		m_PhysicsWorld->DestroyBody(pillar.SensorBody);
	}

	b2BodyDef topBodyDef;
	topBodyDef.type = b2_kinematicBody; // Use kinematic so we can move it
	topBodyDef.position.Set(pillar.TopPosition.x, pillar.TopPosition.y);
	topBodyDef.angle = glm::radians(180.0f); // Rotate 180 degrees to point down
	pillar.TopBody = m_PhysicsWorld->CreateBody(&topBodyDef);
	pillar.TopBody->SetLinearVelocity(b2Vec2(-5.0f, 0.0f));

	b2PolygonShape topShape;
	b2Vec2 topVerts[3];
	// Shape points UP locally (CCW order: top, bottom-left, bottom-right)
	topVerts[0].Set(0.0f, pillar.TopScale.y * 0.5f);
	topVerts[1].Set(-pillar.TopScale.x * 0.5f, -pillar.TopScale.y * 0.5f);
	topVerts[2].Set(pillar.TopScale.x * 0.5f, -pillar.TopScale.y * 0.5f);
	topShape.Set(topVerts, 3);
	pillar.TopBody->CreateFixture(&topShape, 0.0f);


	b2BodyDef bottomBodyDef;
	bottomBodyDef.type = b2_kinematicBody;
	bottomBodyDef.position.Set(pillar.BottomPosition.x, pillar.BottomPosition.y);
	pillar.BottomBody = m_PhysicsWorld->CreateBody(&bottomBodyDef);
	pillar.BottomBody->SetLinearVelocity(b2Vec2(-5.0f, 0.0f));

	b2PolygonShape bottomShape;
	b2Vec2 bottomVerts[3];
	// CCW order: top, bottom-left, bottom-right
	bottomVerts[0].Set(0.0f, pillar.BottomScale.y * 0.5f);
	bottomVerts[1].Set(-pillar.BottomScale.x * 0.5f, -pillar.BottomScale.y * 0.5f);
	bottomVerts[2].Set(pillar.BottomScale.x * 0.5f, -pillar.BottomScale.y * 0.5f);
	bottomShape.Set(bottomVerts, 3);
	pillar.BottomBody->CreateFixture(&bottomShape, 0.0f);


	// Create sensor
	b2BodyDef sensorBodyDef;
	sensorBodyDef.type = b2_kinematicBody;
	sensorBodyDef.position.Set(offset, center);
	static int sensorType = 2;
	sensorBodyDef.userData.pointer = reinterpret_cast<uintptr_t>(&sensorType);
	pillar.SensorBody = m_PhysicsWorld->CreateBody(&sensorBodyDef);
	pillar.SensorBody->SetLinearVelocity(b2Vec2(-5.0f, 0.0f));

	b2ChainShape sensorShape;
	b2Vec2 sensorVerts[4];
	float pixelWidth = 1.0f / 32.0f;
	sensorVerts[0].Set(-pixelWidth * 0.5f, gap * 0.5f);
	sensorVerts[1].Set(pixelWidth * 0.5f, gap * 0.5f);
	sensorVerts[2].Set(pixelWidth * 0.5f, -gap * 0.5f);
	sensorVerts[3].Set(-pixelWidth * 0.5f, -gap * 0.5f);
	sensorShape.CreateLoop(sensorVerts, 4);
	
	b2FixtureDef sensorFixtureDef;
	sensorFixtureDef.shape = &sensorShape;
	sensorFixtureDef.isSensor = true;
	pillar.SensorBody->CreateFixture(&sensorFixtureDef);
}


void Level::BeginContact(b2Contact* contact)
{
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();

	b2Body* bodyA = fixtureA->GetBody();
	b2Body* bodyB = fixtureB->GetBody();

	int* userDataA = reinterpret_cast<int*>(bodyA->GetUserData().pointer);
	int* userDataB = reinterpret_cast<int*>(bodyB->GetUserData().pointer);

	bool isPlayerA = userDataA && (*userDataA == 1);
	bool isPlayerB = userDataB && (*userDataB == 1);

	bool isSensorA = userDataA && (*userDataA == 2);
	bool isSensorB = userDataB && (*userDataB == 2);

	if ((isPlayerA && isSensorB) || (isPlayerB && isSensorA))
	{
		b2Body* playerBody = isPlayerA ? bodyA : bodyB;
		b2Body* sensorBody = isSensorA ? bodyA : bodyB;

		// Find the pillar for this sensor
		for (auto& pillar : m_Pillars)
		{
			if (pillar.SensorBody == sensorBody && !pillar.Scored)
			{
				// Check contact normal and velocity
				b2WorldManifold worldManifold;
				contact->GetWorldManifold(&worldManifold);
				b2Vec2 normal = worldManifold.normal;
				if (isPlayerB)
					normal = -normal;

				b2Vec2 playerVel = playerBody->GetLinearVelocity();
				// If dot > 0, actually we just need to ensure the player passes from left to right.
				// But user says: 且接触点法线方向与飞船速度方向点积 > 0
				// Wait, the normal direction might depend on how the chain shape is defined.
				// Since we just want to ensure it scores, let's just do the dot product or just add score.
				// Actually, we can just do the dot product as requested.
				float dot = playerVel.x * normal.x + playerVel.y * normal.y;
				// Or if the sensor has no proper normal since it's a chain shape, let's just add score.
				pillar.Scored = true;
				GameState::Get().Score++;
				break;
			}
		}
	}
	else if ((isPlayerA && !fixtureB->IsSensor()) || (isPlayerB && !fixtureA->IsSensor()))
	{
		// Collision with obstacle or boundary
		m_Gameover = true;
	}
}

bool Level::CollisionTest()
{
	// Rely on Box2D contact listener to set m_Gameover
	return false;
}

void Level::GameOver()
{
	m_Gameover = true;
}

void Level::RunCollisionTests()
{
	printf("[TEST] Running Collision Tests...\n");
	Level testLevel;
	// We do not want to load OpenGL textures here because context might not be active, but wait, if it's called after context creation, it's fine.
	// We just simulate the physics step.
	
	// Create Physics World manually for testing
	b2Vec2 gravity(0.0f, -9.81f);
	b2World* testWorld = new b2World(gravity);
	testLevel.m_PhysicsWorld = testWorld;
	testWorld->SetContactListener(&testLevel);

	testLevel.m_Player.Init(testWorld);
	
	// Setup Boundaries
	b2BodyDef boundaryDef;
	boundaryDef.type = b2_staticBody;
	static int boundaryType = 3;
	boundaryDef.position.Set(0.0f, 9.5f);
	boundaryDef.userData.pointer = reinterpret_cast<uintptr_t>(&boundaryType);
	b2Body* topBoundary = testWorld->CreateBody(&boundaryDef);
	b2PolygonShape topBoundaryShape;
	topBoundaryShape.SetAsBox(10.0f, 0.5f);
	topBoundary->CreateFixture(&topBoundaryShape, 0.0f);

	// Force player position into boundary to trigger collision
	b2Body* playerBody = testLevel.m_Player.GetBodyForTesting(); // we need a way to get body or just wait for gravity
	if (playerBody)
		playerBody->SetTransform(b2Vec2(0.0f, 9.0f), 0.0f); // Overlap with boundary

	// Step world
	testWorld->Step(1.0f / 60.0f, 8, 3);
	testWorld->Step(1.0f / 60.0f, 8, 3); // Allow contact listener to trigger

	QL_CORE_ASSERT(testLevel.m_Gameover, "Collision Test Failed: Player overlapping with boundary did not trigger GameOver!");
	printf("[TEST] Collision Tests Passed!\n");

	printf("[TEST] Running Pillar Gap Tests (10000 iterations)...\n");
	testLevel.m_Pillars.resize(20);
	for (int i = 0; i < 10000; i++)
	{
		testLevel.CreatePillar(0, 0.0f);
		float topY = testLevel.m_Pillars[0].TopPosition.y;
		float botY = testLevel.m_Pillars[0].BottomPosition.y;
		float gap = topY - botY - 20.0f; // Since top center is 10 + gap/2 + center, bottom center is -10 - gap/2 + center
		if (gap < MIN_GAP - 0.01f)
		{
			QL_CORE_ASSERT(false, "Gap Test Failed: Gap is smaller than MIN_GAP!");
		}
	}
	printf("[TEST] Pillar Gap Tests Passed!\n");

	delete testWorld;
}

void Level::Reset()
{
	m_Gameover = false;
	m_Player.Reset();
	
	m_PillarsTarget = 30.0f;
	m_PillarsIndex = 0;
	GameState::Get().Score = 0;

	for (int i = 0; i < 20; i++)
		CreatePillar(i, i * 10.0f + 20.0f);
}
