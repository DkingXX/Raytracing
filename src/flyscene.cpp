#include "flyscene.hpp"
#include <GLFW/glfw3.h>
#include <ppl.h>
#include <chrono>
#include <thread>

void Flyscene::initialize(int width, int height) {
  // initiliaze the Phong Shading effect for the Opengl Previewer
  phong.initialize();
 
  // set the camera's projection matrix
  flycamera.setPerspectiveMatrix(60.0, width / (float)height, 0.1f, 100.0f);
  flycamera.setViewport(Eigen::Vector2f((float)width, (float)height));

  // load the OBJ file and materials
  Tucano::MeshImporter::loadObjFile(mesh, materials,
                                    "resources/models/scene5.obj");

  // normalize the model (scale to unit cube and center at origin)
  mesh.normalizeModelMatrix();
   
  // pass all the materials to the Phong Shader
  for (int i = 0; i < materials.size(); ++i)
    phong.addMaterial(materials[i]);

  // scale the camera representation (frustum) for the ray debug
  camerarep.shapeMatrix()->scale(0.2);

  glEnable(GL_DEPTH_TEST);
  
  // Used to display semi-transparent colors
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  // Create first random light 
  lights.push_back(SphereLight(Eigen::Vector3f(0.0, 3.0, 0.0), Eigen::Vector3f(0.7, 0.7, 0.7), 0.3f));

  // Create little spheres for sampling points to be used for soft shadowing
  for (int i = 0; i < lights.size(); i++) {
	  vector<Eigen::Vector3f> sp = lights[i].getSamplingPoints();
	  for (int j = 0; j < sp.size(); j++) {
		  samplingPoints.push_back(SphereLight(sp[j], Eigen::Vector3f(0.1, 0.1, 0.1), 0.01f));
	  }
  }
#ifdef ACCEL_STRUCTURE
  // Create acceleration structure
  as = AccelerationStructure(mesh, MAX_FACES_PER_BOX, MAX_OVERLAP);
#endif
}

void Flyscene::paintGL(void) {
  // update the camera view matrix with the last mouse interactions
  flycamera.updateViewMatrix();
  Eigen::Vector4f viewport = flycamera.getViewport();

  // clear the screen and set background color
  glClearColor(backgroundColor.x(), backgroundColor.y(), backgroundColor.z(), 0.0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // position the scene light at the last ray-tracing light source
  scene_light.resetViewMatrix();
  if (!lights.empty())
	scene_light.viewMatrix()->translate(-lights.back().getLightPosition());
  // else just use random light position to still show some shading
  else
	scene_light.viewMatrix()->translate(Eigen::Vector3f(0.0, 0.0, 1.0));

  // render the scene using OpenGL and one light source
  phong.render(mesh, flycamera, scene_light);

  // render debug rays
  for (int i = 0; i < debugRays.size(); i++) {
	  debugRays[i].render(flycamera, scene_light);
  }

  // render normalrays
  for (int i = 0; i < normalRays.size(); i++) {
	  normalRays[i].render(flycamera, scene_light);
  }

  // render debug rays to light
  for (int i = 0; i < lightDebugRays.size(); i++) {
	  lightDebugRays[i].render(flycamera, scene_light);
  }

  // render bounding Box
  if (displayBoundingBoxes) {
	  vector<Tucano::Shapes::Box> aabb = as.getBoxMesh();
	  for (int i = 0; i < aabb.size(); i++) {
		  aabb[i].render(flycamera, scene_light);
	  }
  }

  // render sampling points
  for (int i = 0; i < samplingPoints.size(); i++) {
	  Tucano::Shapes::Sphere l = samplingPoints[i].getShape();
	  l.resetModelMatrix();
	  l.modelMatrix()->translate(samplingPoints[i].getLightPosition());
	  l.render(flycamera, scene_light);
  }

  // render lights
  for (int i = 0; i < lights.size(); i++) {
	  Tucano::Shapes::Sphere l = lights[i].getShape();
	  l.resetModelMatrix();
	  l.modelMatrix()->translate(lights[i].getLightPosition());
	  l.render(flycamera, scene_light);
  }

  // render coordinate system at lower right corner
  flycamera.renderAtCorner();
}

void Flyscene::simulate(GLFWwindow *window) {
  // Update the camera.
  // NOTE(mickvangelderen): GLFW 3.2 has a problem on ubuntu where some key
  // events are repeated: https://github.com/glfw/glfw/issues/747. Sucks.
  float dx = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ? 1.0 : 0.0) -
             (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ? 1.0 : 0.0);
  float dy = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS ||
                      glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS
                  ? 1.0
                  : 0.0) -
             (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS ||
                      glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS
                  ? 1.0
                  : 0.0);
  float dz = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ? 1.0 : 0.0) -
             (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ? 1.0 : 0.0);
  flycamera.translate(dx, dy, dz);
}

