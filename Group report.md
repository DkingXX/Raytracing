## CSE2215 Computer Graphics - Group 46
|Student Number| Members | Email |
|:-:|:-|:-|

|| Çoban, Daglar | d.coban@student.tudelft.nl | <br>
|| Du, Xiangyu | x.du-1@student.tudelft.nl | <br>
|| Nulle, Thijs | t.j.nulle@student.tudelft.nl | <br>
|4960343| Numan, Tim | t.numan@student.tudelft.nl | <br>
|4888014| Rustici, Pietro | p.rustici@student.tudelft.nl | <br>
|| Streef, Sam | s.l.streef@student.tudelft.nl |<br>
### 17/10/2019 First meeting
###### FEATURES STATUS: (Bold **done**, Normal ongoing, Italic *to be done*)
- (1) Perform ray intersections with planes, triangles, and bounding boxes.
- (2) *Computate shading at the  rst impact point (di use and specular).*
- (3) *Perform recursive raytracing for re ections to simulate specular materials.*
- (4) *Calculate hard shadows from a point light.*
- (5) *Calculate soft shadows from a spherical light centered at a point light.*
- (6) *Show an interactive display inOpenGLof the 3D scene and a debug raytracer.*
- (7) *Implement a (simple) acceleration structure.*
- (8) *Show a scene created by the group, exported as a wavefront object (OBJ)and directly loaded into the application.*
###### MEETING SUMMARY:
- Eevryone is present.
- Everyone read the Project Manual before the meeting.
- We started to code together the first feature of the project (Perform ray intersection with planes, triangles, and bounding boxes).
- We agreed that everyone tries to implement on it's own the first feature so that everyone dig into the theory to understand the basic implementation of the raytracer. The deadline is going to be on Sunday, after that we will discuss the differences in the implementations and agree on the one to use for the group.
- We also divided the group in three 2-men subgroups and we divided the features to implement based on personal preference. 
###### (TODO) WHAT TASK ARE ASSIGNED TO WHO:
- Steven, Thijs: (2)
- Tim, Pietro: (4)
- Sam, Daglar: (3)
###### DEADLINES:
- First Feature: 13/10/2019
- Feature (2)/(3)/(4): 24/10/2019
###### SUGGESTION FROM TA:
- Create a variable for the scene color that we can modify from the GUI(?)
- Except for the first feature, rest are not dependent each others so we can split the work load into small groups working on different features.

### 22/10/2019 Second meeting
Worked on our features at the lab, helped each other and discussed some things we should take into account to make the integration between the individual parts easier.
Will discuss completed/yet to do tasks Thursday

### 24/10/2019 Third meeting
###### BASIC FEATURES STATUS: (Bold **done**, Normal ongoing, Italic *to be done*)
- (1) **Ray intersections (plane,tri,box)       COMPLETED** - Everyone
- (2) **Shading first impact point (diff+spec)      COMPLETED** - Thijs, Steven
- (3) Recursive raytracing (specular materials) - Daglar, Sam
- (4) **Hard Shadows point light        COMPLETED** - Pietro, Tim
- (5) **Hard Shadows spherical      COMPLETED** - Pietro, Tim
- (6) *Implement an interactive debug raytracer for the 3D scene* - Thijs, Steven
- (7) Implement a (simple) acceleration structure. - Daglar, Sam
- (8) *Show a scene created by the group, exported as a wavefront object (OBJ)and directly loaded into the application* - ???

###### ADVANCED FEATURES STATUS: (Bold **done**, Normal ongoing, Italic *to be done*)
- (9) *Multithreading* - ???
- (10) Interpolated normals to smooth object - Steven
- (11) *Support Soft Shadows for other light types* - ???
- (12) *Numerical evaluation of the ray tracer performance* - ???
- (13) Make variables (backgroundColor, lights + parameters) adjustable in GUI - Thijs, Steven

###### Questions for TAs:
Pietro:
- How to correclty sum up different lights contribution.
    - Just add them and clam 1
- How to correctly generate sample points inside a sphere.
    - Create random points on surface, different for each pixel (so not before raytracing)
- How the light color should mix with the material parameters.
    - Just multiply
- How is the presentation structured? do we have to make demo ppm images?
    - Explain all features we implemented, slides with pictures rendered beforehand
- What does numerical eveluation of performance mean?
    - Just table in report
- What does support for spherical objects mean?
    - Add (mathematical) sphere in main.cpp 
- Can we create the scene inside the FlyScene class or should we create it in blender and import it.
    - In Blender, allowed to download objects, but state this in report

Steven/Thijs:
- What should we do with interactive display and a debug ray tracer?
    - Debug ray tracer: Display how all bouncing rays after hitting the objects. Interactive display itself is already given, adding adjustable variables is for bonus
    

Tim:
- Why is multithreading giving artifacts?
    - Outer for loop is correct
- How should we implement soft shadows? There is no property for the ligths that says whether it is a point light or a spherical light, so we can't do different things for different lights
 Should we just create 2 shadow methods? One where we assume the light is a point light -> hard shadows & One where we assume the light is a spherical light centered at a point -> soft shadows
 Or can we make a 'light class' ourselven, with attributes position, type (, intensity?, color?) etc.? 
 (Right now we did the last)
    - With multiple classe (like we have it rn) is fine
- Should we implement something which is physically correct, so strictly follow a certain model like Phong, or can we do what we want for shading/shadow
    - Best to do Phong and when we want to change something, give arguments in report
- To fulfill the first requirement, do we literally need three methods intersectPlane, intersectTriangle, and intersectBoundingBox? We have those methods, but to save computation, some are checking more than just if there is an intersection
    - What we have is fine
- How many extra light sources should we support for the extra feature for soft shadows?
    - 1 will give extra points, 2 more etc. Other lights to support could be

### 30/10/2019 Fourth meeting and final lab
All basic features except 8 (own created scene) done, <br>acceleration structure still needs to be integrated though

##### Task division for last 2 days

- Steven
    - Show a scene created by the group, exported as a wavefront object (OBJ)
    <br> and directly loaded into the application [REQUIRED]
    - Implementing a more complex scene hierarchy
- Thijs
    - Make variables (backgroundColor, lights + parameters) adjustable in GUI
    - Done? -> Help Steven with more complex scene
- Daglar & Sam
    - Multithreading (maybe 1 more try, other library, somewhere else in the code?)
    - Implement 2nd, faster acceleration structure
- Pietro
    - Extending the debugger to show then nth reflection of a ray via the key-board, <br> or triggering a ray highlighting and showing command line output of <br>the selected ray's properties
    - A numerical evaluation of the performance of the ray tracer
    - Do final renderings for in presentation/report
    - Try updating the repo with the updated codebase
- Tim
    - Supporting soft shadows for other types of light sources
    - Do some refactoring / improve code quality
    - Write report

Other interesting bonus features (Daglar & Sam?, if above things don't work out): 
- 'Supporting refraction and the display of transparent objects.
    - 'Would be really cool, but no idea how much time it would take....
- 'Adding support for spherical objects'

##### During the lab
- Ask questions (see README) to TA
- Work out main idea for presentation + slides
- Work on final / bonus features

##### Model sources for generating our own scene
- Pot and lib: [https://free3d.com/3d-model/cooking-pot-33147.html]
- Vase: [https://free3d.com/3d-model/vase-90453.html]
- Pear: [https://free3d.com/3d-model/-pear--324071.html]
- Cup: [https://free3d.com/3d-model/glass-91748.html]