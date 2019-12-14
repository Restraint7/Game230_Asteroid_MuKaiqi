#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Main.hpp>

#include <random>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <list>
#include <time.h>

using namespace std;

const float PI = atan(1) * 4;

const int MAXOBJECTINONEGRID = 10;
const int DEPTHOFQUADTREE = 3;
const float BASICSPEEDOFASTEROID = 50;
const float SPEEDOFTHEBULLET = 600;
const float ACCELERATIONOFSPACESHIPPERSECOND = 200;
const float ACCELERATIONOFRESISTANCEPERSECOND = 20;
const float RADIUSOFSPACESHIP = 20;
const float LENGTHOFTHEWINDOW = 800.f;
const float HEIGHTHTOFTHEWINDOW = LENGTHOFTHEWINDOW * 0.75;
const float GAMEAREALENGTH = LENGTHOFTHEWINDOW + 400;
const float GAMEAREAHEIGHT = HEIGHTHTOFTHEWINDOW + 400;
const float WIDTHOFTHEINTERVAL = 20;
const float RADIUSOFBULLET = 4;
const int FONTSIZEOFSCOREBOARD = LENGTHOFTHEWINDOW / 16;
const int FONTSIZEOFPOWERUP = LENGTHOFTHEWINDOW / 50;
const int NUMBEROFGRIDFORGENERATECIRCLEONROW = 8;
const int NUMBEROFGRIDFORGENERATECIRCLEONCOLUNM = 6;


enum class CircleObjectType{Spacecraft,Asteroid,Bullet,PowerUp};
enum class AsteroidType{Large,Medium,Small};
enum class CampType{Friendly,Neutral,Hostile};
enum class ObjectRelationshipType {Contain,IntersectVertically,IntersectHorizontically,IntersectTwoLinesNoCornor,IntersectAndContainCornor,Separated};
enum class AlignOfText { leftAlign, center, rightAlign };
enum class PowerUpType { Shield, ThreeShot, RapidShot, Invincible };
enum class GameScene{MainMenu,GameStart,LevelClear,GameOver};
sf::Texture textureOfSpacecraft;
sf::Texture textureOfExplosion;
sf::Texture textureOfPushingFire;
sf::Texture textureOfShield;
sf::Texture textureOfThreeShot;
sf::Texture textureOfRapidShot;
sf::Texture textureOfInvincible;
sf::Texture textureOfEnemy;
sf::Texture textureOfAsteroid;
sf::Sound soundOfShoot;
sf::SoundBuffer bufferOfShoot;
sf::Sound soundOfTheBallHitTheBound;
sf::SoundBuffer bufferOfTheBallHitTheBound;
sf::Sound soundOfTheBallHitTheBrick;
sf::SoundBuffer bufferOfTheBallHitTheBrick;
sf::Sound soundOfAsteroidBroken;
sf::SoundBuffer bufferOfAsteroidBroken;
sf::Sound soundOfLevelClear;
sf::SoundBuffer bufferOfLevelClear;
sf::Sound soundOfspacecraftBroken;
sf::SoundBuffer bufferOfspacecraftBroken;
sf::Sound soundOfThrust;
sf::SoundBuffer bufferOfThrust;


int sequenceNumber = 0;

class Explosion :public sf::CircleShape {
public:
	sf::Clock clockOfLife;
	Explosion(sf::Vector2f position) {
		setRadius(RADIUSOFSPACESHIP / 2);
		setOrigin(RADIUSOFSPACESHIP / 2, RADIUSOFSPACESHIP / 2);
		setPosition(position);
		setTexture(&textureOfExplosion);
		clockOfLife.restart();
	}
	void drawSelf(sf::RenderWindow& window) {
		setRadius(RADIUSOFSPACESHIP / 2 + clockOfLife.getElapsedTime().asSeconds() * RADIUSOFSPACESHIP * 3);
		setOrigin(getRadius(), getRadius());
		window.draw(*this);
	}
	~Explosion() {

	}
};

class TextForAsteroid :public sf::Text {
public:
	sf::Font font;
	TextForAsteroid() {};
	TextForAsteroid(const sf::String& contentOfTheText, int fontSizeOfTheText, sf::Color colorOfTheText, sf::Vector2f positionOfTheText, AlignOfText alignMethod) {
		initateTextForPong(contentOfTheText, fontSizeOfTheText, colorOfTheText, positionOfTheText, alignMethod);
	}
	void initateTextForPong(const sf::String& contentOfTheText, int fontSizeOfTheText, sf::Color colorOfTheText, sf::Vector2f positionOfTheText, AlignOfText alignMethod) {
		font.loadFromFile("arial.ttf");
		setFont(font);
		setString(contentOfTheText);
		setCharacterSize(fontSizeOfTheText);
		setFillColor(colorOfTheText);
		sf::FloatRect borderOfTheText = getLocalBounds();
		switch (alignMethod) {
		case AlignOfText::leftAlign:setPosition(positionOfTheText); break;
		case AlignOfText::center:setOrigin(sf::Vector2f(borderOfTheText.width / 2, 0)); setPosition(positionOfTheText); break;
		case AlignOfText::rightAlign:setOrigin(sf::Vector2f(borderOfTheText.width, 0)); setPosition(positionOfTheText); break;
		}
	}
};

class CircleObject :public sf::CircleShape {
public:
	CircleObject* lastHitObject = nullptr;
	sf::Vector2f centralPoint;
	PowerUpType powerUpType;
	sf::Vector2f* velocity = new sf::Vector2f(0,0);
	float* speedPerSecond = new float(0);
	float* moveAngle = new float(0);
	int orderNumber = 0;
	bool splitByVerticalLine = false;
	bool splitByHorizonticalLine = false;
	bool touchVerticalBound = false;
	bool touchHorizonticalBound = false;
	bool* destroyed = new bool(false);
	bool shouldBeRemoved = false;
	int frameNumber = 0;
	int containedByHowManyGrid = 0;
	CampType camp;
	CircleObjectType circleObjectType;

	virtual void drawself(sf::RenderWindow& window) {
		window.draw(*this);
	}

	virtual bool collidedWith(CircleObject* object) {
		return false;
	}
	
	virtual void moveByTime(float timePerFrame) {
		velocity->x = sin(*moveAngle / 180 * PI) * *speedPerSecond;
		velocity->y = -cos(*moveAngle / 180 * PI) * *speedPerSecond;
		move(*velocity * timePerFrame);
	}

	virtual CircleObject* clone() {
		CircleObject* newObject = new CircleObject();
		return newObject;
	}

	virtual vector<sf::Vector3f> shootPoisition(){
		vector<sf::Vector3f> bulletPosition;
		return bulletPosition;
	}

	virtual void breakUp(vector<CircleObject*>& waitListOfCrossingObject) {

	}

	virtual void controlByPlayer(bool left, bool right, bool front, bool back, float timePerFrame, sf::RenderWindow& window) {

	}
	virtual ~CircleObject() {
	}
};

class Spacecraft:public CircleObject {
public:
	int timeCountForInvincible = 0;
	bool shielded = false;
	bool invincible = false;
	sf::Clock clockForInvincible;
	bool threeShot = false;
	bool rapidShot = false;
	Spacecraft(sf::Vector2f centerPosition,CampType campType) {
		initializeSpacecraft(centerPosition,0,0,campType);
	}
	Spacecraft(sf::Vector2f centerPosition,float speed, float direction, CampType campType) {
		initializeSpacecraft(centerPosition,speed,direction,campType);
	}
	void initializeSpacecraft(sf::Vector2f centerPosition,float speed, float direction,CampType campType) {
		
		orderNumber = sequenceNumber++;
		camp = campType;
		circleObjectType = CircleObjectType::Spacecraft;
		*speedPerSecond = speed;
		*moveAngle = direction;
		velocity->x = sin(*moveAngle / 180 * PI);
		velocity->y = cos(*moveAngle / 180 * PI);
		centralPoint = centerPosition;
		setRadius(RADIUSOFSPACESHIP);
		setOrigin(RADIUSOFSPACESHIP, RADIUSOFSPACESHIP);
		setPosition(centralPoint);
		setRotation(*moveAngle);
		setOutlineThickness(2);
		if (camp == CampType::Friendly) {
			setOutlineColor(sf::Color::White);
			setTexture(&textureOfSpacecraft);
			invincible = true;
		}
		else {
			setOutlineColor(sf::Color::Red);
			setTexture(&textureOfEnemy);
		}
		clockForInvincible.restart();
	}

