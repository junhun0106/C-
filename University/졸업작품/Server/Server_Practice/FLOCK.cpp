#include "FLOCK.h"

/***********************************/
// VECTOR2D
/***********************************/
float VECTOR2D::Distance(VECTOR2D vec) {
	float delta_x = x_pos - vec.x_pos;
	float delta_z = z_pos - vec.z_pos;
	float final_dist = delta_x * delta_x + delta_z * delta_z;
	return sqrt(final_dist);
}
float VECTOR2D::Length() {
	return sqrt(x_pos * x_pos + z_pos * z_pos);
}
void VECTOR2D::limit(float max) {
	double length = this->Length();
	if (length > numeric_limits<double>::epsilon()) {
		x_pos /= length;
		z_pos /= length;
		x_pos *= max;
		z_pos *= max;
	}
}
void VECTOR2D::normalize() {
	double length = this->Length();
	if (length > numeric_limits<double>::epsilon()) {
		x_pos /= length;
		z_pos /= length;
	}
}
/***********************************/
// BOID
/***********************************/
BOID::BOID() {
	position = VECTOR2D(0.0f, 0.0f);
	velocity = VECTOR2D(0.0f, 0.0f);
	acceleration = VECTOR2D(0.0f, 0.0f);
	max_force = max_speed = 0.0f;
}
BOID::BOID(float x, float z) {
	position = VECTOR2D(x, z);
	velocity = VECTOR2D(0.0f, 0.0f);
	float angle = rand() % 360;
	velocity = VECTOR2D(cos(angle), sin(angle));
	max_force = 0.3f;
	max_speed = 10.0f;
}
VECTOR2D BOID::Seek(VECTOR2D target) {
	VECTOR2D desired = target.Sub(position);
	desired.normalize();
	desired.Multiple(max_speed);

	VECTOR2D steer = desired.Sub(velocity);
	steer.limit(max_force);
	return steer;
}
VECTOR2D BOID::Separate(const vector<BOID*> &boids) {
	float desiredseparate = 25.0f;
	VECTOR2D steer = VECTOR2D(0.0f, 0.0f);
	int count = 0;

	for (auto i = 0; i < boids.size(); ++i) {
		BOID* boid = boids[i];
		float dist = position.Distance(boid->position);
		if (dist > 0 && dist < desiredseparate) {
			VECTOR2D diff = position.Sub(boid->position);
			diff.normalize();
			diff.Divide(dist);
			steer.Add(diff.x_pos, diff.z_pos);
			count++;
		}
	}
	if (count > 0) steer.Divide(static_cast<float>(count));
	if (steer.Length() > 0) {
		steer.normalize();
		steer.Multiple(max_speed);
		steer.Sub(velocity.x_pos, velocity.z_pos);
		steer.limit(max_force);
	}
	return steer;
}
VECTOR2D BOID::Alignment(const vector<BOID*> &boids) {
	float neighbor_dist = 50.0f;
	VECTOR2D sum = VECTOR2D(0.0f, 0.0f);
	int count = 0;
	for (auto i = 0; i < boids.size(); ++i) {
		BOID* boid = boids[i];
		float dist = position.Distance(boid->position);
		if (dist > 0 && dist < neighbor_dist) {
			sum.Add(boid->velocity.x_pos , boid->velocity.z_pos);
			count++;
		}
	}
	if (count > 0) {
		sum.Divide(static_cast<float>(count));
		sum.normalize();
		sum.Multiple(max_speed);
		VECTOR2D steer = sum.Sub(velocity);
		steer.limit(max_force);
		return steer;
	}
	return VECTOR2D(0.0f, 0.0f);
}
VECTOR2D BOID::Cohesion(const vector<BOID*> &boids) {
	float neighbor_dist = 50.0f;
	VECTOR2D sum = VECTOR2D(0.0f, 0.0f);
	int count = 0; 

	for (auto i = 0; i < boids.size(); ++i) {
		BOID* boid = boids[i];
		float dist = position.Distance(boid->position);
		if (dist > 0 && dist < neighbor_dist) {
			sum.Add(boid->velocity.x_pos, boid->velocity.z_pos);
			count++;
		}
	}
	if (count > 0) {
		sum.Divide(static_cast<float>(count));
		return Seek(sum);
	}
	return VECTOR2D(0.0f, 0.0f);
}
void BOID::ApplyForce(VECTOR2D force) {
	acceleration.Add(force.x_pos, force.z_pos);
}
void BOID::Flock(const vector<BOID*> &boids) {
	VECTOR2D sep = Separate(boids);
	VECTOR2D ali = Alignment(boids);
	VECTOR2D coh = Cohesion(boids);

	sep.Multiple(1.5f);
	ali.Multiple(1.0f);
	coh.Multiple(1.0f);

	ApplyForce(sep);
	ApplyForce(ali);
	ApplyForce(coh);
}
void BOID::Update() {
	velocity.Add(acceleration.x_pos, acceleration.z_pos);
	velocity.limit(max_speed);
	position.Add(velocity.x_pos, velocity.z_pos);
	acceleration.Multiple(0.0f);
}
void BOID::Bordes() {
	if (position.x_pos < -5.0)
		position.x_pos = 1000 - 20;
	if (position.x_pos > 1000 + 20)
		position.x_pos = 20.0f;
	if (position.z_pos < -5.0)
		position.z_pos = 1000 - 20;
	if (position.z_pos > 1000 + 20)
		position.z_pos = 20;
}
void BOID::Run(const vector<BOID*> &boids) {
	Flock(boids);
	Update();
	Bordes();
}

/***********************************/
// FLOCKING
/***********************************/

