#include <many_lights/many_lights.h>
#include <memory>

#include "test_application/deferred_renderer.h"

int main()
{
    std::unique_ptr<ml::ManyLights<200>> many_lights = std::make_unique<ml::ManyLights<200>>();
    many_lights->add_model("../assets/sponza/sponza.obj");
    many_lights->set_lights(70, 3.0f);

    many_lights->set_algorithm<TestApplication::Deferred>();

    many_lights->run();
	
    return 0;
}
