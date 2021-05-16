# Rendering Scenes with Many Lights

This project aims to set out a framework for easily testing algorithms that deal with rendering scenes with many lights, as well as some implementations of such algorithms.

The repo exclusively uses OpenGL for it's graphics API.

## Setting up

The repo should be buildable on both Windows and Linux, however it is untested on Linux. To build with CMake:

```
git clone https://github.com/Cascades/many_lights.git
cd many_lights
mkdir build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=[location_to_install] -GNinja
cmake --build .
cmake --install .
```

You can then find the example application in the bin folder of the install directory specified.

Setting the many_lights library up for external consumption is very much planned for the future, and may even work now... who knows.

## Example use

The idea behind the library aspect of this repo is to be as quick and easy to use as possible. Because of this, everything revolves around the `ml::ManyLights` class. To start, first create a scene (with a GLFW context too) we simply call:

```
#include <many_lights/many_lights.h>

std::unique_ptr<ml::ManyLights> many_lights = std::make_unique<ml::ManyLights>();
many_lights->add_model("../assets/sponza/sponza.obj");
many_lights->set_lights(200, 70, 3.0f);
```
This not only sets up the scene framework, but also adds the Sponza model, as well as 70 lights (the first parameter if max_lights, and the last is the automatically arranged lights' height off the ground).

Next we need to create an algorithm. This is done through the creation of `ml::ManyLightsAlgorithm` derived class. This class looks like:

```
class ManyLightsAlgorithm
{
public:
    ManyLightsAlgorithm() = default;
    virtual ~ManyLightsAlgorithm() = default;
    virtual void init(int const& width, int const& height) = 0;
    virtual void adjust_size(int const& width, int const& height) = 0;
    virtual void render(ml::Scene& scene) = 0;
};
```

The `init` function can be used for any pre-processing that shouldn't be done in the main render loop. `adjust_size` is called whenever there is a resize of the window, so a consumer can update their frame buffers. Finally, `render` is called each frame to perform the render passes.

If we create a class `TestApplication::Deferred` like so:

```
class Deferred final : public ml::ManyLightsAlgorithm
{
public:
    ml::Shader geometry_pass_shader;
    ml::Shader light_pass_shader;
    unsigned int g_buffer;
    unsigned int g_position, g_normal, g_diff_spec, g_ambient;
    unsigned int depth_buffer;
    std::vector<unsigned int> attachments = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };

    unsigned int quadVAO;
    unsigned int quadVBO;

    void init(int const& width, int const& height) override;

    void adjust_size(int const& width, int const& height) override;

    void render(ml::Scene& scene) override;
};
```

Then we can encapsulate our rendering algorithm and pass it to `ml::ManyLights` like so:

```
many_lights->set_algorithm<TestApplication::Deferred>();
```

Finally, we need only call `ml::ManyLights::run()` to run the entire application.

```
many_lights->run();
```

Parameters such as number of lights and benchmarking are available through the imgui UI during runtime.