	void controlByPlayer(bool left, bool right, bool front, bool back, float timePerFrame,sf::RenderWindow& window) {
		if (left) {
			*moveAngle -= 180 * timePerFrame;
			*moveAngle < 0 ? *moveAngle += 360 : NULL;
			setRotation(*moveAngle);
		}
		if (right) {
			*moveAngle += 180 * timePerFrame;
			*moveAngle > 360 ? *moveAngle -= 360 : NULL;
			setRotation(*moveAngle);
		}
		if (front) {
			soundOfThrust.play();
			*speedPerSecond += ACCELERATIONOFSPACESHIPPERSECOND * timePerFrame;
			sf::CircleShape pushFire;
			pushFire.setRadius(RADIUSOFSPACESHIP / 2);
			pushFire.setOrigin(RADIUSOFSPACESHIP / 2, RADIUSOFSPACESHIP / 2);
			pushFire.setRotation(*moveAngle + 180);
			pushFire.setPosition(getPosition().x - getRadius() * 3 / 2 * sin(*moveAngle / 180 * PI), getPosition().y + getRadius() * 3 / 2 * cos(*moveAngle / 180 * PI));
			pushFire.setTexture(&textureOfPushingFire);
			window.draw(pushFire);
		}
		if (back) {
			*speedPerSecond > 0 ? *speedPerSecond -= 2*ACCELERATIONOFSPACESHIPPERSECOND * timePerFrame : *speedPerSecond = 0;			
		}
	}
	void moveByTime(float timePerFrame) {
		if (camp != CampType::Hostile) {
			*speedPerSecond > 0 ? *speedPerSecond -= ACCELERATIONOFRESISTANCEPERSECOND * timePerFrame : *speedPerSecond = 0;
		}
		velocity->x = sin(*moveAngle / 180 * PI) * *speedPerSecond;
		velocity->y = -cos(*moveAngle / 180 * PI) * *speedPerSecond;
		move(*velocity * timePerFrame);
	}
	bool collidedWith(CircleObject* object){
		if (object->circleObjectType == CircleObjectType::Spacecraft) {
			if (object->camp == camp) {
				*destroyed = false;
			}
			else {
				if (invincible) {
					*destroyed = false;
				}
				else if(shielded){
					*destroyed = false;
					shielded = false;
				}
				else {
					*destroyed = true;
				}
			}
		}
		if (object->circleObjectType == CircleObjectType::Asteroid) {
			if (invincible) {
				*destroyed = false;
			}
			else if (shielded) {
				*destroyed = false;
				shielded = false;
			}
			else {
				*destroyed = true;
			}
		}
		if (object->circleObjectType == CircleObjectType::Bullet) {
			*destroyed = object->camp == camp ? false : true;
		}
		if (object->circleObjectType == CircleObjectType::PowerUp) {
			*destroyed = false;
			switch (object->powerUpType)
			{
			case PowerUpType::Invincible:
				invincible = true;
				clockForInvincible.restart();
				break;
			case PowerUpType::RapidShot:
				rapidShot = true;
				break;
			case PowerUpType::Shield:
				shielded = true;
				break;
			case PowerUpType::ThreeShot:
				threeShot = true;
				break;
			default:
				break;
			}
		}
		return *destroyed;
	}
	
	vector<sf::Vector3f> shootPoisition() {
		soundOfShoot.play();
		vector<sf::Vector3f> bulletPosition;
		bulletPosition.push_back(sf::Vector3f(getPosition().x,getPosition().y,*moveAngle));
		if (rapidShot) {
			bulletPosition.push_back(sf::Vector3f(getPosition().x + getRadius() * sin(*moveAngle / 180 * PI), getPosition().y - getRadius() * cos(*moveAngle / 180 * PI), *moveAngle));
			bulletPosition.push_back(sf::Vector3f(getPosition().x + getRadius() * 2 * sin(*moveAngle / 180 * PI), getPosition().y - getRadius() * 2 * cos(*moveAngle / 180 * PI), *moveAngle));
		}
		if (threeShot) {
			bulletPosition.push_back(sf::Vector3f(getPosition().x, getPosition().y, *moveAngle + 30 > 360 ? *moveAngle - 330 : *moveAngle + 30));
			bulletPosition.push_back(sf::Vector3f(getPosition().x, getPosition().y, *moveAngle - 30 < 0 ? *moveAngle + 330 : *moveAngle - 30));
		}
		return bulletPosition;
	}

	void drawself(sf::RenderWindow& window) {
		if (shielded) {
			sf::CircleShape shield;
			shield.setOutlineThickness(2);
			shield.setRadius(getRadius() * 1.2);
			shield.setOrigin(getRadius() * 1.2, getRadius() * 1.2);
			shield.setFillColor(sf::Color::Transparent);
			shield.setOutlineColor(sf::Color::White);
			shield.setPosition(getPosition());
			window.draw(shield);
		}
		if (invincible) {
			if (clockForInvincible.getElapsedTime().asSeconds() < 0.3) {
				window.draw(*this);
			}
			else if (clockForInvincible.getElapsedTime().asSeconds() > 0.4) {
				clockForInvincible.restart();
				timeCountForInvincible += 1;
				if (timeCountForInvincible >= 10) {
					invincible = false;
					timeCountForInvincible = 0;
				}
			}
		}
		else {
			window.draw(*this);
		}
	}

	Spacecraft* clone() {
		sf::Vector2f position = getPosition();
		if (position.x < getRadius()) {
			position.x += LENGTHOFTHEWINDOW;
		}
		else if (position.x > LENGTHOFTHEWINDOW - getRadius()) {
			position.x -= LENGTHOFTHEWINDOW;
		}
		if (position.y < getRadius()) {
			position.y += HEIGHTHTOFTHEWINDOW;
		}
		else if (position.y > HEIGHTHTOFTHEWINDOW - getRadius()) {
			position.y -= HEIGHTHTOFTHEWINDOW;
		}
		Spacecraft* newObject = new Spacecraft(position,*speedPerSecond,*moveAngle,camp);
		newObject->splitByHorizonticalLine = splitByHorizonticalLine;
		newObject->splitByVerticalLine = splitByVerticalLine;
		newObject->touchHorizonticalBound = touchHorizonticalBound;
		newObject->touchVerticalBound = touchVerticalBound;
		newObject->containedByHowManyGrid = 0;
		newObject->velocity = velocity;
		newObject->destroyed = destroyed;
		newObject->moveAngle = moveAngle;
		newObject->invincible = invincible;
		newObject->shielded = shielded;
		newObject->timeCountForInvincible = timeCountForInvincible;
		return newObject;
	}

	~Spacecraft() {
	}
};