void Flyscene::addLight(void) {
	std::string input;
	int choice;
	float red, green, blue, radius;

	try {
		std::cout << "\nPoint (0) or Spherical (1) light: ";
		std::cin >> input;
		int choice = std::stoi(input);
		if (choice != 0 && choice != 1) throw std::invalid_argument("");

		float red, green, blue;
		Eigen::Vector3f color = askRGB(red, green, blue);

		SphereLight l;
		if (choice == 1) {
			std::cout << "Sphere radius (enter a number > 0): ";
			std::cin >> input;
			radius = std::stof(input);
			if (radius <= 0) throw std::invalid_argument("");
			l = SphereLight(flycamera.getCenter(), color, radius);
		}
		else {
			l = PointLight(flycamera.getCenter(), color);
		}

		lights.push_back(l);
		vector<Eigen::Vector3f> sp = l.getSamplingPoints();
		for (int j = 0; j < sp.size(); j++) {
			samplingPoints.push_back(SphereLight(sp[j], Eigen::Vector3f(0.1, 0.1, 0.1), 0.01f));
		}
	}
	catch (std::invalid_argument const& e) {
		std::cout << "Invalid input! Please follow the instructions" << std::endl;
	}
}

void Flyscene::clearLights() {
	lights.clear();
	samplingPoints.clear();
	lightDebugRays.clear();
}

void Flyscene::changeBackground(void) {
	float red, green, blue;
	std::cout << "\n";
	try {
		backgroundColor = askRGB(red, green, blue);
	}
	catch (std::invalid_argument const& e) {
		std::cout << "Invalid input! Please enter 3 numbers between 0 and 1" << std::endl;
	}
}

void Flyscene::createDebugRay(const Eigen::Vector2f& mouse_pos) {
	Eigen::Vector3f origin = flycamera.getCenter();
	Eigen::Vector3f dest = flycamera.screenToWorld(mouse_pos);
	displayBoundingBoxes = false;
	debugRays.clear();
	normalRays.clear();
	lightDebugRays.clear();
	traceRay(origin, dest, 0, true);
}

void Flyscene::highlightRay() {
	try {
		if (debugRays.empty()) {
			std::cout << "Please shoot a debug ray first" << std::endl;
			return;
		}

		std::string input;
		std::cout << "\nEnter a number n to highlight nth reflection: ";
		std::cin >> input;
		int newN = std::stoi(input);
		if (newN >= 0 && newN < debugRays.size()) {
			if (n >= 0) {
				debugRays[n].setColor(lastColor);
				normalRays[n].setColor(Eigen::Vector4f(1.0, 1.0, 1.0, 1.0));
			}
			if (n != newN) {
				n = newN;
				lastColor = debugRays[n].getColor();
				debugRays[n].setColor(Eigen::Vector4f(1.0, 1.0, 0.1, 1));
				normalRays[n].setColor(Eigen::Vector4f(1.0, 0.6, 0.0, 1));
			}
			
		}
		else throw std::invalid_argument("");
	}
	catch (std::invalid_argument const& e) {
		std::cout << "Invalid input! Please enter a 0 <= n <= " + to_string(debugRays.size() - 1) << std::endl;
	}
}

