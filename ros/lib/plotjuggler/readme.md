# Building PlotJuggler from sources

To use the latest version of PlotJuggler, it is recommended to build the software from source.

First, initialize and update the repository submodules (if you have not already done so):
```sh
git submodule init
git submodule update
```

Next, initialize and update rosdep (if not already done in another ROS 2 project):
```sh
sudo rosdep init
rosdep update
```

Then, install the dependencies and build PlotJuggler along with its ROS 2 plugin.

Set INSTALL_PATH to your desired absolute installation path. 
It is usually best to use the same installation path as this project so that the generated launch script works correctly.

```sh
    cd ros/lib/plotjuggler
    rosdep install --from-paths src --ignore-src -y
    colcon build --parallel-workers 6 --cmake-clean-cache --install-base ${INSTALL_PATH}/opt/ros2 --cmake-args -DCMAKE_INSTALL_PREFIX="${INSTALL_PATH}/opt/ros2"
```