class Asteroid :public CircleObject {
public:
	AsteroidType asteroidType = AsteroidType::Small;
	Asteroid(sf::Vector2f centerPosition, float speed, float direction, float radius) {
		initializeAsteroid(centerPosition,speed, direction,radius);
	}
	void initializeAsteroid(sf::Vector2f centerPosition, float speed, float direction , float radius) {
		orderNumber = sequenceNumber++;
		camp = CampType::Neutral;
		circleObjectType = CircleObjectType::Asteroid;
		*speedPerSecond = speed;
		*moveAngle = direction;
		centralPoint = centerPosition;
		setRadius(radius);
		setOrigin(radius, radius);
		setFillColor(sf::Color::White);
		setTexture(&textureOfAsteroid);	
		float radiusOfAsteroid;
		if (radius > 60) {
			asteroidType = AsteroidType::Large;
		}
		else if (radius>30) {
			asteroidType = AsteroidType::Medium;
		}
		else {
			asteroidType = AsteroidType::Small;
		}
		setPosition(centralPoint);
	}

	void moveByTime(float timePerFrame) {
		velocity->x = sin(*moveAngle / 180 * PI) * *speedPerSecond;
		velocity->y = -cos(*moveAngle / 180 * PI) * *speedPerSecond;
		move(*velocity * timePerFrame);
	}
	bool collidedWith(CircleObject* object) {
		if (object->circleObjectType == CircleObjectType::Spacecraft) {
			*destroyed = true;
		}
		if (object->circleObjectType == CircleObjectType::Asteroid) {
			if (lastHitObject != object) {
				*moveAngle -= 180;
				if (*moveAngle < 0) {
					*moveAngle += 360;
				}
				lastHitObject = object;
			}
			
			//Momentum Conservation and Energy Conservation
			/*if (lastHitObject != object) {
				float massOfThisObject = 4 / 3 * PI * getRadius() * getRadius();
				float massOfComingObject = 4 / 3 * PI * object->getRadius() * object->getRadius();
				velocity->x = ((massOfThisObject - massOfComingObject) * velocity->x + 2 * massOfComingObject * object->velocity->x) / (massOfThisObject + massOfComingObject);
				velocity->y = ((massOfThisObject - massOfComingObject) * velocity->y + 2 * massOfComingObject * object->velocity->y) / (massOfThisObject + massOfComingObject);
				lastHitObject = object;
			}*/
			*destroyed = false;
		}
		if (object->circleObjectType == CircleObjectType::Bullet) {
			*destroyed = true;
		}
		if (object->circleObjectType == CircleObjectType::PowerUp) {
			*destroyed = false;
		}
		return *destroyed;
	}

	Asteroid* clone() {
		sf::Vector2f position = getPosition();
		if (position.x < getRadius()) {
			position.x += LENGTHOFTHEWINDOW;
		}
		else if (position.x > LENGTHOFTHEWINDOW - getRadius()) {
			position.x -= LENGTHOFTHEWINDOW;
		}
		if (position.y < getRadius()) {
			position.y += HEIGHTHTOFTHEWINDOW;
		}
		else if (position.y > HEIGHTHTOFTHEWINDOW - getRadius()) {
			position.y -= HEIGHTHTOFTHEWINDOW;
		}
		Asteroid* newObject = new Asteroid(position, *speedPerSecond, *moveAngle, getRadius());
		newObject->splitByHorizonticalLine = splitByHorizonticalLine;
		newObject->splitByVerticalLine = splitByVerticalLine;
		newObject->touchHorizonticalBound = touchHorizonticalBound;
		newObject->touchVerticalBound = touchVerticalBound;
		newObject->containedByHowManyGrid = 0;
		newObject->velocity = velocity;
		newObject->destroyed = destroyed;
		newObject->moveAngle = moveAngle;
		return newObject;
	}

	void breakUp(vector<CircleObject*>& waitListOfCrossingObject) {
		switch (asteroidType)
		{
		case AsteroidType::Small: {
			break;
		}
		case AsteroidType::Medium: {
			std::default_random_engine generator(time(NULL));
			std::uniform_real_distribution<float> distributionOfRadius(15,30);
			std::uniform_real_distribution<float> distributionOfSpeed(0, 1.2 * *speedPerSecond);
			std::uniform_real_distribution<float> distributionOfAngle(0,360);
			float childRadius = distributionOfRadius(generator);
			float childSpeed = distributionOfSpeed(generator);
			float childAngle = distributionOfAngle(generator);
			sf::Vector2f directionOne = sf::Vector2f(sin(childAngle / 180 * PI), -cos(childAngle / 180 * PI));
			sf::Vector2f directionTwo = sf::Vector2f(-sin(childAngle / 180 * PI), cos(childAngle / 180 * PI));
			waitListOfCrossingObject.push_back(new Asteroid(getPosition() + directionOne * childRadius * float(1.1), childSpeed, childAngle, childRadius));
			waitListOfCrossingObject.push_back(new Asteroid(getPosition() + directionTwo * childRadius * float(1.1), childSpeed, childAngle + 180 > 360 ? childAngle - 180 : childAngle + 180, childRadius));
			break;
		}
		case AsteroidType::Large: {
			std::default_random_engine generator(time(NULL));
			std::uniform_real_distribution<float> distributionOfRadius(30, 60);
			std::uniform_real_distribution<float> distributionOfSpeed(0, 1.2 * *speedPerSecond);
			std::uniform_real_distribution<float> distributionOfAngle(0, 360);
			float childRadius = distributionOfRadius(generator);
			float childSpeed = distributionOfSpeed(generator);
			float childAngle = distributionOfAngle(generator);
			sf::Vector2f directionOne = sf::Vector2f(sin(childAngle / 180 * PI), -cos(childAngle / 180 * PI));
			sf::Vector2f directionTwo = sf::Vector2f(-sin(childAngle / 180 * PI), cos(childAngle / 180 * PI));
			waitListOfCrossingObject.push_back(new Asteroid(getPosition() + directionOne * childRadius * float(1.1), childSpeed, childAngle, childRadius));
			waitListOfCrossingObject.push_back(new Asteroid(getPosition() + directionTwo * childRadius * float(1.1), childSpeed, childAngle + 180 > 360 ? childAngle - 180 : childAngle + 180, childRadius));
			break;
		}
		default:
			break;
		}
	}

	~Asteroid() {
	}
};

class Bullet :public CircleObject {
public:
	Bullet(sf::Vector2f centerPosition,float direction,CampType campType, float speed = SPEEDOFTHEBULLET) {
		sf::Vector2f bulletPosition;
		bulletPosition.x = centerPosition.x + sin(direction / 180 * PI) * RADIUSOFSPACESHIP;
		bulletPosition.y = centerPosition.y - cos(direction / 180 * PI) * RADIUSOFSPACESHIP;
		initializeBullet(bulletPosition, speed, direction, campType);
	}
	Bullet(sf::Vector2f centerPosition, sf::Vector2f directionOfBullet) {
		camp = CampType::Hostile;
		float normalize = sqrt(directionOfBullet.y * directionOfBullet.y + directionOfBullet.x * directionOfBullet.x);
		velocity->x = directionOfBullet.x/normalize;
		velocity->y = directionOfBullet.y/normalize;
		*speedPerSecond = SPEEDOFTHEBULLET;
		centralPoint = centerPosition;
		setOutlineColor(sf::Color::Red);
		setRadius(RADIUSOFBULLET);
		setOrigin(RADIUSOFBULLET, RADIUSOFBULLET);
		setPosition(centralPoint);
		setFillColor(sf::Color::Red);
	}
	void initializeBullet(sf::Vector2f centerPosition, float speed, float direction, CampType campType) {
		camp = campType;
		circleObjectType = CircleObjectType::Bullet;
		*speedPerSecond = speed;
		*moveAngle = direction;
		velocity->x = sin(*moveAngle / 180 * PI);
		velocity->y = -cos(*moveAngle / 180 * PI);
		centralPoint = centerPosition;
		if (camp == CampType::Friendly) {
			setOutlineColor(sf::Color::White);
		}
		else {
			setOutlineColor(sf::Color::Red);
		}
		setRadius(RADIUSOFBULLET);
		setOrigin(RADIUSOFBULLET, RADIUSOFBULLET);
		setPosition(centralPoint);
	}

