#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "matrix.h"
#include "quaternion.h"
#include <cmath>

namespace math {
	class Object {

		Matrix4 _model;
		float _radius;
		Vector2 _direction;
		Vector2 _speed;
		Vector2 _acceleration;
		float _mass;
		float friction = 0.0002;
		Quaternion rotationQuaternion = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);

	public:
		Object(){}
		Object(Matrix4 m, float r, Vector2 d, Vector2 s, Vector2 a, float mass) {
			_model = m;
			_radius = 0.32;
			_direction = d;
			_speed = s;
			_acceleration = a;
			_mass = mass;
		}
		~Object(){}

		Matrix4 model() { return this->_model; }
		Matrix4 modelMatrix() { return this->_model*this->rotationQuaternion.toMatrix(); }
		Vector2 direction() { return this->_direction; }
		Vector2 speed() { return this->_speed; }
		Vector2 acceleration() { return this->_acceleration; }
		float radius() { return this->_radius; }
		float mass() { return _mass; }
		float posX() { return this->_model.getElement(0, 3); }
		float posY() { return this->_model.getElement(1, 3); }

		void setModel(Matrix4 m) { _model = m; }
		void setRadius(float r) { _radius = r; }
		void setDirection(Vector2 d) { _direction = d; }
		void setSpeed(Vector2 s) { _speed = s; }
		void setAcceleration(Vector2 a) { _acceleration = a; }
		void setMass(float m) { _mass = m; }

		//AABB overlaps method for balls
		friend bool overlaps(Object *o1, Object *o2) {
			if (o1->model().getElement(0,3) + o1->radius() + o2->radius() > o2->model().getElement(0, 3)
				&& o1->model().getElement(0, 3) < o1->radius() + o2->radius() + o2->model().getElement(0, 3)
				&& o1->model().getElement(1, 3) + o1->radius() + o2->radius() > o2->model().getElement(1, 3)
				&& o1->model().getElement(1, 3) < o1->radius() + o2->radius() + o2->model().getElement(1, 3))
				return true;
			return false;
		}

		//Check if balls are colliding
		friend bool colliding(Object *o1, Object *o2) {
			float v1x = o1->model().getElement(0, 3);
			float v1y = o1->model().getElement(1, 3);

			float v2x = o2->model().getElement(0, 3);
			float v2y = o2->model().getElement(1, 3);

			Vector2 collisionv1 = Vector2(v1x, v1y);
			Vector2 collisionv2 = Vector2(v2x,v2y);

			Vector2 collision = collisionv1 - collisionv2;
			float distance = collision.norm();

			//float distance = sqrt(pow(o1->model().getElement(0, 3) - o2->model().getElement(0, 3), 2) + pow(o1->model().getElement(1, 3) - o2->model().getElement(1, 3), 2));
			if (distance < o1->radius() + o2->radius())
			{
				return true;
			}
			return false;
		}

		//Return collision x and y position
		friend Vector2 collisionPoints(Object* o1, Object *o2) {
			float collisionPointX = ((o1->model().getElement(0, 3)*o2->radius()) + o2->model().getElement(0, 3)*o1->radius()) / (o1->radius() + o2->radius());
			float collisionPointY = ((o1->model().getElement(1, 3)*o2->radius()) + o2->model().getElement(1, 3)*o1->radius()) / (o1->radius() + o2->radius());
			return { collisionPointX, collisionPointY };
		}

		//Buggy collision
		friend void collide(Object* o1, Object* o2) {

			float v1x = o1->posX();
			float v1y = o1->posY();

			float v2x = o2->posX();
			float v2y = o2->posY();

			float radiusv1X = v1x - v2x < 0 ? o1->radius() : -o1->radius();
			float radiusv1Y = v1y - v2y < 0 ? o1->radius() : -o1->radius();

			Vector2 collisionv1 = Vector2(v1x, v1y);
			Vector2 collisionv2 = Vector2(v2x, v2y);

			Vector2 collision = collisionv1 - collisionv2;
			float distance = collision.norm();
			collision = collision / distance;

			float aci = dot(o1->speed(), collision);
			float bci = dot(o2->speed(), collision);

			// Solve for the new velocities using the 1-dimensional elastic collision equations.
			// Turns out it's really simple when the masses are the same.
			float acf = bci;
			float bcf = aci;

			// Replace the collision velocity components with the new ones
			o1->setSpeed(o1->speed() + (collision * (acf - aci)));
			o2->setSpeed(o2->speed() + (collision * (bcf - bci)));

			o1->setModel(o1->model() * Create4DTranslation(o1->speed().x - radiusv1X, o1->speed().y - radiusv1Y, 0));
			o2->setModel(o2->model() * Create4DTranslation(o2->speed().x + radiusv1X, o2->speed().y + radiusv1Y, 0));
		}

		void updatePosition(int timeElapsed)
		{
			_speed = _speed + _acceleration;
			_model = _model * Create4DTranslation(_speed.x, _speed.y, 0);

			_speed.x = _speed.x > 0 ? _speed.x - friction : _speed.x + friction;
			_speed.y = _speed.y > 0 ? _speed.y - friction : _speed.y + friction;

			_acceleration.x = _acceleration.x > 0 ? _acceleration.x - friction : _acceleration.x + friction;
			_acceleration.y = _acceleration.y > 0 ? _acceleration.y - friction : _acceleration.y + friction;

			if (fabs(_speed.x) < 0.001)
				_speed.x = 0;

			if (fabs(_speed.y) < 0.001)
				_speed.y = 0;

			if (fabs(_acceleration.x) < 0.001)
				_acceleration.x = 0;

			if (fabs(_acceleration.y) < 0.001)
				_acceleration.y = 0;

			Quaternion rotationQtrnY = Quaternion(_speed.x * 100, math::Vector4(0.0f, 1.0f, 0.0f, 1.0f));
			Quaternion rotationQtrnX = Quaternion(-_speed.y * 100, math::Vector4(1.0f, 0.0f, 0.0f, 1.0f));

			rotationQuaternion = rotationQtrnX * rotationQtrnY * rotationQuaternion;
		}
	};
}

#endif;