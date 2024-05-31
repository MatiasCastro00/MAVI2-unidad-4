#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>
#include <cmath>
#include <vector>

const float SCALE = 30.f;
const float DEGTORAD = 0.0174532925199432957f;

class Game {
public:
	Game(int ancho, int alto, std::string titulo) {
		wnd = new sf::RenderWindow(sf::VideoMode(ancho, alto), titulo);
		wnd->setFramerateLimit(60);
		frameTime = 1.0f / 60.0f;
		cannonAngle = 45.0f;
		InitPhysics();
		SetZoom();
		CreateGround();
		CreateAnchoredRectangle();
		CreateStaticSquare();
		CreateBorders();
	}

	~Game() {
		delete wnd;
		delete phyWorld;
	}

	void Loop() {
		while (wnd->isOpen()) {
			sf::Event evt;
			while (wnd->pollEvent(evt)) {
				switch (evt.type) {
				case sf::Event::Closed:
					wnd->close();
					break;
				case sf::Event::MouseMoved:
					UpdateCannonAngle(evt.mouseMove.x, evt.mouseMove.y);
					break;
				case sf::Event::MouseButtonPressed:
					if (evt.mouseButton.button == sf::Mouse::Left) {
						CreateRagdoll();
						Fire(evt.mouseButton.x, evt.mouseButton.y);
					}
					break;
				}
			}

			UpdatePhysics();
			wnd->clear(sf::Color::Black);
			DrawGame();
			wnd->display();
		}
	}

private:
	sf::RenderWindow* wnd;
	b2World* phyWorld;
	float frameTime;
	float cannonAngle;
	std::vector<std::vector<b2Body*>> ragdolls;
	b2Body* anchoredRect;
	b2Body* staticSquare;

	void InitPhysics() {
		b2Vec2 gravity(0.0f, 9.8f);
		phyWorld = new b2World(gravity);
	}

	b2Body* createBox(float x, float y, float width, float height, bool dynamic, float density, float friction) {
		b2BodyDef bodyDef;
		bodyDef.position = b2Vec2(x / SCALE, y / SCALE);
		bodyDef.type = dynamic ? b2_dynamicBody : b2_staticBody;
		b2Body* body = phyWorld->CreateBody(&bodyDef);

		b2PolygonShape shape;
		shape.SetAsBox((width / 2) / SCALE, (height / 2) / SCALE);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;
		fixtureDef.density = density;
		fixtureDef.friction = friction;
		body->CreateFixture(&fixtureDef);

		return body;
	}

	b2Body* createCircle(float x, float y, float radius, bool dynamic, float density, float friction) {
		b2BodyDef bodyDef;
		bodyDef.position = b2Vec2(x / SCALE, y / SCALE);
		bodyDef.type = dynamic ? b2_dynamicBody : b2_staticBody;
		b2Body* body = phyWorld->CreateBody(&bodyDef);

		b2CircleShape shape;
		shape.m_radius = radius / SCALE;

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;
		fixtureDef.density = density;
		fixtureDef.friction = friction;
		body->CreateFixture(&fixtureDef);

		return body;
	}

	b2RevoluteJoint* createRevoluteJoint(b2Body* bodyA, b2Body* bodyB, const b2Vec2& anchor) {
		b2RevoluteJointDef jointDef;
		jointDef.Initialize(bodyA, bodyB, anchor);
		return static_cast<b2RevoluteJoint*>(phyWorld->CreateJoint(&jointDef));
	}

	b2RevoluteJoint* connectBodies(b2Body* bodyA, b2Body* bodyB, const b2Vec2& anchor, float lowerAngle, float upperAngle) {
		b2RevoluteJointDef jointDef;
		jointDef.Initialize(bodyA, bodyB, bodyA->GetWorldPoint(anchor));
		jointDef.lowerAngle = lowerAngle * DEGTORAD;
		jointDef.upperAngle = upperAngle * DEGTORAD;
		jointDef.enableLimit = true;

		return static_cast<b2RevoluteJoint*>(phyWorld->CreateJoint(&jointDef));
	}

	void CreateGround() {
		createBox(400, 580, 800, 40, false, 0.f, 0.3f);
	}

	void CreateAnchoredRectangle() {
		anchoredRect = createBox(650, 250, 200, 10, true, 1.f, 0.3f);

		b2BodyDef anchorDef;
		anchorDef.position.Set(550 / SCALE, 250 / SCALE);
		b2Body* anchor = phyWorld->CreateBody(&anchorDef);

		createRevoluteJoint(anchor, anchoredRect, b2Vec2(550 / SCALE, 250 / SCALE));
	}

	void CreateStaticSquare() {
		staticSquare = createBox(750, 500, 50, 50, false, 1.f, 0.3f);
	}

	void CreateBorders() {
		createBox(400, 0, 800, 40, false, 0.f, 0.3f); // Top
		createBox(0, 300, 40, 600, false, 0.f, 0.3f); // Left
		createBox(800, 300, 40, 600, false, 0.f, 0.3f); // Right
	}