void Flyscene::raytraceScene(int width, int height) {
  std::cout << "<RAY TRACING STARTED>" << std::endl;

  // if no width or height passed, use dimensions of current viewport
  Eigen::Vector2i image_size(width, height);
  if (width == 0 || height == 0) {
    image_size = flycamera.getViewportSize();
  }

  // create 2d vector to hold pixel colors and resize to match image size
  vector<vector<Eigen::Vector3f>> pixel_data;
  pixel_data.resize(image_size[1]);
  for (int i = 0; i < image_size[1]; ++i)
    pixel_data[i].resize(image_size[0]);

  // origin of the ray is always the camera center
  Eigen::Vector3f origin = flycamera.getCenter();
  Eigen::Vector3f screen_coords;


#ifdef MULTITHREADING

  int overallCores = std::thread::hardware_concurrency();
  int parallelElements = overallCores - 1;
  std::thread* threads = new std::thread[parallelElements];

  int coreChunk = int(image_size[1] / parallelElements);
  int stop = parallelElements * coreChunk;
  int spare_rows = image_size[1] - stop;

  //j might not be the most appropriate name 
  int j = 0;
  while (j < image_size[1]) {

	  if (j == stop) overallCores = spare_rows + 1;

	  for (int index = 0; index < parallelElements; ++index) {
		  threads[index] = std::thread([&image_size, j, &origin, this, &pixel_data] {

			  for (int i = 0; i < image_size[0]; ++i) {
				  Eigen::Vector3f screen_coords = flycamera.screenToWorld(Eigen::Vector2f(i, j));
				  pixel_data[i][j] = traceRay(origin, screen_coords, 0, false);
			  }
			  });

		  ++j;
	  }
	  for (int i = 0; i < parallelElements; ++i)
		  threads[i].join();
  }
#else // MULTITHREADING

  clock_t timeStart = clock();
  int progress = 0;
  for (int i = 0; i < image_size[1]; ++i) {
	  for (int j = 0; j < image_size[0]; ++j) {
		  screen_coords = flycamera.screenToWorld(Eigen::Vector2f(i, j));
		  pixel_data[i][j] = traceRay(origin, screen_coords, 0, false);
	  }
	  int newProgress = ((i * 100) / image_size[1]);
	  if (newProgress > progress) std::cout << "RayTracing: " << (progress = newProgress) << "%\r";
	  std::cout.flush();
  }
  clock_t timeEnd = clock();

  std::cout << "RayTracing: 100% | Trace time: " << (float)((timeEnd - timeStart) / CLOCKS_PER_SEC) << " seconds" << std::endl;

#endif
  // write the ray tracing result to a PPM image
#ifdef SUPERSAMPLING
  pixel_data = superSampling(pixel_data);
#endif // SUPERSAMPLING

  Tucano::ImageImporter::writePPMImage("result.ppm", pixel_data);
  std::cout << "<RAY TRACING DONE>"<< std::endl;
}

