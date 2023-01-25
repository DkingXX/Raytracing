#ifndef __SPHERE_LIGHT__
#define __SPHERE_LIGHT__

#include <Eigen/Dense>
#include <cmath>
#include <tucano/shapes/cylinder.hpp>
#include <tucano/shapes/sphere.hpp>
#include <tucano/shapes/box.hpp>

// Let the number of sampling points depend on the radius (larger sphere light -> more samples)
#define SAMPLING_POINTS_PER_RADIUS 80
// A sphere light won't be displayed transparant (alpha = 1) 
#define SPHERE_ALPHA 1.f

class SphereLight {
private:
	Eigen::Vector3f position;
	Eigen::Vector3f color;
	float radius;
	Tucano::Shapes::Sphere s;
	vector<Eigen::Vector3f> samplingPoints;
public:
	SphereLight() {}

	SphereLight(Eigen::Vector3f _position, Eigen::Vector3f _color, float _radius)
	{
		position = _position;
		radius = _radius;
		color = _color;
		s = Tucano::Shapes::Sphere(radius, 4);
		s.setColor(Eigen::Vector4f(color.x(), color.y(), color.z(), SPHERE_ALPHA));
		samplingPoints = this->generateSamplingPoints(max(radius * SAMPLING_POINTS_PER_RADIUS, 1.f));
	}

	Eigen::Vector3f getLightPosition() { return position; }
	Eigen::Vector3f getLightColor() { return color; }
	Tucano::Shapes::Sphere getShape() { return s; }
	vector<Eigen::Vector3f> getSamplingPoints() { return samplingPoints; }

	/*
	Allows the change the attributes of the sphere that is represting this light in the 3D scene
	Mainly used in PointLight.hpp for creating a PointLight with radius 0, while still be able to render it
	*/
	void setShapeAttributes(float radius, float alpha) {
		s.setSize(radius);
		s.setColor(Eigen::Vector4f(color.x(), color.y(), color.z(), alpha));
	}

	/*
	We want to have randomly generated points inside the sphere light so we can
	trace a ray to each point to calculate soft shadows
	*/
	vector<Eigen::Vector3f> generateSamplingPoints(int samples) {
		vector<Eigen::Vector3f> result;

		// Simulate a point light
		if (samples == 1) {
			result.push_back(this->position);
			return result;
		}
		float rX;
		float rY;
		float rZ;
		for (int i = 0; i < samples; i++) {
			// Generate a random float between 0 and 1
			rX = ((float)rand()) / (float)RAND_MAX; 
			rY = ((float)rand()) / (float)RAND_MAX;
			rZ = ((float)rand()) / (float)RAND_MAX;
			// Transform to a random number between -1 and 1
			rX = (rX * 2) - 1; 
			rY = (rY * 2) - 1;
			rZ = (rZ * 2) - 1;
			// Create a random unit vector
			Eigen::Vector3f rVector = Eigen::Vector3f(rX, rY, rZ).normalized();
			// Random point on sphere surface
			Eigen::Vector3f point = this->position + this->radius * rVector;
			result.push_back(point);
		}
		return result;
	}
};

#endif // SPHERE_LIGHT