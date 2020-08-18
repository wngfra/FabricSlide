#pragma once

#include <array>
#include <string>
#include <thread>

#include <franka/control_types.h>
#include <franka/duration.h>
#include <franka/robot.h>
#include <franka/robot_state.h>
#include <franka/gripper.h>

using namespace std;

class CartVelCtrl
{
public:
    CartVelCtrl(double time_max, double duration, array<double, 6> &v_max) : time_max_(time_max), duration_(duration), v_max_(v_max) {}

    //Set time_ to zero to reuse the controller.
    void zeroTime();
    void setVelMax(array<double, 6> &);
    void setDuration(double);
    void setTimeMax(double);
    /*
     * Sends Cartesian velocity calculations
     *
     * @return Cartesian velocities for use inside a control loop
     */
    franka::CartesianVelocities operator()(const franka::RobotState &, franka::Duration);

protected:
    array<double, 6> v_max_;
    double time_max_; // time to the max velocity
    double time_ = 0.0;
    double duration_;
    array<double, 6> v_;
};

class CartPoseCtrl : public CartVelCtrl
{
public:
    CartPoseCtrl(double time, array<double, 6> &d_pose) : CartVelCtrl(time / 2.0, 0.0, d_pose) { setPose(d_pose); };

    void setPose(array<double, 6> &);

private:
    array<double, 6> d_pose_ = {{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};
};