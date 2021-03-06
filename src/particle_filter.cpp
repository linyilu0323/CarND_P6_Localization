/**
 * particle_filter.cpp
 *
 * Created on: Dec 12, 2016
 * Author: Tiffany Huang
 */

#include "particle_filter.h"

#include <math.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <cstddef>

#include "helper_functions.h"

using std::string;
using std::vector;
using std::size_t;

void ParticleFilter::init(double x, double y, double theta, double std[]) {
  /**
   * TODO: Set the number of particles. Initialize all particles to
   *   first position (based on estimates of x, y, theta and their uncertainties
   *   from GPS) and all weights to 1.
   * TODO: Add random Gaussian noise to each particle.
   * NOTE: Consult particle_filter.h for more information about this method
   *   (and others in this file).
   */
  num_particles = 100;  // TODO: Set the number of particles

  // declare a random engine to be used
  std::default_random_engine gen;

  // create normal distributions for x, y and theta
  std::normal_distribution<double> dist_x(x, std[0]);
  std::normal_distribution<double> dist_y(y, std[1]);
  std::normal_distribution<double> dist_theta(theta, std[2]);

  // create particles and initialize their position with randomized Gaussian noise
  for (size_t i = 0; i < num_particles; i++) {
    Particle p;
    p.id = i;
    p.x = dist_x(gen);
    p.y = dist_y(gen);
    p.theta = dist_theta(gen);
    p.weight = 1.0;

    particles.push_back(p);
  }

  is_initialized = true;
}

void ParticleFilter::prediction(double delta_t, double std_pos[],
                                double velocity, double yaw_rate) {
  /**
   * TODO: Add measurements to each particle and add random Gaussian noise.
   * NOTE: When adding noise you may find std::normal_distribution
   *   and std::default_random_engine useful.
   *  http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
   *  http://www.cplusplus.com/reference/random/default_random_engine/
   */

   // declare a random engine to be used
   std::default_random_engine gen;

   // create normal distributions for noise
   std::normal_distribution<double> dist_x(0, std_pos[0]);
   std::normal_distribution<double> dist_y(0, std_pos[1]);
   std::normal_distribution<double> dist_theta(0, std_pos[2]);

   // calculate new state with noise
   for (size_t i = 0; i < num_particles; i++) {
     // update position estimates
     if (fabs(yaw_rate) < 0.0001) {
       particles[i].x += velocity * delta_t * cos(particles[i].theta);
       particles[i].y += velocity * delta_t * sin(particles[i].theta);
     }
     else {
       particles[i].x += velocity / yaw_rate * (sin(particles[i].theta + yaw_rate * delta_t) - sin(particles[i].theta));
       particles[i].y += velocity / yaw_rate * (cos(particles[i].theta) - cos(particles[i].theta + yaw_rate * delta_t));
       particles[i].theta += yaw_rate * delta_t;
     }

     // add noise for each component
     particles[i].x += dist_x(gen);
     particles[i].y += dist_y(gen);
     particles[i].theta += dist_theta(gen);
   }
}

//void ParticleFilter::dataAssociation(vector<LandmarkObs> predicted,
//                                     vector<LandmarkObs>& observations) {

void ParticleFilter::dataAssociation(vector<LandmarkObs> predicted, vector<LandmarkObs>& observations, Particle &particle) {
  //below are for visualization
	std::vector<int> associations;
	std::vector<double> sense_x;
	std::vector<double> sense_y;

  /**
   * TODO: Find the predicted measurement that is closest to each
   *   observed measurement and assign the observed measurement to this
   *   particular landmark.
   * NOTE: this method will NOT be called by the grading code. But you will
   *   probably find it useful to implement this method and use it as a helper
   *   during the updateWeights phase.
   */
   for (size_t i = 0; i < observations.size(); i++) {

     LandmarkObs obs = observations[i];
     //below are for visualization
     double lm_x = observations[i].x;
     double lm_y = observations[i].y;


     // init some variables
     double min_distance = std::numeric_limits<double>::max();
     int index_min = -1;

     for (size_t j = 0; j < predicted.size(); j++) {
       // load each prediction and calculate the Euclidean distance
       LandmarkObs pred = predicted[j];
       double distance = dist(obs.x, obs.y, pred.x, pred.y);
       // update to find out the minimum distance and store the ID.
       if (distance < min_distance) {
         min_distance = distance;
         index_min = pred.id;
       }
     }

     // assign the landmark id of nearest distance to this observation
     observations[i].id = index_min;
     // below are for visualization only.
     associations.push_back(index_min);
		 sense_x.push_back(lm_x);
		 sense_y.push_back(lm_y);
   }

  // Set assocations for visualization purpose only
	particle = SetAssociations(particle, associations, sense_x, sense_y);
}

