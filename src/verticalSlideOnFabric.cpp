#include <chrono>
#include <math.h>
#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>

#include <franka/exception.h>
#include <franka/gripper.h>
#include <franka/robot.h>

#include "pcan_interface.h"
#include "controller.h"
#include "examples_common.h"

using namespace std;

void hold(franka::Gripper &gripper, franka::GripperState &gripper_state, CANBus &can, bool &is_holding, int target_val)
{
    int step = 1;
    double val;
    double width = 0.0, target_width = 0.0;
    while (is_holding && can.status() == 0)
    {
        val = can.read().average();
        target_width = (val - target_val) * 1E-5 / step + width;
        width = target_width;
        printf("Step [%d], width: %f, target width: %f, average: %f\n", step, width, target_width, val);
        if (!gripper.move(target_width, 0.01))
        {
            cerr << "Cannot move to target width!" << endl;
            break;
        }
        if (step < 50)
            step += 1;
    }
    gripper.stop();

    if (can.status() == -1)
    {
        cerr << "CAN-Bus reading error!" << endl;
    }
}

int slideMulti(franka::Robot &robot, franka::Gripper &gripper, int startID, int endID, int repitition)
{
    int res = 0;
    franka::GripperState gripper_state = gripper.readOnce();

    // CAN-BUS variables
    chrono::steady_clock::time_point now = chrono::steady_clock::now();
    CANBus can(now);

    // fabric list
    map<int, string> fabrics;
    fabrics[0] = "fabric1";
    fabrics[1] = "fabric2";
    fabrics[2] = "fabric3";
    fabrics[3] = "fabric4";
    fabrics[4] = "fabric5";

    // Define robot variables
    double p_x = 0.1;
    double p_dy = 0.15;
    double v_zs[6] = {0.01, 0.02, 0.05, 0.1, 0.12, 0.15};
    int forze[4] = {120, 150, 180, 210};

    // velocities and end-poses
    array<double, 6> v_max{{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};
    array<double, 6> pose_out{{p_x, 0.0, 0.0, 0.0, 0.0, 0.0}};
    array<double, 6> pose_in{{-p_x, 0.0, 0.0, 0.0, 0.0, 0.0}};
    array<double, 6> pose_next{{0.0, p_dy, 0.0, 0.0, 0.0, 0.0}};
    array<double, 6> pose_up{{0.0, 0.0, 0.56, 0.0, 0.0, 0.0}};

    // Controllers
    CartPoseCtrl pc(3, pose_out);
    CartVelCtrl vc(0.5, 5.0, v_max);

    string const path = "../sliding_test/";
    string filename;
    thread holder;
    bool is_holding = false;
    
    std::cout << "Calibrating sensors' zero reading..." << endl;
    filename = "zero-calibration.csv";
    can.logOn(path, filename);
    this_thread::sleep_for(chrono::seconds(5));
    can.logOff();

    try
    {
        int i = 0;
        // Repeated sliding movement
        while (i < endID)
        {
            if (i >= startID)
            {
                // Reach out
                pc.setPose(pose_out);
                robot.control(pc);
                printf("INFO: fabric ID - %s\n", fabrics[i].c_str());

                for (auto &forza : forze)
                {
                    bool calibrated = false;
                    for (auto &v_z : v_zs)
                    {
                        vc.setDuration(0.5 / v_z);
                        vc.setTimeMax(0.03 * M_PI / (2 * v_z));
                        int id_velocity = (int)(v_z * 1000);
                        for (int j = 0; j < repitition; ++j)
                        {
                            if (can.status() != 0)
                            {
                                throw franka::Exception("Cannot hold the gripper!");
                            }
                            // Wait for the gripper to reach the holding width
                            is_holding = true;

                            gripper.homing();
                            gripper.move(0.0015, 0.2);
                            holder = thread([&gripper, &gripper_state, &can, &is_holding, forza]() {
                                hold(gripper, gripper_state, can, is_holding, forza);
                            });
                            this_thread::sleep_for(chrono::seconds(15));

                            if (!calibrated) 
                            {
                            filename = fabrics[i] + "_F" + to_string(forza) + "_calibration.txt";
                                can.logOn(path, filename);
                            this_thread::sleep_for(chrono::seconds(5));
                            can.logOff();
                            calibrated = true;
                            }

                            printf("INFO: Sliding down at v=%.3f...\n", -v_z);
                            filename = fabrics[i] + "_F" + to_string(forza) + "_V" + to_string(-id_velocity) + "mmps_" + to_string(j) + ".csv";
                            can.logOn(path, filename);
                            v_max[2] = -v_z;
                            vc.setVelMax(v_max);
                            robot.control(vc);
                            can.logOff();

                            is_holding = false;
                            if (holder.joinable())
                            {
                                holder.join();
                            }
                            gripper.move(gripper_state.max_width, 0.3);

                            printf("INFO: Sliding up...\n");
                            pc.setPose(pose_up);
                            robot.control(pc);
                        }
                    };
                }
                gripper.move(gripper_state.max_width / 2, 0.1);

                // Retreat
                pc.setPose(pose_in);
                robot.control(pc);
            }
            // Move to next fabric position
            pc.setPose(pose_next);
            robot.control(pc);

            i++;
        }
    }
    catch (const franka::Exception &e)
    {
        can.logOff();
        is_holding = false;
        if (holder.joinable())
            holder.join();
        gripper.stop();
        gripper.move(gripper_state.max_width / 2, 0.1);
        cerr << e.what() << endl;
        robot.automaticErrorRecovery();
        res = -1;
        goto exit;
    }

exit:
    // Retreat
    pc.setPose(pose_in);
    robot.control(pc);

    return res;
}

int main(int argc, char **argv)
{
    string robot_ip = "172.16.0.2";

    /***************************
     * Prepare robot and sensors
     **************************/
    franka::Robot robot(robot_ip);
    setDefaultBehavior(robot);
    franka::Gripper gripper(robot_ip);
    gripper.move(0.05, 0.3);

    // First move the robot to a suitable joint configuration
    cout << "INFO: Moving to initial pose..." << endl;
    array<double, 7> q_goal = {{-0.796261, -0.2465, -0.258172, -1.78312, 1.45704, 2.11494, -0.72011}};
    MotionGenerator motion_generator(0.3, q_goal);
    robot.control(motion_generator);

    cout << "Prepared to run tests. Press enter to continue...";
    cin.ignore();

    int startID = 0, endID = 5, repitition = 3;
    if (argc >= 2)
    {
        startID = stoi(argv[1]);
    }
    if (argc >= 3)
    {
        endID = stoi(argv[2]);
    }
    if (argc >= 4)
    {
        repitition = stoi(argv[3]);
    }

    if (slideMulti(robot, gripper, startID, endID, repitition) < 0)
    {
        cerr << "WARNING: Task failed!" << endl;
    }
    else
    {
        cout << "INFO: Task finished." << endl;
    }

    return 0;
}
