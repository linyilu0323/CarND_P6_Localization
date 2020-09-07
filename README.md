# Kidnapped Vehicle Project

### Project Description

Your robot has been kidnapped and transported to a new location! Luckily it has a map of this location, a (noisy) GPS estimate of its initial location, and lots of (noisy) sensor and control data.

In this project you will implement a 2 dimensional particle filter in C++. Your particle filter will be given a map and some initial localization information (analogous to what a GPS would provide). At each time step your filter will also get observation and control data.

### Project Rubric

The things the grading code is looking for are:


1. **Accuracy**: your particle filter should localize vehicle position and yaw to within the values specified in the parameters `max_translation_error` and `max_yaw_error` in `src/main.cpp`.

2. **Performance**: your particle filter should complete execution within the time of 100 seconds.

------

### 1. Code Walk-through

The directory structure of this repository is as follows:

```
root
|   build.sh
|   clean.sh
|   CMakeLists.txt
|   README.md
|   run.sh
|
|___data
|   |   
|   |   map_data.txt
|   
|   
|___src
    |   helper_functions.h
    |   main.cpp
    |   map.h
    |   particle_filter.cpp
    |   particle_filter.h
```

The only file you should modify is `particle_filter.cpp` in the `src` directory. The file contains the scaffolding of a `ParticleFilter` class and some associated methods. Read through the code, the comments, and the header file `particle_filter.h` to get a sense for what this code is expected to do.

**Brief overview of the sub-functions in `particle_filter.cpp`:**

1. `init` - Initializing the particle filter: in this section, the particles are being initialized with a specified noise and starting position (very rough GPS measurements).
2. `precition` - Utilize motion model to update the predicted position of the particles.
3. `updateWeights` - Update the weights of each particle: the steps for this include:
  - Get positions and heading of each particle;
  - Get predicted landmarks within sensor range of the particle;
  - Translate observations from vehicle coordinates to world coordinates (so that it's same as "predictions" of landmark);
  - Run data associations using nearest-neighbor method;
  - Calculate particle weight using multivariate Gaussian distribution.
4. `resample` - Resample particles with sampling probability proportional to their weights.

**A flow chart to visualize the process is shown below:**

![02-l-pseudocode.00-00-47-13.still006](/Users/yilulin/Documents/CarND-Kidnapped-Vehicle-Project/src/pf_flowchart.png)


### 2. Watch-outs and Best-practices

- **Use discrete distribution for resampling:** A simple and straight-forward method to do resampling with weights is to use `std::discrete_distribution`, refer to http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution for details.
- **Avoid division by zero:** as mentioned in lecture, the motion model with and without yaw_rate is going to be different - using generalized equation when yaw_rate is near zero can cause division-by-zero error. Make sure to have a if-else statement here to update predictions with the correct equation.
- **Implement Visualization:** the default version of `particle_filter.cpp` does not include a visualization of what landmarks the particles are sensing, this feature is helpful to bebugging the code. Refer to [Gregory D'Angelo's project](https://github.com/gdangelo/CarND-Kidnapped-Vehicle-Project/blob/master/src/particle_filter.cpp) as an example for how to achieve that.

### 3. Results and Discussion

Below is the results I got from the simulator, where the actual (ground truth) position of the vehicle is represented by the blue vehicle icon, and the estimated position as a result of particle filter is represented by the blue circle. The blue lines and green lines represent the "prediciton" and "observation" respectively, so if they are close to each other, we know that the particle filter is capturing the correct location and landmark association.

[![alt text](https://img.youtube.com/vi/VY7vrSsbhqw/0.jpg)](https://youtu.be/VY7vrSsbhqw)