	bool collidedWith(CircleObject* object) {
		if (object->circleObjectType == CircleObjectType::Spacecraft) {
			*destroyed = camp == object->camp ? false : true;
		}
		if (object->circleObjectType == CircleObjectType::Asteroid) {
			*destroyed = true;
		}
		if (object->circleObjectType == CircleObjectType::Bullet) {
			*destroyed = camp == object->camp ? false : true;
		}
		if (object->circleObjectType == CircleObjectType::PowerUp) {
			*destroyed = false;
		}
		return *destroyed;
	}

	void moveByTime(float timePerFrame) {
		move(*velocity * *speedPerSecond *timePerFrame);
	}

	~Bullet() {
	}
};

class PowerUp :public CircleObject {
public:
	PowerUp(sf::Vector2f centerPosition,PowerUpType type) {
		initializePowerUp(centerPosition,type);
	}
	/*Bullet(sf::Vector2f centerPosition, float speed, float direction, CampType campType) {
		initializeBullet(centerPosition, speed, direction, campType);
	}*/
	void initializePowerUp(sf::Vector2f centerPosition,PowerUpType type) {
		camp = CampType::Friendly;
		circleObjectType = CircleObjectType::PowerUp;
		powerUpType = type;
		*speedPerSecond = 0;
		*moveAngle = 0;
		*velocity = sf::Vector2f(0, 0);
		centralPoint = centerPosition;
		setFillColor(sf::Color::Color(18, 150, 219, 255));
		switch (powerUpType)
		{
		case PowerUpType::Invincible:
			setTexture(&textureOfInvincible);
			break;
		case PowerUpType::RapidShot:
			setTexture(&textureOfRapidShot);
			break;
		case PowerUpType::Shield:
			setTexture(&textureOfShield);
			break;
		case PowerUpType::ThreeShot:
			setTexture(&textureOfThreeShot);
			break;
		default:
			break;
		}
		//setTexture(&textureOfSpacecraft);
		setRadius(RADIUSOFSPACESHIP);
		setOrigin(RADIUSOFSPACESHIP, RADIUSOFSPACESHIP);
		setPosition(centralPoint);
		setOutlineThickness(3);
		setOutlineColor(sf::Color::Color(18, 150, 219, 255));
	}
	bool collidedWith(CircleObject* object) {
		if (object->circleObjectType == CircleObjectType::Spacecraft) {
			*destroyed = camp == object->camp ? true : false;
		}
		if (object->circleObjectType == CircleObjectType::Asteroid) {
			*destroyed = false;
		}
		if (object->circleObjectType == CircleObjectType::Bullet) {
			*destroyed = false;
		}
		if (object->circleObjectType == CircleObjectType::PowerUp) {
			*destroyed = false;
		}
		return *destroyed;
	}
	PowerUp* clone() {
		sf::Vector2f position = getPosition();
		if (position.x < getRadius()) {
			position.x += LENGTHOFTHEWINDOW;
		}
		else if (position.x > LENGTHOFTHEWINDOW - getRadius()) {
			position.x -= LENGTHOFTHEWINDOW;
		}
		if (position.y < getRadius()) {
			position.y += HEIGHTHTOFTHEWINDOW;
		}
		else if (position.y > HEIGHTHTOFTHEWINDOW - getRadius()) {
			position.y -= HEIGHTHTOFTHEWINDOW;
		}
		PowerUp* newObject = new PowerUp(position,powerUpType);
		newObject->splitByHorizonticalLine = splitByHorizonticalLine;
		newObject->splitByVerticalLine = splitByVerticalLine;
		newObject->touchHorizonticalBound = touchHorizonticalBound;
		newObject->touchVerticalBound = touchVerticalBound;
		newObject->containedByHowManyGrid = 0;
		return newObject;
	}

};

bool checkCollisionBetweenTwoCircleObject(CircleObject* objectOne, CircleObject* objectTwo) {
	float distanceOnX = objectOne->getPosition().x - objectTwo->getPosition().x;
	float distanceOnY = objectOne->getPosition().y - objectTwo->getPosition().y;
	float collisionDistance = objectOne->getRadius() + objectTwo->getRadius();
	if (distanceOnX * distanceOnX + distanceOnY * distanceOnY <= collisionDistance * collisionDistance) {
		return true;
	}
	else {
		return false;
	}
}

class GameGrid {
public:
	sf::Vector2f centerOfThisGrid;
	float lengthOfThisGrid;
	float heightOfThisGrid;
	GameGrid() {
		centerOfThisGrid = sf::Vector2f(0,0);
		lengthOfThisGrid = 0;
		heightOfThisGrid = 0;
	}
	GameGrid(sf::Vector2f centerPoint, float length, float height) {
		centerOfThisGrid = centerPoint;
		lengthOfThisGrid = length;
		heightOfThisGrid = height;
	}
	ObjectRelationshipType containObject(CircleObject* object) {
		float distanceToLeftLine = object->getPosition().x - centerOfThisGrid.x + lengthOfThisGrid / 2;
		float distanceToRightLine = object->getPosition().x - centerOfThisGrid.x - lengthOfThisGrid / 2;
		float distanceToTopLine = object->getPosition().y - centerOfThisGrid.y + heightOfThisGrid / 2;
		float distanceToBottomLine = object->getPosition().y - centerOfThisGrid.y - heightOfThisGrid / 2;
		float radius = object->getRadius();
		float absdistanceToLeftTopCorner = sqrt(distanceToLeftLine * distanceToLeftLine + distanceToTopLine * distanceToTopLine);
		float absdistanceToRightTopCorner = sqrt(distanceToRightLine * distanceToRightLine + distanceToTopLine * distanceToTopLine);
		float absdistanceToLeftBottomCorner = sqrt(distanceToLeftLine * distanceToLeftLine + distanceToBottomLine * distanceToBottomLine);
		float absdistanceToRightBottomCorner = sqrt(distanceToRightLine * distanceToRightLine + distanceToBottomLine * distanceToBottomLine);

		if (distanceToRightLine <= -radius && distanceToLeftLine >= radius && distanceToBottomLine <= -radius && distanceToTopLine >= radius) {
			return ObjectRelationshipType::Contain;
		}
		else if (absdistanceToLeftBottomCorner < radius || absdistanceToLeftTopCorner < radius || absdistanceToRightBottomCorner < radius || absdistanceToRightTopCorner < radius) {
			return ObjectRelationshipType::IntersectAndContainCornor;
		}
		else if (distanceToLeftLine >= -radius && distanceToLeftLine < radius && distanceToTopLine > 0 && distanceToBottomLine < 0) {
			return  ObjectRelationshipType::IntersectVertically;
		}
		else if (distanceToRightLine > -radius && distanceToRightLine <= radius && distanceToTopLine > 0 && distanceToBottomLine < 0) {
			return  ObjectRelationshipType::IntersectVertically;
		}
		else if (distanceToTopLine >= -radius && distanceToTopLine < radius && distanceToLeftLine >0 && distanceToRightLine < 0) {
			return ObjectRelationshipType::IntersectHorizontically;
		}
		else if (distanceToBottomLine > -radius && distanceToBottomLine <= radius && distanceToLeftLine > 0 && distanceToRightLine < 0) {
			return ObjectRelationshipType::IntersectHorizontically;
		}
		else {
			return ObjectRelationshipType::Separated;
		}
	}
};

class GridOfQuadTree:public GameGrid {
public:	
	int depthOfThisGrid;
	bool split = false;
	GridOfQuadTree* pointerToChild[4];
	list<CircleObject*> objectInThisGrid;
	

	GridOfQuadTree(sf::Vector2f centerPoint, float length, float height, int depth) {
		depthOfThisGrid = depth;
		centerOfThisGrid = centerPoint;
		lengthOfThisGrid = length;
		heightOfThisGrid = height;
		pointerToChild[0] = pointerToChild[1] = pointerToChild[2] = pointerToChild[3] = nullptr;
	}
	
	

