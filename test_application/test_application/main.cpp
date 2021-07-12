#include <many_lights/many_lights.h>
#include <memory>

#include "test_application/deferred_renderer.h"
#include "test_application/forward_renderer.h"
#include "test_application/stochastic_lightcuts_renderer.h"

int main()
{
    constexpr size_t max_lights = 5000;
	
    std::unique_ptr<ml::ManyLights<max_lights>> many_lights = std::make_unique<ml::ManyLights<max_lights>>();
    many_lights->add_model("../assets/sponza/sponza.obj");
    many_lights->set_lights(20, 3.0f);

    many_lights->add_algorithm<TestApplication::StochasticLightcuts<max_lights, 20, 32>>(many_lights->get_scene());
    many_lights->add_algorithm<TestApplication::Deferred<max_lights>>(many_lights->get_scene());
    many_lights->add_algorithm<TestApplication::ForwardBlinnPhong<max_lights>>(many_lights->get_scene());

    many_lights->run();
	
    return 0;
}