	void CreateRagdoll() {
		std::vector<b2Body*> parts;

		b2Body* head = createCircle(50, 50, 15, true, 1.f, 0.3f);
		b2Body* torso = createBox(50, 100, 30, 50, true, 1.f, 0.3f);
		b2Body* leftArm = createBox(30, 100, 10, 50, true, 1.f, 0.3f);
		b2Body* rightArm = createBox(70, 100, 10, 50, true, 1.f, 0.3f);
		b2Body* leftLeg = createBox(45, 150, 10, 50, true, 1.f, 0.3f);
		b2Body* rightLeg = createBox(55, 150, 10, 50, true, 1.f, 0.3f);

		connectBodies(torso, head, b2Vec2(0.f, -1.5f), -30.f, 30.f);
		connectBodies(torso, leftArm, b2Vec2(-0.75f, -1.f), -90.f, 90.f);
		connectBodies(torso, rightArm, b2Vec2(0.75f, -1.f), -90.f, 90.f);
		connectBodies(torso, leftLeg, b2Vec2(-0.4f, 2.f), -45.f, 45.f);
		connectBodies(torso, rightLeg, b2Vec2(0.4f, 2.f), -45.f, 45.f);

		parts.push_back(head);
		parts.push_back(torso);
		parts.push_back(leftArm);
		parts.push_back(rightArm);
		parts.push_back(leftLeg);
		parts.push_back(rightLeg);

		ragdolls.push_back(parts);
	}

	void UpdateCannonAngle(int mouseX, int mouseY) {
		sf::Vector2f cannonPos = { 30.0f, 300.0f };
		sf::Vector2i mousePos(mouseX, mouseY);
		float dx = mousePos.x - cannonPos.x;
		float dy = mousePos.y - cannonPos.y;
		cannonAngle = std::atan2(dy, dx) * 180 / b2_pi;
	}

	void Fire(int mouseX, int mouseY) {
		float radians = cannonAngle * DEGTORAD;
		float forceMagnitude = CalculateForce(mouseX, mouseY);
		b2Vec2 force(std::cos(radians) * forceMagnitude, std::sin(radians) * forceMagnitude);

		auto& parts = ragdolls.back();
		for (auto& part : parts) {
			part->SetTransform(b2Vec2(50.0f / SCALE, 300.0f / SCALE), 0);
			part->SetLinearVelocity(b2Vec2(0, 0));
			part->ApplyLinearImpulseToCenter(force, true);
		}
	}

	float CalculateForce(int mouseX, int mouseY) {
		sf::Vector2f cannonPos = { 30.0f, 300.0f };
		sf::Vector2f mousePos = { static_cast<float>(mouseX), static_cast<float>(mouseY) };
		float distance = std::hypot(mousePos.x - cannonPos.x, mousePos.y - cannonPos.y);
		float minForce = 5.0f;
		float maxForce = 20.0f;
		return std::min(maxForce, std::max(minForce, distance / 30.0f));
	}

	void UpdatePhysics() {
		phyWorld->Step(frameTime, 8, 3);
	}

	void DrawGame() {
		sf::RectangleShape cannon(sf::Vector2f(100.0f, 20.0f));
		cannon.setFillColor(sf::Color::White);
		cannon.setPosition(30.0f, 300.0f);
		cannon.setOrigin(0.0f, 10.0f);
		cannon.setRotation(cannonAngle);
		wnd->draw(cannon);

		sf::RectangleShape ground(sf::Vector2f(800.f, 40.f));
		ground.setFillColor(sf::Color::Green);
		ground.setOrigin(400.f, 20.f);
		ground.setPosition(400.f, 580.f);
		wnd->draw(ground);

		sf::RectangleShape anchoredRectShape(sf::Vector2f(200.f, 10.f));
		anchoredRectShape.setFillColor(sf::Color::Magenta);
		anchoredRectShape.setOrigin(0.f, 5.f);
		anchoredRectShape.setPosition(550.f, 250.f);
		anchoredRectShape.setRotation(anchoredRect->GetAngle() * 180 / b2_pi);
		wnd->draw(anchoredRectShape);

		sf::RectangleShape staticSquareShape(sf::Vector2f(50.f, 50.f));
		staticSquareShape.setFillColor(sf::Color::Blue);
		staticSquareShape.setOrigin(25.f, 25.f);
		staticSquareShape.setPosition(staticSquare->GetPosition().x * SCALE, staticSquare->GetPosition().y * SCALE);
		wnd->draw(staticSquareShape);

		auto drawBody = [&](b2Body* body, float width, float height, sf::Color color) {
			sf::RectangleShape shape(sf::Vector2f(width, height));
			shape.setOrigin(width / 2, height / 2);
			shape.setPosition(body->GetPosition().x * SCALE, body->GetPosition().y * SCALE);
			shape.setRotation(body->GetAngle() * 180 / b2_pi);
			shape.setFillColor(color);
			wnd->draw(shape);
			};

		for (auto& ragdoll : ragdolls) {
			for (auto& part : ragdoll) {
				if (part->GetFixtureList()->GetShape()->GetType() == b2Shape::e_circle) {
					sf::CircleShape shape(15.f);
					shape.setOrigin(15.f, 15.f);
					shape.setPosition(part->GetPosition().x * SCALE, part->GetPosition().y * SCALE);
					shape.setRotation(part->GetAngle() * 180 / b2_pi);
					shape.setFillColor(sf::Color::Blue);
					wnd->draw(shape);
				}
				else {
					b2PolygonShape* polygonShape = (b2PolygonShape*)part->GetFixtureList()->GetShape();
					float width = (polygonShape->m_vertices[2].x - polygonShape->m_vertices[0].x) * SCALE;
					float height = (polygonShape->m_vertices[2].y - polygonShape->m_vertices[0].y) * SCALE;

					if (width == 10.f && height == 50.f) {
						drawBody(part, width, height, sf::Color::Yellow);
					}
					else {
						drawBody(part, width, height, sf::Color::Red);
					}
				}
			}
		}
	}

	void SetZoom() {
		sf::View camara(sf::FloatRect(0, 0, 800, 600));
		wnd->setView(camara);
	}
};

int main() {
	Game game(800, 600, "Ragdoll Cannon");
	game.Loop();
	return 0;
}
