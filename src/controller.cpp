#include <array>
#include <chrono>
#include <math.h>
#include <stdlib.h>

#include <franka/exception.h>
#include <franka/robot.h>

#include "controller.h"

#define sgn(x) ((x > 0) ? 1 : ((x < 0) ? -1 : 0))

using namespace std;

void CartVelCtrl::zeroTime() { time_ = 0.0; };

void CartVelCtrl::setVelMax(array<double, 6> &v_max)
{
    v_max_ = v_max;
    zeroTime();
}

void CartVelCtrl::setDuration(double duration)
{
    duration_ = duration;
    zeroTime();
}

void CartVelCtrl::setTimeMax(double time_max)
{
    time_max_ = time_max;
    zeroTime();
}

franka::CartesianVelocities CartVelCtrl::operator()(const franka::RobotState &robot_state, franka::Duration period)
{
    time_ += period.toSec();
    bool motion_finished = false;

    if (time_ < time_max_)
    {
        for (size_t i = 0; i < 6; ++i)
        {
            v_[i] = v_max_[i] * sin(0.5 * M_PI * time_ / time_max_);
        }
    }
    else if (time_ >= time_max_ && time_ < time_max_ + duration_)
    {
        for (size_t i = 0; i < 6; ++i)
            v_[i] = v_max_[i];
    }
    else if (time_ < 2 * time_max_ + duration_)
    {
        double t = time_ - duration_;
        for (size_t i = 0; i < 6; ++i)
            v_[i] = v_max_[i] * sin(0.5 * M_PI * t / time_max_);
    }
    else
    {
        v_.fill(0.0);
        motion_finished = true;
    }

    franka::CartesianVelocities output(v_);
    if (motion_finished)
    {
        return franka::MotionFinished(output);
    }
    else
    {
        return output;
    }
};

void CartPoseCtrl::setPose(array<double, 6> &d_pose)
{
    d_pose_ = d_pose;

    for (size_t i = 0; i < 6; ++i)
    {
        v_max_[i] = 0.25 * M_PI * d_pose[i] / time_max_;
    }

    zeroTime();
};