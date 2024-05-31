#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>
#include <iostream>

class Game {
public:
	Game(int ancho, int alto, std::string titulo) {
		wnd = new sf::RenderWindow(sf::VideoMode(ancho, alto), titulo);
		wnd->setFramerateLimit(60);
		frameTime = 1.0f / 60.0f;
		InitPhysics();
		SetZoom();
	}

	~Game() {
		delete wnd;
		delete phyWorld;
	}

	void Loop() {
		while (wnd->isOpen()) {
			sf::Event evt;
			while (wnd->pollEvent(evt)) {
				if (evt.type == sf::Event::Closed)
					wnd->close();
			}

			UpdatePhysics();
			wnd->clear(sf::Color::Black);
			DrawGame3();
			wnd->display();
		}
	}

private:
	sf::RenderWindow* wnd;
	b2World* phyWorld;
	b2Body* groundBody;
	b2Body* fallingBody;
	sf::View camara;

	float frameTime;

	void InitPhysics() {
		b2Vec2 gravity(0.0f, 9.8f);
		phyWorld = new b2World(gravity);

		// Ground body
		b2BodyDef groundBodyDef;
		groundBodyDef.position.Set(400.0f / 30.0f, 550.0f / 30.0f);
		groundBody = phyWorld->CreateBody(&groundBodyDef);
		b2PolygonShape groundBox;
		groundBox.SetAsBox(400.0f / 30.0f, 10.0f / 30.0f);
		groundBody->CreateFixture(&groundBox, 0.0f);

		// Falling body
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(400.0f / 30.0f, 300.0f / 30.0f);
		fallingBody = phyWorld->CreateBody(&bodyDef);
		b2PolygonShape dynamicBox;
		dynamicBox.SetAsBox(30.0f / 30.0f, 30.0f / 30.0f);
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &dynamicBox;
		fixtureDef.density = 1.0f;
		fixtureDef.friction = 0.3f;
		fallingBody->CreateFixture(&fixtureDef);
	}

	void UpdatePhysics() {
		phyWorld->Step(frameTime, 8, 3);
	}

	void DrawGame3() {
		sf::RectangleShape groundShape(sf::Vector2f(800.0f, 20.0f));
		groundShape.setFillColor(sf::Color::Green);
		groundShape.setPosition(0.0f, 550.0f);
		wnd->draw(groundShape);

		sf::RectangleShape blockShape(sf::Vector2f(60.0f, 60.0f));
		blockShape.setFillColor(sf::Color::Red);
		b2Vec2 position = fallingBody->GetPosition();
		blockShape.setPosition(position.x * 30.0f - 30.0f, position.y * 30.0f - 30.0f);
		wnd->draw(blockShape);
	}

	void SetZoom() {
		camara.setSize(800.0f, 600.0f);
		camara.setCenter(400.0f, 300.0f);
		wnd->setView(camara);
	}
};

int mainasd() {
	Game game(800, 600, "Box2D and SFML Simulation");
	game.Loop();
	return 0;
}
