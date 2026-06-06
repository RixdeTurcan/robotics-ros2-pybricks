# Building PlotJuggler from sources

In order to have the last version of PlotJuggler, we recommend to directly build the softwxare from source.

The first step, if not already done, is to initialize the repository submodules :
```sh
git submodule init
git submodule update
```

Then, if not done in another ROS2 project, update rosdep :
```sh
sudo rosdep init
rosdep update
```
Now, we can install the dependencies, then build PlotJuggler and its ROS2 plugin 
(set INSTALL_PATH with your desired absolute install path, 
usually the same than this project install path, to have the generated launch script works directly) :
```sh
    cd ros/lib/plotjuggler
    rosdep install --from-paths src --ignore-src -y
    colcon build --parallel-workers 6 --cmake-clean-cache --install-base ${INSTALL_PATH}/opt/ros2 --cmake-args -DCMAKE_INSTALL_PREFIX="${INSTALL_PATH}/opt/ros2"
```