	void splitCurrentGrid() {
		sf::Vector2f newCenter;
		if (depthOfThisGrid < DEPTHOFQUADTREE) {
			for (int i = 0; i < 4; i++) {
				newCenter.x = (i & 1 ? lengthOfThisGrid / 4 : -lengthOfThisGrid / 4) + centerOfThisGrid.x;
				newCenter.y = (i & 2 ? heightOfThisGrid / 4 : -heightOfThisGrid / 4) + centerOfThisGrid.y;
				pointerToChild[i] = new GridOfQuadTree(newCenter, lengthOfThisGrid / 2, heightOfThisGrid / 2, depthOfThisGrid + 1);
			}
		}
	}

	void insertObject(CircleObject* object) {
		GameGrid* gameArea = new GameGrid(sf::Vector2f(LENGTHOFTHEWINDOW / 2, HEIGHTHTOFTHEWINDOW / 2), LENGTHOFTHEWINDOW, HEIGHTHTOFTHEWINDOW);
		if (gameArea->containObject(object) == ObjectRelationshipType::Separated) {
			float positionX = object->getPosition().x;
			float positionY = object->getPosition().y;
			if (positionX < 0) {
				positionX += LENGTHOFTHEWINDOW;
			}
			else if (positionX > LENGTHOFTHEWINDOW) {
				positionX -= LENGTHOFTHEWINDOW;
			}
			if (positionY < 0) {
				positionY += HEIGHTHTOFTHEWINDOW;
			}
			else if (positionY > HEIGHTHTOFTHEWINDOW) {
				positionY -= HEIGHTHTOFTHEWINDOW;
			}
			object->setPosition(positionX,positionY);
		}
		delete gameArea;
		if (depthOfThisGrid >= DEPTHOFQUADTREE || nullptr == pointerToChild[0]) {
			objectInThisGrid.push_back(object);
			object->containedByHowManyGrid += 1;
			if (objectInThisGrid.size() >= 10 && depthOfThisGrid < DEPTHOFQUADTREE) {
				splitCurrentGrid();
				while (!objectInThisGrid.empty()) {
					objectInThisGrid.back()->containedByHowManyGrid = 0;
					insertObject(objectInThisGrid.back());
					objectInThisGrid.pop_back();
				}
			}
		}
		else {
			for (int i = 0; i < 4; i++) {
				switch (pointerToChild[i]->containObject(object)) {
				case ObjectRelationshipType::Contain:

					pointerToChild[i]->insertObject(object);
					break;
				case ObjectRelationshipType::IntersectHorizontically:
					object->splitByHorizonticalLine = true;
					pointerToChild[i]->insertObject(object);
					break;
				case ObjectRelationshipType::IntersectVertically:
					object->splitByVerticalLine = true;
					pointerToChild[i]->insertObject(object);
					break;
				case ObjectRelationshipType::IntersectTwoLinesNoCornor:
					object->splitByHorizonticalLine = true;
					object->splitByVerticalLine = true;
					pointerToChild[i]->insertObject(object);
					break;
				case ObjectRelationshipType::IntersectAndContainCornor:
					object->splitByHorizonticalLine = true;
					object->splitByVerticalLine = true;
					pointerToChild[i]->insertObject(object);
					break;
				case ObjectRelationshipType::Separated:
					break;
				}
			}
		}
	}

