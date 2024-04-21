#pragma once
#include "Utilities.h"

class Actor;

class Rigidbody
{
private:

public:
	static inline bool world_initialized = false;
	static inline b2World* world;
	
	b2Body* body = nullptr;

	string type = "Rigidbody";
	string key = "???";
	Actor* actor = nullptr;
	bool enabled = true;

	Rigidbody() {
		if (!world_initialized) {
			world_initialized = true;
			b2Vec2 gravity(0.0f, 9.8f);
			world = new b2World(gravity);
		}
	}

	float x = 0.0f;
	float y = 0.0f;
	string body_type = "dynamic";
	bool precise = true;
	float gravity_scale = 1.0f;
	float density = 1.0f;
	float angular_friction = 0.3f;
	float rotation = 0.0f;
	bool has_collider = true;
	string collider_type = "box";
	bool has_trigger = true;
	string trigger_type = "box";
	float trigger_width = 1.0f;
	float trigger_height = 1.0f;
	float trigger_radius = 0.5f;

	float width = 1.0f;
	float height = 1.0f;
	float radius = 0.5f;
	float friction = 0.3f;
	float bounciness = 0.3f;

	b2Vec2 GetPosition() const {
		return body->GetPosition();
	}

	float GetRotation() const {
		float radians = body->GetAngle();
		return radians * (180.0f / b2_pi);
	}

	void AddForce(b2Vec2 vec2) {
		body->ApplyForceToCenter(vec2, true);
	}

	void SetVelocity(b2Vec2 vec2) {
		body->SetLinearVelocity(vec2);
	}

	void SetPosition(b2Vec2 vec2) {
		if (body == nullptr) {
			x = vec2.x;
			y = vec2.y;
			return;
		}
		body->SetTransform(vec2, body->GetAngle());
	}

	void SetRotation(float degrees_clockwise) {
		float radians_clockwise = degrees_clockwise * (b2_pi / 180.0f);
		body->SetTransform(body->GetPosition(), radians_clockwise);
	}

	void SetAngularVelocity(float degrees_clockwise) {
		float radians_clockwise = degrees_clockwise * (b2_pi / 180.0f);
		body->SetAngularVelocity(radians_clockwise);
	}

	void SetGravityScalse(float scale) {
		body->SetGravityScale(scale);
	}

	void SetUpDirection(b2Vec2 direction) {
		direction.Normalize();
		float angle = glm::atan(direction.x, -direction.y);
		body->SetTransform(body->GetPosition(), angle);
	}

	void SetRightDirection(b2Vec2 direction) {
		direction.Normalize();
		float angle = atan2(direction.x, -direction.y);
		angle -= b2_pi / 2.0f;

		body->SetTransform(body->GetPosition(), angle);
	}

	b2Vec2 GetVelocity() const {
		if (body == nullptr) {
			return b2Vec2(x, y);
		}
		return body->GetLinearVelocity();
	}

	float GetAngularVelocity() const {
		float angularVelocityRadians = body->GetAngularVelocity();
		float angularVelocityDegrees = angularVelocityRadians * (180.0f / b2_pi);

		return angularVelocityDegrees;
	}

	float GetGravityScale() const {
		return body->GetGravityScale();
	}

	b2Vec2 GetUpDirection() const {
		float angle = body->GetAngle();

		float x = sin(angle);
		float y = -cos(angle);
		b2Vec2 upDirection = b2Vec2(x, y);
		upDirection.Normalize();
		//testing autograder discrepancy
		return upDirection;
	}

	b2Vec2 GetRightDirection() const {
		float angle = body->GetAngle();
		
		float x = cos(angle);
		float y = sin(angle);

		b2Vec2 rightDirection = b2Vec2(x, y);
		rightDirection.Normalize();

		return rightDirection;
	}

	void OnDestroy() const {
		world->DestroyBody(body);
	}

	void OnStart() {
		b2BodyDef bodyDef;
		if (body_type == "dynamic") {
			bodyDef.type = b2_dynamicBody;
		}
		else if (body_type == "kinematic") {
			bodyDef.type = b2_kinematicBody;
		}
		else if (body_type == "static") {
			bodyDef.type = b2_staticBody;
		}

		bodyDef.position = b2Vec2(x, y);
		bodyDef.bullet = precise;
		bodyDef.angularDamping = angular_friction;
		bodyDef.gravityScale = gravity_scale;
		bodyDef.angle = rotation * (b2_pi / 180.0f);

		body = world->CreateBody(&bodyDef);

		if (!has_collider && !has_trigger) {
			b2PolygonShape phantom_shape;
			phantom_shape.SetAsBox(width * 0.5f, height * 0.5f);

			b2FixtureDef phantom_fixture_def;
			phantom_fixture_def.shape = &phantom_shape;
			phantom_fixture_def.density = density;

			phantom_fixture_def.isSensor = true;
			body->CreateFixture(&phantom_fixture_def);
		}
		else {
			if (has_collider) {
				b2FixtureDef fixture;
				fixture.isSensor = false;
				b2PolygonShape box;
				b2CircleShape circle;

				if (collider_type == "box") {

					box.SetAsBox(width * 0.5f, height * 0.5f);
					fixture.shape = &box;
				}
				else if (collider_type == "circle") {
					circle.m_radius = radius;
					fixture.shape = &circle;
				}

				fixture.density = density;
				fixture.friction = friction;
				fixture.restitution = bounciness;
				fixture.userData.pointer = reinterpret_cast<uintptr_t>(actor);
				body->CreateFixture(&fixture);
			}
			if (has_trigger) {
				b2FixtureDef fixture;
				fixture.isSensor = true;
				b2PolygonShape box;
				b2CircleShape circle;
				if (trigger_type == "box") {

					box.SetAsBox(trigger_width * 0.5f, trigger_height * 0.5f);
					fixture.shape = &box;
				}
				else if (trigger_type == "circle") {
					circle.m_radius = trigger_radius;
					fixture.shape = &circle;
				}
				fixture.density = density;
				fixture.friction = friction;
				fixture.restitution = bounciness;
				fixture.userData.pointer = reinterpret_cast<uintptr_t>(actor);
				body->CreateFixture(&fixture);
			}
			
		}
	}

	static inline std::shared_ptr<luabridge::LuaRef> GetRigidComponent(lua_State* lua_state) {
		Rigidbody* rigidbody = new Rigidbody();
		luabridge::push(lua_state, rigidbody);

		luabridge::LuaRef componentRef = luabridge::LuaRef::fromStack(lua_state, -1);
		lua_pop(lua_state, 1); // Pop the object from the stack after creating the LuaRef

		return std::make_shared<luabridge::LuaRef>(componentRef);
	}
};