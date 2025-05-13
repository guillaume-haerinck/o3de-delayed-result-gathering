<u>Supported o3de versions</u> : **24.09.2**

# Delayed Result Gathering

![gameplay](gameplay.gif?raw=true)

A simple hide-and-seek gameplay map used to try out 03DE profiling gems. You control a character with a 3rd person camera, and as you move in the map, the enemy entity will try to hide so that you can't see him anymore.

You switch at runtime between multiple performance optimizations to try out the profilers. More information below.

This project is inspired by The Last of Us AI picking system. See the links below :

- https://allenchou.net/2021/05/delayed-result-gathering/
- https://www.gdcvault.com/play/1029277/Picking-a-Good-Spot-Naughty

A similar version of this project exist [for Unreal Engine](https://gitlab.com/learn-computer-graphics/the-last-of-us-spot-picking).

This project was presented during 03DE connect in May 2025. The presentation slides are [available here](https://docs.google.com/presentation/d/1j28zqFNe3FuRtARux-FLO8Nxp71g734Uod-Z27dXyE0/edit?usp=sharing).

## Prerequisites

You need to build or [install O3DE engine](https://o3de.org/download/).

As this project uses C++ code, you need to have compiler. See how to proceed via [this documentation](https://www.docs.o3de.org/docs/welcome-guide/requirements/#microsoft-windows).

## How to run

1. Clone the github repository (`git clone https://github.com/guillaume-haerinck/o3de-delayed-result-gathering.git`). When prompted to authenticate, use your github username and the token as password.
2. Launch O3DE. It will open the Project manager. Click on the **New Project** button then **Open Existing Project** option.
3. Navigate to the cloned repository. Open the folder. The project should now be registered.

![project](preview.png?raw=true)

4. Click on the **Build Project** button, located on the **DelayedResultGathering** image.
5. Once the project has been built successfully, use the **Open Editor** button.
6. The asset pre-processor will run for a bit. Once it is over you will be welcomed with the **Open a Level** window. Pick the default one.

## Controls

Use the WASD keys to move the character around and the mouse to move the camera

## How to switch between performance optimizations ?

In the top navigation bar, go to **"Tools->Console Variables"**. If you search for "Exposure" you should see multiple variables to modify. Hover over them to show a tooltip for the description.

- `exposuremap_threadMode` : This is the main option with the following values
    - "0" : Single threaded
    - "1" : Multi-threaded with result gathering done during the same frame
    - "2" : Multi-threaded with result gathering done at the next frame
    - "3" : Multi-threaded with time slicing

- `exposuremap_forceSlow` : Computes fibonnacci suite at given index if value is >0 for during each raycast call. Use that to simulate heavy workload (value of 10 is already heavy)

- `exposuremap_forceCpuCoreCount` : At 0 it won't change anything but all above that will split the batch jobs as if you had this amount of CPU cores available

- `exposuremap_show` : At 0 do not do the debug drawing for the grid, at 1 do it

## License

MIT

