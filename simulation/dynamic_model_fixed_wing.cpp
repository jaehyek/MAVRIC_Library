/*******************************************************************************
 * Copyright (c) 2009-2016, MAV'RIC Development Team
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

/*******************************************************************************
 * \file dynamic_model_fixed_wing.cpp
 *
 * \author MAV'RIC Team
 * \author Nicolas Jacquemin
 * \author Julien Lecoeur
 *
 * \brief Simulated dynamics of a fixed wing UAV
 *
 ******************************************************************************/


#include "simulation/dynamic_model_fixed_wing.hpp"
#include <iostream>

extern "C"
{
#include "hal/common/time_keeper.hpp"
}



//------------------------------------------------------------------------------
// PUBLIC FUNCTIONS IMPLEMENTATION
//------------------------------------------------------------------------------

Dynamic_model_fixed_wing::Dynamic_model_fixed_wing(Servo& servo_motor,
        Servo& servo_flap_left,
        Servo& servo_flap_right,
        dynamic_model_fixed_wing_conf_t config):
    servo_motor_(servo_motor),
    servo_flap_left_(servo_flap_left),
    servo_flap_right_(servo_flap_right),
    config_(config),
    motor_speed_(0.0f),
    left_flap_(0.0f,quat_t{1.0f, {0.0f, 0.0f, 0.0f}}, 0.0f, 0.1f, 0.0f, 0.02f, 0.1f),//TODO: change these to the correct values
    right_flap_(0.0f,quat_t{1.0f, {0.0f, 0.0f, 0.0f}}, 0.0f, -0.1f, 0.0f, 0.02f, 0.1f),
    left_drift_(0.0f,quat_t{0.0f, {0.0f, 0.0f, 0.0f}}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
    right_drift_(0.0f,quat_t{0.0f, {0.0f, 0.0f, 0.0f}}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
    torques_bf_(std::array<float, 3> {{0.0f, 0.0f, 0.0f}}),
    rates_bf_(std::array<float, 3> {{0.0f, 0.0f, 0.0f}}),
    lin_forces_bf_(std::array<float, 3> {{0.0f, 0.0f, 0.0f}}),
    acc_bf_(std::array<float, 3> {{0.0f, 0.0f, 0.0f}}),
    vel_bf_(std::array<float, 3> {{5.0f, 0.0f, 0.0f}}),
    vel_(std::array<float, 3> {{5.0f, 0.0f, 0.0f}}),
    attitude_(quat_t{1.0f, {0.0f, 0.0f, 0.0f}}),
    last_update_us_(time_keeper_get_us()),
    dt_s_(0.004f)
{
    // Init local position
    local_position_.pos[0]  = 0.0f;
    local_position_.pos[1]  = 0.0f;
    local_position_.pos[2]  = -50.0f;
    local_position_.heading = 0.0f;
    local_position_.origin.latitude         = config_.home_coordinates[0];
    local_position_.origin.longitude        = config_.home_coordinates[1];
    local_position_.origin.altitude         = config_.home_coordinates[2];
    local_position_.origin.heading          = 0.0f;

    // Init global position
    global_position_ = coord_conventions_local_to_global_position(local_position_);
}


bool Dynamic_model_fixed_wing::update(void)
{
    int32_t i;
    quat_t qtmp1, qvel_bf, qed;
    const quat_t up     = { 0.0f, {UPVECTOR_X, UPVECTOR_Y, UPVECTOR_Z} };

    // Update timing
    float now       = time_keeper_get_us();
    dt_s_           = (now - last_update_us_) / 1000000.0f;
    last_update_us_ = now;


    // Do nothing if updated too often
    /*if (dt_s_ < 0.001f)
    {
        return true;
    }*///TODO: remove this when it works

    // Clip dt if too large, this is not realistic but the simulation will be more precise
    //if (dt_s_ > 0.1f) TODO: Uncomment this
    {
        dt_s_ = 0.01f;
    }

    // compute torques and forces based on servo commands and positions of the flaps
    forces_from_servos();

    // integrate torques to get simulated gyro rates (with some damping)
    rates_bf_[0] = maths_clip((1.0f - 0.1f * dt_s_) * rates_bf_[0] + dt_s_ * torques_bf_[0] / config_.roll_momentum, 10.0f);
    rates_bf_[1] = maths_clip((1.0f - 0.1f * dt_s_) * rates_bf_[1] + dt_s_ * torques_bf_[1] / config_.pitch_momentum, 10.0f);
    rates_bf_[2] = maths_clip((1.0f - 0.1f * dt_s_) * rates_bf_[2] + dt_s_ * torques_bf_[2] / config_.yaw_momentum, 10.0f);


    qtmp1.s = 0.0f;
    for (i = 0; i < 3; i++)
    {
        qtmp1.v[i] = 0.5f * rates_bf_[i];
    }

    // apply step rotation
    qed = quaternions_multiply(attitude_, qtmp1);

    // TODO: correct this formulas when using the right scales
    attitude_.s = attitude_.s + qed.s * dt_s_;
    attitude_.v[0] += qed.v[0] * dt_s_;
    attitude_.v[1] += qed.v[1] * dt_s_;
    attitude_.v[2] += qed.v[2] * dt_s_;

    attitude_           = quaternions_normalise(attitude_);
    quat_t up_vec       = quaternions_global_to_local(attitude_, up);

    // velocity and position integration

    // check altitude - if it is lower than 0, clamp everything (this is in NED, assuming negative altitude)
    if (local_position_.pos[Z] > 0)
    {
        printf("Ouccccccccccccccccccccccccccccch!!!\n");//TODO: REMOVE
        vel_[Z] = 0.0f;
        local_position_.pos[Z] = 0.0f;

        // simulate "acceleration" caused by contact force with ground, compensating gravity
        for (i = 0; i < 3; i++)
        {
            lin_forces_bf_[i] = up_vec.v[i] * config_.total_mass * config_.gravity;
        }

        // slow down... (will make velocity slightly inconsistent until next update cycle, but shouldn't matter much)
        for (i = 0; i < 3; i++)
        {
            vel_bf_[i] = 0.95f * vel_bf_[i];
        }

        //upright
        rates_bf_[0] =  up_vec.v[1];
        rates_bf_[1] =  - up_vec.v[0];
        rates_bf_[2] = 0;
    }

    for (i = 0; i < 3; i++)
    {
        qtmp1.v[i] = vel_[i];
    }
    qtmp1.s = 0.0f;
    qvel_bf = quaternions_global_to_local(attitude_, qtmp1);

    for (i = 0; i < 3; i++)
    {
        vel_bf_[i] = qvel_bf.v[i];

        // this is the "clean" acceleration without gravity
        acc_bf_[i] = lin_forces_bf_[i] / config_.total_mass - up_vec.v[i] * config_.gravity;

        vel_bf_[i] = vel_bf_[i] + acc_bf_[i] * dt_s_;
    }

    // calculate velocity in global frame
    // vel = qe *vel_bf * qe - 1
    qvel_bf.s = 0.0f; qvel_bf.v[0] = vel_bf_[0]; qvel_bf.v[1] = vel_bf_[1]; qvel_bf.v[2] = vel_bf_[2];
    qtmp1 = quaternions_local_to_global(attitude_, qvel_bf);
    vel_[0] = qtmp1.v[0]; vel_[1] = qtmp1.v[1]; vel_[2] = qtmp1.v[2];

    for (i = 0; i < 3; i++)
    {
        local_position_.pos[i] = local_position_.pos[i] + vel_[i] * dt_s_;
    }

    local_position_.heading = coord_conventions_get_yaw(attitude_);

    global_position_ = coord_conventions_local_to_global_position(local_position_);

    return true;
}


const float& Dynamic_model_fixed_wing::last_update_us(void) const
{
    return last_update_us_;
}


const std::array<float, 3>& Dynamic_model_fixed_wing::acceleration_bf(void) const
{
    return acc_bf_;
}


const std::array<float, 3>& Dynamic_model_fixed_wing::velocity_lf(void) const
{
    return vel_;
}


const local_position_t& Dynamic_model_fixed_wing::position_lf(void) const
{
    return local_position_;
}


const global_position_t& Dynamic_model_fixed_wing::position_gf(void) const
{
    return global_position_;
}


const std::array<float, 3>& Dynamic_model_fixed_wing::angular_velocity_bf(void) const
{
    return rates_bf_;
}


const quat_t& Dynamic_model_fixed_wing::attitude(void) const
{
    return attitude_;
}


//------------------------------------------------------------------------------
// PRIVATE FUNCTIONS IMPLEMENTATION
//------------------------------------------------------------------------------

void Dynamic_model_fixed_wing::forces_from_servos(void)
{
    // float motor_command = 1.0f;//servo_motor_.read() - config_.rotor_rpm_offset;
    float motor_command = servo_motor_.read() - config_.rotor_rpm_offset;
    float flaps_angle_left = servo_flap_left_.read() - config_.flap_offset; //TODO: transform these in angles
    float flaps_angle_right = servo_flap_right_.read() - config_.flap_offset;
    left_flap_.set_flap_angle(flaps_angle_left);
    right_flap_.set_flap_angle(flaps_angle_right);

    quat_t wind_gf  = {};
    wind_gf.s       = 0;
    wind_gf.v[0]    = config_.wind_x;
    wind_gf.v[1]    = config_.wind_y;
    wind_gf.v[2]    = 0.0f;
    quat_t wind_bf  =  quaternions_global_to_local(attitude_, wind_gf);
    //Take into account the speed of the plane
    wind_bf.v[0] -= vel_bf_[0];
    wind_bf.v[1] -= vel_bf_[1];
    wind_bf.v[2] -= vel_bf_[2];
    printf("\n%f %f %f\n",vel_bf_[0],vel_bf_[1],vel_bf_[2]);
    wing_model_forces_t motor_forces = compute_motor_forces(wind_bf, motor_command);
    wing_model_forces_t left_flap_force = left_flap_.compute_forces(wind_bf);
    wing_model_forces_t right_flap_force = right_flap_.compute_forces(wind_bf);
    wing_model_forces_t left_drift_force = left_drift_.compute_forces(wind_bf);
    wing_model_forces_t right_drift_force = right_drift_.compute_forces(wind_bf);

    // torque around x axis (roll)
    torques_bf_[ROLL] = motor_forces.torque[ROLL] +
			left_flap_force.torque[ROLL] +
			right_flap_force.torque[ROLL] +
			left_drift_force.torque[ROLL] +
      right_drift_force.torque[ROLL];

    // torque around y axis (pitch)
    torques_bf_[PITCH] = motor_forces.torque[PITCH] +
			 left_flap_force.torque[PITCH] +
			 right_flap_force.torque[PITCH] +
       left_drift_force.torque[PITCH] +
       right_drift_force.torque[PITCH];

    // torque around z axis (yaw)
    torques_bf_[YAW] = motor_forces.torque[YAW] +
		       left_flap_force.torque[YAW] +
		       right_flap_force.torque[YAW] +
           left_drift_force.torque[YAW] +
           right_drift_force.torque[YAW];

    lin_forces_bf_[X] = motor_forces.force[0] +
			left_flap_force.force[0] +
			right_flap_force.force[0] +
      left_drift_force.force[0] +
      right_drift_force.force[0];

    lin_forces_bf_[Y] = motor_forces.force[1] +
			left_flap_force.force[1] +
			right_flap_force.force[1] +
      left_drift_force.force[1] +
      right_drift_force.force[1];

    lin_forces_bf_[Z] = motor_forces.force[2] +
			left_flap_force.force[2] +
			right_flap_force.force[2] +
      left_drift_force.force[2] +
      right_drift_force.force[2];
      for(int i=0; i<3; i++) printf(" torque%d: %f",i, torques_bf_[i]);
      printf("\n");
      for(int i=0; i<3; i++) printf(" force%d: %f",i, lin_forces_bf_[i]);
      printf("\n");
}

wing_model_forces_t Dynamic_model_fixed_wing::compute_motor_forces(quat_t wind_bf,float motor_command)
{
  //Same as for quad rotors
  // temporarily save old rotor speeds
  float old_motor_speed = motor_speed_;
  // estimate rotor speeds
  motor_speed_ = (motor_command * config_.rotor_rpm_gain);

  // calculate torque created by rotor inertia
  float rotor_inertia = (motor_speed_ - old_motor_speed) / dt_s_ * config_.rotor_momentum;

  float sqr_lat_airspeed = SQR(wind_bf.v[1]) + SQR(wind_bf.v[2]);
  float ldb = lift_drag_base(motor_speed_, sqr_lat_airspeed, -wind_bf.v[0]);

  wing_model_forces_t motor_forces;
  motor_forces.force[0] = ldb * config_.rotor_cl;
  motor_forces.force[1] = 0.0f;
  motor_forces.force[2] = 0.0f;
  motor_forces.torque[ROLL] = 0.0f;//(10.0f * ldb * config_.rotor_cd + rotor_inertia) * config_.rotor_diameter;
  motor_forces.torque[PITCH] = 0.0f;
  motor_forces.torque[YAW] = 0.0f;

  printf("Motor force: %f, Motor torque: %f\n",motor_forces.force[0], motor_forces.torque[0]);

  return motor_forces;
}

float Dynamic_model_fixed_wing::lift_drag_base(float rpm, float sqr_lat_airspeed, float axial_airspeed)
{
    if (rpm < 0.1f)
    {
        return 0.0f;
    }
    float mean_vel = config_.rotor_diameter * PI * rpm / 60.0f;
    float exit_vel = rpm / 60.0f * config_.rotor_pitch;

    return (0.5f * config_.air_density * (mean_vel * mean_vel + sqr_lat_airspeed) * config_.rotor_foil_area  * (1.0f - (axial_airspeed / exit_vel)));
}
