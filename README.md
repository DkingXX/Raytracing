# Raytracing Project

### Schedule
- Project is due on **31 October 2019 at 08:45**.
    - Upload source code, models, report and presentation slides. For more details see Project Description -> Deliverables
- The presentations of our group will take place on **1 November 2019, 14:25 - 14:35**.


### Questions for TA!
Tim:
- PROBLEMS WITH UPDATING CODEBASE!
- Do the changes for the codebase actually really affect the raytracer, as in will our code probably not run on their pc's when we don't update our repo with their codebase changes? Or is it only for the 3D view?
    - We don't have to update the codebase, just say in our report we tried, but got lots of errors, so used version 2 and
- How to know whether our implemenation of the (basic) features is 'correct'?
    - Doesn't have to be perfect, so what we have right now is fine
- 'Zip  the  source  code  for  your  ray  tracer,  containing  only  the  .cpp  and.h/.hpp  les, and name it groupnumber.zip.' This doesn't include the libraries right? 
Just the flyscene class and header file and the other classes we created? What if we modified something in Tucano?
-Should we calculate the diffuse and specular effects from the sampling points and add those (divided by the number of points) -> see commented code
Or should we calculate them from the ligth position (center of the sphere) and then multiply by the shadowfactor (fraction of the samples that could the point)
First one sounds better, but when comparing the images, strangly enough the last one seems a lot smoother
-What is good code quality in C++
-How will the presentation look like, what should we do?
    - ASK ON BRIGHTSPACE

Pietro:
- What do we exactly need to do for: 
'Extending the debugger to show the nth reflection of a ray via the key-board, or triggering a ray highlighting and showing command line outputof the selected ray's properties'
    - Show everything, highlight n-th ray
- What are the requirements for: 'A numerical evaluation of the performance of the ray tracer'
    - Not really clear, just add some numbers to report/slides
Steven:
- What are the requirements for the last required feature?
'Show a scene created by the group, exported as a wavefront object (OBJ)and directly loaded into the application.'
How complex should it be, or is A SCENE just enough.
    - No hard requirtements, just show all features for ray tracer
- What are the requirements for the first bounus feature?
'Implementing a more complex scene hierarchy'
    - No idea, I think we should just 'claim' or scene is complex

Sam/Daglar:
- Project description states acceleration structure should use flat array, but we use tree. 
Does this cover that? (So is it enough/even better)
    - Acceleration structure for flat array should be somewhere in the code, but if tree is faster we can just use that

Thijs:
- Sliders not working for Tucano GUI
    - ASK TO RICARDO
