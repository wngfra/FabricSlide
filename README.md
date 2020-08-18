# Tactile Sensing via Sliding Motion

## verticalSlidingFabric
* Library dependency: `libfranka-0.8.0`, `pcanbasic-8.8.0`, `Eigen3`
* Using shared memory for CAN data logging; a basic logger is implemented in `pcan_interface`
* Default build-type is `Release`, for debugging with GDB, change the build-type in `CMakeLists.txt`
* `matlab` contains a simple PCA and clustering algorithm for signal anslysis

## HOOI (Higher-Order Orthogonal Iteration)
* [python]Functional data representation (Fourier basis)
* [matlab]Tensor based multi-linear PCA analysis and data encoding
* Exploring potential linear/quasi-linear representation of higher dimensional tactile data

## Note
* `CAD` contains the components for 3D printing to mount the CPM-finger sensor and fabric samples