	void goThroughTheTreeAndUpDate(GameGrid*& gameGrid,vector<CircleObject*>& waitListOfCrossingObject, list<Explosion*>& explosionPosition,int* score,bool* spacecraftIsAlive) {
		if (!objectInThisGrid.empty()) {
			for (auto i = objectInThisGrid.begin(); i != objectInThisGrid.end();) {
				if (*((*i)->destroyed) || (*i)->shouldBeRemoved) {
					i++;
				}
				else {
					auto j = i;
					j++;
					for (; j != objectInThisGrid.end(); j++) {
						if (!(*(*j)->destroyed)) {
							if (checkCollisionBetweenTwoCircleObject(*i, *j)) {
								bool added = false;
								if ((*j)->collidedWith(*i)) {
									if ((*j)->circleObjectType == CircleObjectType::Asteroid) {
										soundOfAsteroidBroken.play();
										*score += (*i)->getRadius()* (*i)->getRadius();
										(*j)->breakUp(waitListOfCrossingObject);
										if (rand() % 10 > 7) {
											waitListOfCrossingObject.push_back(new PowerUp((*j)->getPosition(), PowerUpType(rand() % 4)));
										}
										explosionPosition.push_back(new Explosion(sf::Vector2f(((*i)->getPosition().x + (*j)->getPosition().x) / 2, ((*i)->getPosition().y + (*j)->getPosition().y) / 2)));
										added = true;
									}
									if ((*j)->circleObjectType == CircleObjectType::Spacecraft){
										soundOfspacecraftBroken.play();
										if ((*j)->camp != CampType::Hostile) {
											*spacecraftIsAlive = false;
											if (!added) {
												explosionPosition.push_back(new Explosion(sf::Vector2f(((*i)->getPosition().x + (*j)->getPosition().x) / 2, ((*i)->getPosition().y + (*j)->getPosition().y) / 2)));
												added = true;
											}
										}
									}
									
								}
								if ((*i)->collidedWith(*j)) {
									if ((*i)->circleObjectType == CircleObjectType::Asteroid) {
										soundOfAsteroidBroken.play();
										*score += (*i)->getRadius() * (*i)->getRadius();
										(*i)->breakUp(waitListOfCrossingObject);
										if (rand() % 10 > 7) {
											waitListOfCrossingObject.push_back(new PowerUp((*j)->getPosition(), PowerUpType(rand() % 4)));
										}
										if (!added) {
											explosionPosition.push_back(new Explosion(sf::Vector2f(((*i)->getPosition().x + (*j)->getPosition().x) / 2, ((*i)->getPosition().y + (*j)->getPosition().y) / 2)));
											added = true;
										}
									}
									if ((*i)->circleObjectType == CircleObjectType::Spacecraft){
										soundOfspacecraftBroken.play();
										if ((*i)->camp != CampType::Hostile){
											*spacecraftIsAlive = false;
											if (!added) {
												explosionPosition.push_back(new Explosion(sf::Vector2f(((*i)->getPosition().x + (*j)->getPosition().x) / 2, ((*i)->getPosition().y + (*j)->getPosition().y) / 2)));
											}
										}
									}
									break;
								}
							}
						}
					}
					bool clone = false;
					switch (gameGrid->containObject(*i)) {
					case ObjectRelationshipType::Contain:
						(*i)->touchHorizonticalBound = false;
						(*i)->touchVerticalBound = false;
						break;
					case ObjectRelationshipType::IntersectHorizontically:
						(*i)->touchVerticalBound = false;
						if ((*i)->touchHorizonticalBound == false) {
							(*i)->touchHorizonticalBound = true;
							clone = true;
						}
						break;
					case ObjectRelationshipType::IntersectVertically:
						(*i)->touchHorizonticalBound = false;
						if ((*i)->touchVerticalBound == false) {
							(*i)->touchVerticalBound = true;
							clone = true;
						}
						break;
					case ObjectRelationshipType::IntersectTwoLinesNoCornor:
						if ((*i)->touchVerticalBound == false) {
							(*i)->touchVerticalBound = true;
							clone = true;
						}
						if ((*i)->touchHorizonticalBound == false) {
							(*i)->touchHorizonticalBound = true;
							clone = true;
						}
						break;
					case ObjectRelationshipType::IntersectAndContainCornor:
						if ((*i)->touchVerticalBound == false) {
							(*i)->touchVerticalBound = true;
							clone = true;
						}
						if ((*i)->touchHorizonticalBound == false) {
							(*i)->touchHorizonticalBound = true;
							clone = true;
						}
						break;
					case ObjectRelationshipType::Separated:
						(*i)->destroyed = new bool(true);
						break;
					}
					if (clone) {
						if ((*i)->circleObjectType != CircleObjectType::Bullet && (*i)->camp != CampType::Hostile) {
							waitListOfCrossingObject.push_back((*i)->clone());
						}
					}
					switch (containObject(*i)) {
					case ObjectRelationshipType::Contain:
						(*i)->splitByHorizonticalLine = false;
						(*i)->splitByVerticalLine = false;
						i++;
						break;
					case ObjectRelationshipType::IntersectHorizontically:
						(*i)->splitByVerticalLine = false;
							
						if ((*i)->containedByHowManyGrid<=1) {
							(*i)->splitByHorizonticalLine = true;
							waitListOfCrossingObject.push_back(*i);
							objectInThisGrid.erase(i++);
						}
						else {
							i++;
						}
						break;
					case ObjectRelationshipType::IntersectVertically:
							
						(*i)->splitByHorizonticalLine = false;
						if ((*i)->containedByHowManyGrid <= 1) {
							(*i)->splitByVerticalLine = true;					
							waitListOfCrossingObject.push_back(*i);
							objectInThisGrid.erase(i++);
						}
						else {
							i++;
						}
						break;
					case ObjectRelationshipType::IntersectTwoLinesNoCornor:
							
						(*i)->splitByVerticalLine = true;
						(*i)->splitByHorizonticalLine = true;
						if ((*i)->containedByHowManyGrid <= 2 && !(*i)->shouldBeRemoved) {
							waitListOfCrossingObject.push_back(*i);
							objectInThisGrid.erase(i++);
						}
						else {
							i++;
						}
						break;
					case ObjectRelationshipType::IntersectAndContainCornor:
						(*i)->splitByVerticalLine = true;
						(*i)->splitByHorizonticalLine = true;	
						if ((*i)->containedByHowManyGrid <= 3 && !(*i)->shouldBeRemoved) { 
							(*i)->shouldBeRemoved = true;
							waitListOfCrossingObject.push_back(*i);
						}
						else {
							i++;
						}
						break;
					case ObjectRelationshipType::Separated:
						(*i)->containedByHowManyGrid -= 1;
						if ((*i)->containedByHowManyGrid <= 0) {
							(*i)->~CircleObject();
						}
						objectInThisGrid.erase(i++);
						break;
					}
				}
			}
		}
		if (nullptr != pointerToChild[0]){
			for (int i = 0; i < 4; i++) {
				pointerToChild[i]->goThroughTheTreeAndUpDate(gameGrid, waitListOfCrossingObject,explosionPosition,score,spacecraftIsAlive);
			}
		}
	}
	int goThroughTheTreeAndDraw(sf::RenderWindow& window, float timePerFrame, int currentFrame, vector<CircleObject*>& waitListOfCrossingObject,bool* fire,sf::Vector2f* positionOfSpacecraft) {
		int numberOfAsteroid = 0;
		if (!objectInThisGrid.empty()) {
			for (auto i = objectInThisGrid.begin(); i != objectInThisGrid.end();) {
				if (*(*i)->destroyed) {
					(*i)->containedByHowManyGrid -= 1;
					if ((*i)->containedByHowManyGrid <= 0){
						(*i)->~CircleObject();
					}
					objectInThisGrid.erase(i++);
				}
				else if ((*i)->shouldBeRemoved) {
					(*i)->containedByHowManyGrid -= 1;
					if ((*i)->containedByHowManyGrid <= 0) {
						(*i)->shouldBeRemoved = false;
					}
					objectInThisGrid.erase(i++);
				}
				else {
					if ((*i)->circleObjectType == CircleObjectType::Spacecraft) {
						if ((*i)->camp == CampType::Friendly) {
							(*i)->controlByPlayer(sf::Keyboard::isKeyPressed(sf::Keyboard::Left), sf::Keyboard::isKeyPressed(sf::Keyboard::Right), sf::Keyboard::isKeyPressed(sf::Keyboard::Up), sf::Keyboard::isKeyPressed(sf::Keyboard::Down), timePerFrame, window);
							if (*fire) {
								vector<sf::Vector3f> positonOfBullet = (*i)->shootPoisition();
								while (!positonOfBullet.empty()) {
									waitListOfCrossingObject.push_back(new Bullet(sf::Vector2f(positonOfBullet.back().x, positonOfBullet.back().y), positonOfBullet.back().z, (*i)->camp));
									positonOfBullet.pop_back();
								}
								*fire = false;
							}
						}
						else {
							if (currentFrame % 400 == 0) {
								waitListOfCrossingObject.push_back(new Bullet((*i)->getPosition(),*positionOfSpacecraft - (*i)->getPosition()));
							}
						}
					}
					else if ((*i)->circleObjectType == CircleObjectType::Asteroid) {
						numberOfAsteroid += 1;
					}
					if ((*i)->frameNumber != currentFrame) {
						(*i)->moveByTime(timePerFrame);
						(*i)->frameNumber = currentFrame;
					}
					if (!*(*i)->destroyed) {
						(*i)->drawself(window);
					}
					i++;
				}
			}
		}
		if (nullptr != pointerToChild[0]) {
			for (int i = 0; i < 4; i++) {
				numberOfAsteroid += pointerToChild[i]->goThroughTheTreeAndDraw(window, timePerFrame, currentFrame, waitListOfCrossingObject,fire,positionOfSpacecraft);
			}
		}
		return numberOfAsteroid;
	}

	void clearGrid() {
		if (!objectInThisGrid.empty()) {
			for (auto i = objectInThisGrid.begin(); i != objectInThisGrid.end();) {
				(*i)->containedByHowManyGrid -= 1;
				if ((*i)->containedByHowManyGrid <= 0) {
					(*i)->~CircleObject();
				}
				objectInThisGrid.erase(i++);
			}
		}
		if (nullptr != pointerToChild[0]) {
			for (int i = 0; i < 4; i++) {
				pointerToChild[i]->clearGrid();
			}
		}
	}
};

float distanceBetweenTwoPoint(sf::Vector2f pointOne,sf::Vector2f pointTwo) {
	float distanceOnX = pointOne.x - pointTwo.x;
	float distanceOnY = pointOne.y - pointTwo.y;
	return sqrt(distanceOnX * distanceOnX + distanceOnY * distanceOnY);
}

