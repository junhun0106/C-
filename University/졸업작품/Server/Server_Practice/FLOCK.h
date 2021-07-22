#pragma once

#include "stdafx.h"


class VECTOR2D {
public:
	float x_pos, z_pos;
public:
	VECTOR2D() { x_pos = z_pos = 0; }
	VECTOR2D(float x, float z) { x_pos = x, z_pos = z; }
	~VECTOR2D() {}

	void Multiple(float n) { x_pos *= n, z_pos *= n; }
	void Divide(float n) { x_pos /= n, z_pos /= n; }
	void Add(float x, float z) { x_pos += x, z_pos += z; }
	VECTOR2D Add(VECTOR2D vec) { return VECTOR2D(x_pos + vec.x_pos, z_pos + vec.z_pos); }
	void Sub(float x, float z) { x_pos -= x, z_pos -= z; }
	VECTOR2D Sub(VECTOR2D vec) { return VECTOR2D(x_pos - vec.x_pos, z_pos - vec.z_pos); }

	float Distance(VECTOR2D);
	float Length();
	
	void limit(float max);
	void normalize();
};

/************************************************/

class BOID {
	VECTOR2D position;
	VECTOR2D velocity;
	VECTOR2D acceleration;

	float max_force;
	float max_speed;
public:
	BOID();
	BOID(float x, float z);
	~BOID() {}

	VECTOR2D Seek(VECTOR2D target);
	VECTOR2D Separate(const vector<BOID*> &boids);
	VECTOR2D Alignment(const vector<BOID*> &boids);
	VECTOR2D Cohesion(const vector<BOID*> &boids);

	void ApplyForce(VECTOR2D force);
	void Flock(const vector<BOID*> &boids);
	void Update();
	void Run(const vector<BOID*> &boids);
	void Bordes();

	VECTOR2D GetPositon() { return position; }
	VECTOR2D GetVelocity() { return velocity; }
};

/************************************************/
class CFLOCK
{
	vector<BOID*> boids;
public:
	CFLOCK() { boids.clear(); }
	~CFLOCK() { boids.clear(); }
	void Run() {
		for (auto i = 0; i < boids.size(); ++i)
			boids[i]->Run(boids);
	}
	void Add(BOID* boid) {
		boids.push_back(boid);
	}

	vector<BOID*> GetBoid() { return boids; }
};