Eigen::Vector3f Flyscene::traceRay(Eigen::Vector3f &origin, Eigen::Vector3f &dest, const int &depth, bool isDebug) {
	// we limit the amount of bounces the reflected ray can do
	if (depth > MAX_RECURSIVE_DEPTH) return backgroundColor;

	Eigen::Vector3f rayDirection = dest - origin;
	rayDirection.normalize();
	
#ifdef ACCEL_STRUCTURE
	vector<int> faces = as.intersectAccelStruct(rayDirection, origin);
	// if the ray did not intersect with a (non-empty) bounding box, return backgroundcolor
	if (faces.empty()) {
		if (isDebug)
			addDebugRay(origin, Eigen::Vector3f(0.0, 0.0, 0.0), rayDirection, Eigen::Vector4f(0.0, 0.0, 0.0, 1.0));
		return backgroundColor;
	}
#endif 

	int index = -1;													
	float minDistance = FLT_MAX;
	Eigen::Vector3f intersectionPoint;

#ifdef ACCEL_STRUCTURE
	for (int i = 0; i < faces.size(); ++i) {
		Tucano::Face face = mesh.getFace(faces[i]);
#else
	for (int i = 0; i < mesh.getNumberOfFaces(); ++i) {
		Tucano::Face face = mesh.getFace(i);
#endif 
		
		float D;
		Eigen::Vector3f point;
		
		/* This method will first check whether the ray intersects with the plane created from the triangle,
		then check whether D < minDistance to avoid unnecessary computations,
		and if so return whether the point on the plane lies inside the triangle */
		if (intersectTriangleNearest(face, rayDirection, origin, point, minDistance, D)) {
			minDistance = D;
			index = i;
			intersectionPoint = point;
		}
	}

	// index >= 0 means we hitted a face so calculate shading and show red debug ray
	if (index >= 0) {

#ifdef ACCEL_STRUCTURE
		index = faces[index];
#endif 

		if (isDebug) {
			// reflected ray
			addDebugRay(origin, intersectionPoint, rayDirection, Eigen::Vector4f(1.0, 0.0, 0.0, 1.0));
			// surface normal
			addDebugRay(intersectionPoint, intersectionPoint, mesh.getFace(index).normal.normalized(), Eigen::Vector4f(0.0, 0.0, 0.0, 0.0));
		}

		return calculateShading(mesh.getFace(index), intersectionPoint, rayDirection, depth, isDebug);
	}
	// otherwise return backgroundcolor and show black, infinite, debug ray
	else {
		if (isDebug) 
			addDebugRay(origin, intersectionPoint, rayDirection, Eigen::Vector4f(0.0, 0.0, 0.0, 1.0));

		return backgroundColor;
	}
}

/*
Calculate the shading for a given face
*/
Eigen::Vector3f Flyscene::calculateShading(Tucano::Face face, Eigen::Vector3f point, Eigen::Vector3f rayDirection, int depth, bool isDebug) {
	return calculateDirectLight(face, point, rayDirection, isDebug) + calculateReflectedLight(face, point, rayDirection, depth, isDebug);
}

/*
Calculate the direct light for a face 
*/
Eigen::Vector3f Flyscene::calculateDirectLight(Tucano::Face face, Eigen::Vector3f point, Eigen::Vector3f rayDirection, bool isDebug) {
	int material_id = face.material_id;
	Tucano::Material::Mtl mat = materials[material_id];
	Eigen::Vector3f result = mat.getAmbient();

	Eigen::Vector3f normal = face.normal;
	normal.normalize();

	for (int i = 0; i < lights.size(); i++) {
		float shadowFactor = 0.0f;
		SphereLight l = lights[i];
		vector<Eigen::Vector3f> points = l.getSamplingPoints();

		for (int j = 0; j < points.size(); j++) {
			Eigen::Vector3f lightPosition = points[j];
			Eigen::Vector3f lightRayDirection = lightPosition - point;
			float pointLightDistance = lightRayDirection.norm();
			lightRayDirection.normalize();
			if (!inShadow(point, normal, lightRayDirection, pointLightDistance)) 
				shadowFactor += (1.0f / ((float) points.size()));
			if (isDebug) 
				addDebugRay(point, lightPosition, lightRayDirection, Eigen::Vector4f(0.0, 1.0, 0.0, 1.0), true);
		}

		Eigen::Vector3f lightPosition = l.getLightPosition();
		Eigen::Vector3f lightRayDirection = lightPosition - point;
		lightRayDirection.normalize();
		Eigen::Vector3f lightRayReflection = reflect(-lightRayDirection, normal);
		Eigen::Vector3f diffuse = (mat.getDiffuse() * max(lightRayDirection.dot(normal), 0.0f));
		Eigen::Vector3f specular = (mat.getSpecular() * max(pow(lightRayReflection.dot(rayDirection), mat.getShininess()), 0.0f));
		result += componentWiseMultiplication(diffuse + specular, l.getLightColor()) * shadowFactor;
	}
	return result;
}

/*
Calculate reflected light for a face
*/
Eigen::Vector3f Flyscene::calculateReflectedLight(Tucano::Face face, Eigen::Vector3f point, Eigen::Vector3f rayDirection, int depth, bool isDebug) {
	Tucano::Material::Mtl mat = materials[face.material_id];
	// Material type 3: Reflection on and Ray trace on
	// Source http://paulbourke.net/dataformats/mtl/
	if (mat.getIlluminationModel() == 3) {
		Eigen::Vector3f reflectedRay = reflect(rayDirection, face.normal);
		Eigen::Vector3f dest = reflectedRay + point;
		return componentWiseMultiplication(traceRay(point, dest, depth + 1, isDebug), mat.getSpecular());
	}
	else
		return Eigen::Vector3f(0.0, 0.0, 0.0);
}

/*
Decide whether a point is in shadow (true) or lit (false)
*/
bool Flyscene::inShadow(Eigen::Vector3f point, Eigen::Vector3f normal, Eigen::Vector3f lightRayDirection, float pointLightDistance) {
	// check if the face is a back face w.r.t. to light direction
	// we check for < 0 -> angle between 90 and -90 
	// on the right side of the unitary circle
	if (normal.dot(lightRayDirection) < 0) return true;

	// depth bias, to prevent self collision detection
	float epsilon = 0.00001;
	Eigen::Vector3f lightRayOrigin = point + epsilon * lightRayDirection;

	//Check whether there is an object that intersects with the lightRay (within the given distance, otherwise it is behind the light)
#ifdef ACCEL_STRUCTURE
	vector<int> faces = as.intersectAccelStruct(lightRayDirection, lightRayOrigin);
	for (int i = 0; i < faces.size(); ++i) {
		Tucano::Face otherFace = mesh.getFace(faces[i]);
#else
	for (int i = 0; i < mesh.getNumberOfFaces(); ++i) {
		Tucano::Face otherFace = mesh.getFace(i);
#endif
		float D;
		Eigen::Vector3f point2;
		if (intersectTriangleNearest(otherFace, lightRayDirection, lightRayOrigin, point2, pointLightDistance, D)) return true;
	}
	return false;
}

/*
Check whether a ray intersects with the plane laying onto the face
Also calculates v0, D and t, which can be used in other methods
*/
bool Flyscene::intersectPlane(Tucano::Face face, Eigen::Vector3f rayDirection, Eigen::Vector3f origin, Eigen::Vector3f& v0, float& D, float& t) {
	// normal is in opposite direction, multiply by -1
	Eigen::Vector3f normal = -face.normal;
	normal.normalize();
	
	// if denominator is zero the ray is parallel to the plane, use <= instead of != so we also check whether we are on the right side of the face
	float denominator = normal.dot(rayDirection);
	if (denominator <= 0.0f) return false;
	
	// calculate the distance and the angle between the ray and the normal of the plane, using one of the vertices
	v0 = (mesh.getShapeModelMatrix() * mesh.getVertex(face.vertex_ids[0])).head<3>();
	D = normal.dot(v0);
	t = (D - normal.dot(origin)) / denominator;

	return t > 0;
}

/*
Check whether the ray intersects with the given triangle AND the triangle is maximum minDistance away
Also calculates D and (the intersection) point, which can be used in other methods
*/
bool Flyscene::intersectTriangleNearest(Tucano::Face triangle, Eigen::Vector3f rayDirection, Eigen::Vector3f origin, Eigen::Vector3f& point, float maxDistance, float& D) {
	float t;
	Eigen::Vector3f v0;

	if (intersectPlane(triangle, rayDirection, origin, v0, D, t)) {
		if (D < maxDistance) {
			point = origin + t * rayDirection;
			Eigen::Vector3f v1 = (mesh.getShapeModelMatrix() * mesh.getVertex(triangle.vertex_ids[1])).head<3>();
			Eigen::Vector3f v2 = (mesh.getShapeModelMatrix() * mesh.getVertex(triangle.vertex_ids[2])).head<3>();
			return pointInTriangle(v0, v1, v2, point);
		}
	}
	return false;
}

/*
Check whether the ray intersects with the given triangle
*/
bool Flyscene::intersectTriangle(Tucano::Face triangle, Eigen::Vector3f rayDirection, Eigen::Vector3f origin) {
	float D, t;
	Eigen::Vector3f normal, v0;

	if (intersectPlane(triangle, rayDirection, origin, v0, D, t)) {
		Eigen::Vector3f point = origin + t * rayDirection;
		Eigen::Vector3f v1 = (mesh.getShapeModelMatrix() * mesh.getVertex(triangle.vertex_ids[1])).head<3>();
		Eigen::Vector3f v2 = (mesh.getShapeModelMatrix() * mesh.getVertex(triangle.vertex_ids[2])).head<3>();
		return pointInTriangle(v0, v1, v2, point);
	}
	return false;
}

/*
 Check whether a point lies inside the triangle formed by 3 vertices
 To find if a point lies inside the triangle ABC we can rewrite the point in the form:
 P = A + r * (C - A) + t * (B - A)
 The point P is inside the triangle if 0 <= r <= 1 and 0 <= r <= 1 and r + t <= 1
*/
bool Flyscene::pointInTriangle(Eigen::Vector3f v0, Eigen::Vector3f v1, Eigen::Vector3f v2, Eigen::Vector3f p) {
	Eigen::Vector3f  u = v1 - v0;
	Eigen::Vector3f  v = v2 - v0;
	Eigen::Vector3f  w = p - v0;

	Eigen::Vector3f vCw = v.cross(w);
	Eigen::Vector3f vCu = v.cross(u);
	// r is in proportion to vCw.dot(vCu), so we can already check if r is positive 
	// without already having to calculate r (saves computation)
	if (vCw.dot(vCu) < 0) return false;

	Eigen::Vector3f uCw = u.cross(w);
	Eigen::Vector3f uCv = u.cross(v);
	if (uCw.dot(uCv) < 0) return false;

	float denominator = uCv.norm();
	float r = vCw.norm() / denominator;
	float t = uCw.norm() / denominator;

	return (r + t <= 1);
}

/*
Check whether a ray intersects with the given box
*/
bool Flyscene::intersectBox(Box box, Eigen::Vector3f rayDirection, Eigen::Vector3f origin) {
	return box.intersect(rayDirection, origin);
}

/*
Reflect vector A around vector B
*/
Eigen::Vector3f Flyscene::reflect(Eigen::Vector3f A, Eigen::Vector3f B) {
	return A - 2.f * A.dot(B) * B;
}

/*
Vector * Vector component wise multiplication
Method from eigen caused error in some cases, so we just created our own one
*/
Eigen::Vector3f Flyscene::componentWiseMultiplication(Eigen::Vector3f A, Eigen::Vector3f B) {
	return Eigen::Vector3f(A.x() * B.x(), A.y() * B.y(), A.z() * B.z());
}

/*
Method used to compute anti-aliasing by averaging a 2x2 pixel block into a single pixel
Generate smoother image but cut the resolution in half.
*/
vector<vector<Eigen::Vector3f>> Flyscene::superSampling(vector<vector<Eigen::Vector3f>> pixel_data) {
	int newWidth = pixel_data[0].size()/2;
	int newheight = pixel_data[1].size()/2;
	vector<vector<Eigen::Vector3f>> output;

	output.resize(newheight);
	for (int i = 0; i < newheight; ++i) 
		output[i].resize(newWidth);

	for (int i = 0; i < newWidth; i++) {
		for (int j = 0; j < newheight; j++) {
			output[i][j] = (pixel_data[(i * 2)][(j * 2)] + pixel_data[(i * 2)][(j * 2) + 1] 
				+ pixel_data[(i * 2) + 1][(j * 2)] + pixel_data[(i * 2) + 1][(j * 2) + 1]) / 4;
		}
	}
	return output;
}

/*
Generate and add a debug ray to the debugRay vector
*/
void Flyscene::addDebugRay(Eigen::Vector3f origin, Eigen::Vector3f destination, Eigen::Vector3f direction, Eigen::Vector4f color, bool toLight) {
	float length;
	bool isNormal = false;

	if (destination.norm() == 0) 
		length = 10.0f;
	else if (color.norm() == 0) {
		length = 0.3f;
		color = Eigen::Vector4f(1, 1, 1, 1);
		isNormal = true;
	}
	else 
		length = sqrt(pow((origin.x() - destination.x()), 2) + pow((origin.y() - destination.y()), 2) + pow((origin.z() - destination.z()), 2));
	
	Tucano::Shapes::Cylinder ray = Tucano::Shapes::Cylinder(0.1, 1.0, 16, 64);
	ray.setColor(color);
	ray.setSize(0.005, length);
	ray.setOriginOrientation(origin, direction);
	if (isNormal)
		normalRays.push_back(ray);
	else if (toLight)
		lightDebugRays.push_back(ray);
	else
		debugRays.push_back(ray);
}

/*
Ask the user the enter RGB color values
*/
Eigen::Vector3f Flyscene::askRGB(float& red, float& green, float& blue) {
	std::string input;
	
	std::cout << "Choose RGB colour values (0 -> 1)" << std::endl;
	std::cout << "Red: ";
	std::cin >> input;
	red = std::stof(input);
	if (red < 0 || red > 1) throw std::invalid_argument("");
	std::cout << "Green: ";
	std::cin >> input;
	green = std::stof(input);
	if (green < 0 || green > 1) throw std::invalid_argument("");
	std::cout << "Blue: ";
	std::cin >> input;
	blue = std::stof(input);
	if (blue < 0 || blue > 1) throw std::invalid_argument("");
	
	return Eigen::Vector3f(red, green, blue);
}