void generateANewLevel(int levelNumber, GridOfQuadTree* rootGrid) {
	sf::Vector2f centerOfTheCircle[NUMBEROFGRIDFORGENERATECIRCLEONCOLUNM][NUMBEROFGRIDFORGENERATECIRCLEONROW];
	float radiusOfTheCircle[NUMBEROFGRIDFORGENERATECIRCLEONCOLUNM][NUMBEROFGRIDFORGENERATECIRCLEONROW];
	for (int i = 0; i < NUMBEROFGRIDFORGENERATECIRCLEONCOLUNM; i++) {
		for (int j = 0; j < NUMBEROFGRIDFORGENERATECIRCLEONROW; j++) {

			if (!((i == 3 || i == 2) && (j == 4 || j == 3))) {
				std::default_random_engine generator(i*10+j*100);
				std::uniform_real_distribution<float> distribution(-50, 50);
				centerOfTheCircle[i][j].x = j * 100 + 50 + distribution(generator);
				centerOfTheCircle[i][j].y = i * 100 + 50 + distribution(generator);
			}
			else {
				centerOfTheCircle[i][j].x = j * 100 + 50;
				centerOfTheCircle[i][j].y = i * 100 + 50;
			}
		}
	}
	for (int i = 0; i < NUMBEROFGRIDFORGENERATECIRCLEONCOLUNM; i++) {
		for (int j = 0; j < NUMBEROFGRIDFORGENERATECIRCLEONROW; j++) {
			vector<float> calculateRadius;
			if (j != 0) {
				calculateRadius.push_back(distanceBetweenTwoPoint(centerOfTheCircle[i][j], centerOfTheCircle[i][j - 1]));
			}
			if (j != NUMBEROFGRIDFORGENERATECIRCLEONROW - 1) {
				calculateRadius.push_back(distanceBetweenTwoPoint(centerOfTheCircle[i][j], centerOfTheCircle[i][j + 1]));
			}
			if (i != NUMBEROFGRIDFORGENERATECIRCLEONCOLUNM - 1) {
				calculateRadius.push_back(distanceBetweenTwoPoint(centerOfTheCircle[i][j], centerOfTheCircle[i + 1][j]));
			}
			if (i != NUMBEROFGRIDFORGENERATECIRCLEONCOLUNM - 1 && j != 0) {
				calculateRadius.push_back(distanceBetweenTwoPoint(centerOfTheCircle[i][j], centerOfTheCircle[i + 1][j - 1]));
			}
			if (i != NUMBEROFGRIDFORGENERATECIRCLEONCOLUNM - 1 && j != NUMBEROFGRIDFORGENERATECIRCLEONROW - 1) {
				calculateRadius.push_back(distanceBetweenTwoPoint(centerOfTheCircle[i][j], centerOfTheCircle[i + 1][j + 1]));
			}
			if (i != 0 && j != 0) {
				calculateRadius.push_back(distanceBetweenTwoPoint(centerOfTheCircle[i][j], centerOfTheCircle[i - 1][j - 1]));
			}
			if (i != 0) {
				calculateRadius.push_back(distanceBetweenTwoPoint(centerOfTheCircle[i][j], centerOfTheCircle[i - 1][j]));
			}
			if (i != 0 && j != NUMBEROFGRIDFORGENERATECIRCLEONROW - 1) {
				calculateRadius.push_back(distanceBetweenTwoPoint(centerOfTheCircle[i][j], centerOfTheCircle[i - 1][j + 1]));
			}
			float radius = *min_element(calculateRadius.begin(), calculateRadius.end());
			radiusOfTheCircle[i][j] = radius/2;

		}
	}
	for (int i = 0; i < NUMBEROFGRIDFORGENERATECIRCLEONCOLUNM; i++) {
		for (int j = 0; j < NUMBEROFGRIDFORGENERATECIRCLEONROW; j++) {
			if (!((i == 3 || i == 2) && (j == 4 || j == 3))) {
				std::default_random_engine generator(i * 100 + j * 10);
				std::uniform_real_distribution<float> distribution(0, 10);
				float randomSeed = distribution(generator);
				if (randomSeed > (10-levelNumber)) {
					std::uniform_real_distribution<float> distribution2(0, 360);
					float speedOfAsteroid = 0;
					speedOfAsteroid = BASICSPEEDOFASTEROID * (1 + levelNumber / 10);
					rootGrid->insertObject(new Asteroid(centerOfTheCircle[i][j],speedOfAsteroid, distribution2(generator), radiusOfTheCircle[i][j]));
				}
			}
		}
	}
}