void ParticleFilter::updateWeights(double sensor_range, double std_landmark[],
                                   const vector<LandmarkObs> &observations,
                                   const Map &map_landmarks) {
  /**
   * TODO: Update the weights of each particle using a mult-variate Gaussian
   *   distribution. You can read more about this distribution here:
   *   https://en.wikipedia.org/wiki/Multivariate_normal_distribution
   * NOTE: The observations are given in the VEHICLE'S coordinate system.
   *   Your particles are located according to the MAP'S coordinate system.
   *   You will need to transform between the two systems. Keep in mind that
   *   this transformation requires both rotation AND translation (but no scaling).
   *   The following is a good resource for the theory:
   *   https://www.willamette.edu/~gorr/classes/GeneralGraphics/Transforms/transforms2d.htm
   *   and the following is a good resource for the actual equation to implement
   *   (look at equation 3.33) http://planning.cs.uiuc.edu/node99.html
   */
   for (size_t i = 0; i < num_particles; i++) {
     // looping each particle
     particles[i].weight = 1.0;

     // ** get the positions and heading of the particle **
     double px = particles[i].x;
     double py = particles[i].y;
     double ptheta = particles[i].theta;

     // ** get predicted landmarks within sensor range **
     vector<LandmarkObs> predictions;

     for (size_t j = 0; j < map_landmarks.landmark_list.size(); j++) {
       // looping each landmark in the map
       double lm_dist = dist(px, py, map_landmarks.landmark_list[j].x_f, map_landmarks.landmark_list[j].y_f);
       if (lm_dist < sensor_range) {
         predictions.push_back(LandmarkObs{
           map_landmarks.landmark_list[j].id_i,
           map_landmarks.landmark_list[j].x_f,
           map_landmarks.landmark_list[j].y_f
         });
       }
     }

     // ** translate observations from vehicle coordinates to world coordinates **
     vector<LandmarkObs> observations_world;
     for (size_t j = 0; j < observations.size(); j++) {
       // looping each observations (to the landmark)
       double x_world = cos(ptheta)*observations[j].x - sin(ptheta)*observations[j].y + px;;
       double y_world = sin(ptheta)*observations[j].x + cos(ptheta)*observations[j].y + py;
       observations_world.push_back(LandmarkObs{
         observations[j].id,
         x_world,
         y_world
       });
     }

     // ** associate observations with predictions **
     //dataAssociation(predictions, observations_world);
     dataAssociation(predictions, observations_world, particles[i]);

     // ** calculate particle weight using multivariate Gaussian distribution **

     for (size_t j = 0; j < observations_world.size(); j++) {
       // looping each observations
       double obsX = observations_world[j].x;
       double obsY = observations_world[j].y;
       double predX, predY;
       int lmID = observations_world[j].id;

       // find the corresponding prediction coordinates
       for (size_t jj = 0; jj < predictions.size(); jj++) {
         if (predictions[jj].id == lmID) {
           predX = predictions[jj].x;
           predY = predictions[jj].y;
           break;
         }
       }

       // calculate weights
       double sx = std_landmark[0];
       double sy = std_landmark[1];
       double obs_w = ( 1/(2*M_PI*sx*sy)) * exp( -(pow(predX-obsX,2)/(2*pow(sx,2)) + (pow(predY-obsY,2)/(2*pow(sy,2))) ) );
       particles[i].weight *= obs_w;
     }
   }
}

void ParticleFilter::resample() {
  /**
   * TODO: Resample particles with replacement with probability proportional
   *   to their weight.
   * NOTE: You may find std::discrete_distribution helpful here.
   *   http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution
   */
   vector<Particle> resampled_particles;

   vector<double> weights;

   for (size_t i = 0; i < num_particles; i++) {
     weights.push_back(particles[i].weight);
   }

   std::discrete_distribution<> dist(weights.begin(), weights.end());
   std::default_random_engine gen;

   int index = -1;

   for (size_t i = 0; i < num_particles; i++) {
     index = dist(gen);
     resampled_particles.push_back(particles[index]);
   }

   particles = resampled_particles;
}

//void ParticleFilter::SetAssociations(Particle& particle,
//                                     const vector<int>& associations,
//                                     const vector<double>& sense_x,
//                                     const vector<double>& sense_y) {
Particle ParticleFilter::SetAssociations(Particle& particle, const vector<int> &associations,
  const vector<double> &sense_x, const vector<double> &sense_y){
  // particle: the particle to which assign each listed association,
  //   and association's (x,y) world coordinates mapping
  // associations: The landmark id that goes along with each listed association
  // sense_x: the associations x mapping already converted to world coordinates
  // sense_y: the associations y mapping already converted to world coordinates
  particle.associations= associations;
  particle.sense_x = sense_x;
  particle.sense_y = sense_y;
  return particle;
}

string ParticleFilter::getAssociations(Particle best) {
  vector<int> v = best.associations;
  std::stringstream ss;
  copy(v.begin(), v.end(), std::ostream_iterator<int>(ss, " "));
  string s = ss.str();
  s = s.substr(0, s.length()-1);  // get rid of the trailing space
  return s;
}

string ParticleFilter::getSenseCoord(Particle best, string coord) {
  vector<double> v;

  if (coord == "X") {
    v = best.sense_x;
  } else {
    v = best.sense_y;
  }

  std::stringstream ss;
  copy(v.begin(), v.end(), std::ostream_iterator<float>(ss, " "));
  string s = ss.str();
  s = s.substr(0, s.length()-1);  // get rid of the trailing space
  return s;
}
