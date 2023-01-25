#ifndef __POINT_LIGHT__
#define __POINT_LIGHT__

#include "SphereLight.hpp"

//Although it's a point light, we want to display it as a sphere with radius SPHERE_RADIUS and alpha < 1 (bit transparant) in the 3D view
#define SPHERE_RADIUS 0.04f
#define POINT_ALPHA 0.5f

class PointLight : public SphereLight {
public:
	PointLight(Eigen::Vector3f _position, Eigen::Vector3f _color) : SphereLight(_position, _color, 0) {
		this->setShapeAttributes(SPHERE_RADIUS, POINT_ALPHA);
	}
};

#endif // POINT_LIGHT