int main()
{
	sf::RenderWindow window(sf::VideoMode(LENGTHOFTHEWINDOW, HEIGHTHTOFTHEWINDOW), "Asteroid");
	textureOfSpacecraft.loadFromFile("spaceship.png");
	textureOfAsteroid.loadFromFile("asteroid.jpg");
	textureOfExplosion.loadFromFile("explosion.png");
	textureOfPushingFire.loadFromFile("fire.png");
	textureOfInvincible.loadFromFile("star.png");
	textureOfRapidShot.loadFromFile("rapid.png");
	textureOfShield.loadFromFile("shield.png");
	textureOfThreeShot.loadFromFile("split.png");
	textureOfEnemy.loadFromFile("enemy.png");

	bufferOfShoot.loadFromFile("shoot.wav");
	soundOfShoot.setBuffer(bufferOfShoot);
	bufferOfLevelClear.loadFromFile("levelClear.wav");
	soundOfLevelClear.setBuffer(bufferOfLevelClear);
	bufferOfspacecraftBroken.loadFromFile("spacecraftBroken.wav");
	soundOfspacecraftBroken.setBuffer(bufferOfspacecraftBroken);
	bufferOfTheBallHitTheBrick.loadFromFile("ballHitBrick.wav");
	soundOfTheBallHitTheBrick.setBuffer(bufferOfTheBallHitTheBrick);
	bufferOfTheBallHitTheBound.loadFromFile("ballHitWall.wav");
	soundOfTheBallHitTheBound.setBuffer(bufferOfTheBallHitTheBound);
	bufferOfAsteroidBroken.loadFromFile("asteroidBroken.wav");
	soundOfAsteroidBroken.setBuffer(bufferOfAsteroidBroken);
	bufferOfThrust.loadFromFile("thrust.wav");
	soundOfThrust.setBuffer(bufferOfThrust);
	list<CircleObject*> listOfSpaceCraft;
	vector<CircleObject*> waitListOfCrossingObject;
	sf::RectangleShape upRecTangle = sf::RectangleShape(sf::Vector2f(LENGTHOFTHEWINDOW, HEIGHTHTOFTHEWINDOW / 2));
	sf::RectangleShape downRecTangle = sf::RectangleShape(sf::Vector2f(LENGTHOFTHEWINDOW, HEIGHTHTOFTHEWINDOW / 2));
	upRecTangle.setFillColor(sf::Color::White);
	downRecTangle.setFillColor(sf::Color::White);
	GridOfQuadTree* rootGrid = new GridOfQuadTree(sf::Vector2f(LENGTHOFTHEWINDOW / 2, HEIGHTHTOFTHEWINDOW / 2),GAMEAREALENGTH, GAMEAREAHEIGHT, 1);
	GameGrid* gameArea = new GameGrid(sf::Vector2f(LENGTHOFTHEWINDOW / 2, HEIGHTHTOFTHEWINDOW / 2), LENGTHOFTHEWINDOW, HEIGHTHTOFTHEWINDOW);
	list<Explosion*> explosion;
	sf::Vector2f positionOfSpacecraft = sf::Vector2f(LENGTHOFTHEWINDOW / 2, HEIGHTHTOFTHEWINDOW / 2);
	sf::Clock clock;
	int score = 0;
	int currentScore = 0;
	float timePerFrame = 0;
	int frameNumber = 0;
	int levelNumber = 1;
	int lifeOfPlayer = 2;
	bool nextLevelBegin = true;
	bool startCountForReborn = false;
	bool spacecraftIsAlive = false;
	sf::Clock clockForReborn;
	sf::Clock clockForProduceEnemy;
	GameScene stateOfTheGame = GameScene::MainMenu;
	while (window.isOpen())
	{
		switch (stateOfTheGame) {
		case GameScene::MainMenu: {
			TextForAsteroid startGame(std::string("Start"), FONTSIZEOFSCOREBOARD, sf::Color::White, sf::Vector2f(LENGTHOFTHEWINDOW / 2, HEIGHTHTOFTHEWINDOW - 12 * WIDTHOFTHEINTERVAL), AlignOfText::center);
			TextForAsteroid clickToStart(std::string("Click or press Space to start"), FONTSIZEOFPOWERUP, sf::Color::White, sf::Vector2f(LENGTHOFTHEWINDOW / 2, HEIGHTHTOFTHEWINDOW - 7 * WIDTHOFTHEINTERVAL), AlignOfText::center);
			window.clear();
			window.draw(startGame);
			window.draw(clickToStart);
			window.display();
			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
				if (event.type == sf::Event::MouseButtonReleased) {
					if (event.mouseButton.button == sf::Mouse::Left) {
						if (startGame.getGlobalBounds().contains(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y)) {
							lifeOfPlayer = 2;
							currentScore = 0;
							score = 0;
							levelNumber = 1;
							stateOfTheGame = GameScene::LevelClear;
							nextLevelBegin = true;
							clock.restart();
							clockForProduceEnemy.restart();
						}
					}
				}
				if (event.type == sf::Event::KeyReleased) {
					if (event.key.code == sf::Keyboard::Space) {
						lifeOfPlayer = 2;
						currentScore = 0;
						score = 0;
						levelNumber = 1;
						stateOfTheGame = GameScene::LevelClear;
						nextLevelBegin = true;
						clockForProduceEnemy.restart();
						clock.restart();
					}
				}
			}
			break;
		}
		case GameScene::LevelClear: {
			timePerFrame = clock.restart().asSeconds();
			TextForAsteroid startGame(std::string("Start"), FONTSIZEOFSCOREBOARD, sf::Color::Black, sf::Vector2f(LENGTHOFTHEWINDOW / 2, HEIGHTHTOFTHEWINDOW/2), AlignOfText::center);
			TextForAsteroid showLevel(std::string("Level:") + std::to_string(levelNumber), 0.8 * FONTSIZEOFSCOREBOARD, sf::Color::Color(60, 138, 255, 255), sf::Vector2f(LENGTHOFTHEWINDOW / 2, HEIGHTHTOFTHEWINDOW / 2 - startGame.getGlobalBounds().height), AlignOfText::center);
			if (nextLevelBegin) {
				soundOfLevelClear.play();
				rootGrid->clearGrid();
				upRecTangle.setPosition(sf::Vector2f(0, -HEIGHTHTOFTHEWINDOW / 2));
				downRecTangle.setPosition(sf::Vector2f(0, HEIGHTHTOFTHEWINDOW));
				generateANewLevel(levelNumber, rootGrid);
				rootGrid->insertObject(new Spacecraft(sf::Vector2f(LENGTHOFTHEWINDOW / 2, HEIGHTHTOFTHEWINDOW / 2), CampType::Friendly));
				spacecraftIsAlive = true;
				nextLevelBegin = false;
			}
			if (upRecTangle.getPosition().y < 0) {
				upRecTangle.move(0, HEIGHTHTOFTHEWINDOW * timePerFrame);
				downRecTangle.move(0, -HEIGHTHTOFTHEWINDOW * timePerFrame);
			}
			window.clear();
			window.draw(upRecTangle);
			window.draw(downRecTangle);
			if (upRecTangle.getPosition().y >= 0) {
				window.draw(showLevel);
				window.draw(startGame);
			}
			window.display();
			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
				if (event.type == sf::Event::MouseButtonReleased) {
					if (event.mouseButton.button == sf::Mouse::Left) {
						if (startGame.getGlobalBounds().contains(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y)) {
							stateOfTheGame = GameScene::GameStart;
							clock.restart();
							frameNumber = 0;
						}	
					}
				}
				if (event.type == sf::Event::KeyReleased) {
					if (event.key.code == sf::Keyboard::Space) {
						stateOfTheGame = GameScene::GameStart;
						clock.restart();
						frameNumber = 0;
					}
				}
			}
			break;
		}
		case GameScene::GameStart: {
			timePerFrame = clock.restart().asSeconds();
			bool fire = false;
			if (currentScore < score) {
				currentScore += (score - currentScore) / 100 + 1;
			}
			TextForAsteroid scoreBoard(std::to_string(currentScore), FONTSIZEOFSCOREBOARD, sf::Color::Color(60, 138, 255, 255), sf::Vector2f(0, 0), AlignOfText::leftAlign);
			TextForAsteroid lifeBoard(std::string("Life:") + std::to_string(lifeOfPlayer), FONTSIZEOFSCOREBOARD, sf::Color::Color(60, 138, 255, 255), sf::Vector2f(LENGTHOFTHEWINDOW - LENGTHOFTHEWINDOW/10, 0), AlignOfText::rightAlign);
			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
				if (event.type == sf::Event::KeyReleased) {
					if (event.key.code == sf::Keyboard::Space) {
						fire = true;
					}
					if (event.key.code == sf::Keyboard::Escape) {
						stateOfTheGame = GameScene::MainMenu;
					}
				}
			}
			std::default_random_engine generator(time(NULL));
			std::uniform_real_distribution<float> distributionOfTime(5, 6);
			if (clockForProduceEnemy.getElapsedTime().asSeconds() > distributionOfTime(generator)) {
				std::uniform_real_distribution<float> distributionOfPositionY(0, HEIGHTHTOFTHEWINDOW);
				rootGrid->insertObject(new Spacecraft(sf::Vector2f(0, distributionOfPositionY(generator)), 200, distributionOfPositionY(generator) * 0.3, CampType::Hostile));
				clockForProduceEnemy.restart();
			}
			window.clear();
			rootGrid->goThroughTheTreeAndUpDate(gameArea, waitListOfCrossingObject,explosion,&score,&spacecraftIsAlive);
			int numeberOfAsteroid = rootGrid->goThroughTheTreeAndDraw(window, timePerFrame, frameNumber, waitListOfCrossingObject,&fire, &positionOfSpacecraft);
			if (numeberOfAsteroid == 0 && waitListOfCrossingObject.empty()) {
				levelNumber += 1;
				stateOfTheGame = GameScene::LevelClear;
				nextLevelBegin = true;
				
			}
			if (!spacecraftIsAlive) {
				if (!startCountForReborn) {
					clockForReborn.restart();
					startCountForReborn = true;
				}
				else if (clockForReborn.getElapsedTime().asSeconds() > 1) {
					rootGrid->insertObject(new Spacecraft(sf::Vector2f(LENGTHOFTHEWINDOW / 2, HEIGHTHTOFTHEWINDOW / 2), CampType::Friendly));
					spacecraftIsAlive = true;;
					lifeOfPlayer -= 1;
					startCountForReborn = false;
					if (lifeOfPlayer < 0) {
						stateOfTheGame = GameScene::GameOver;
					}
				}
			}
			while (!waitListOfCrossingObject.empty()) {
				waitListOfCrossingObject.back()->containedByHowManyGrid = 0;
				rootGrid->insertObject(waitListOfCrossingObject.back());
				waitListOfCrossingObject.pop_back();
			}
			if (!explosion.empty()) {
				for (auto i = explosion.begin(); i != explosion.end();) {
					if ((*i)->clockOfLife.getElapsedTime().asSeconds() > 0.8) {
						(*i)->~Explosion();
						explosion.erase(i++);
					}
					else {
						(*i)->drawSelf(window);
						i++;
					}
				}
			}
			window.draw(scoreBoard);
			window.draw(lifeBoard);
			window.display();
			frameNumber++;
			break;
		}
		case GameScene::GameOver:
			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
				if (event.type == sf::Event::KeyReleased) {
					if (event.key.code == sf::Keyboard::Space) {
						stateOfTheGame = GameScene::MainMenu;
					}
				}
			}
			TextForAsteroid startGame(std::string("GameOver"), FONTSIZEOFSCOREBOARD, sf::Color::White, sf::Vector2f(LENGTHOFTHEWINDOW / 2, HEIGHTHTOFTHEWINDOW / 2), AlignOfText::center);
			TextForAsteroid scoreBoard(std::string("Your Score:") + std::to_string(score), 0.8 * FONTSIZEOFSCOREBOARD, sf::Color::Color(60, 138, 255, 255), sf::Vector2f(LENGTHOFTHEWINDOW / 2, HEIGHTHTOFTHEWINDOW / 2 - startGame.getGlobalBounds().height), AlignOfText::center);
			window.clear();
			window.draw(startGame);
			window.draw(scoreBoard);
			window.display();
			break;
		}
	}

	return 0;
}

