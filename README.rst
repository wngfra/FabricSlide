==================================
Tactile Sensing via Sliding Motion
==================================

## verticalSlidingFabric
* Library dependency: `libfranka-0.8.0`, `pcanbasic-8.8.0`, `Eigen3`
* Using shared memory for CAN data logging; a basic logger is implemented in `pcan_interface`
* Default build-type is `Release`, for debugging with GDB, change the build-type in `CMakeLists.txt`

## Data Analysis
1. Pre-built [wngfra/dl-jupyterlab](https://hub.docker.com/r/wngfra/dl-jupyterlab) images for data analysis
2. Provided `docker-compose.yml` to set up a `functional-data-analysis` container for tactile signal analysis
3. Check `analysis.ipynb` for preliminary data analysis

## Note
* `CAD` contains the `SolidWorks` design for 3D printing to mount the CPM-finger sensor and fabric samples

Citation 
^^^^^^^^^^^^^^^^^^^^^^^^

`Wang, S. A., Albini, A., Maiolino, P., Mastrogiovanni, F., & Cannata, G. (2022). Fabric Classification Using a Finger-Shaped Tactile Sensor via Robotic Sliding. Frontiers in neurorobotics, 16, 808222. <https://doi.org/10.3389/fnbot.2022.808222>`