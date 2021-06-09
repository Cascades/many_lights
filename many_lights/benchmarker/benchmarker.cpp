#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "many_lights/benchmarker.h"

#include <fstream>
#include <chrono>
#include <iostream>

ml::BenchMarker::BenchMarker() :
	begin_if_primed([]() { return; }),
	end_if_primed([]() { return; })
{
    glGenQueries(1, &time_query);
}

ml::BenchMarker::~BenchMarker()
{
    glDeleteQueries(1, &time_query);
}

void ml::BenchMarker::begin()
{
    glBeginQuery(GL_TIME_ELAPSED, time_query);
}

void ml::BenchMarker::end()
{
    glEndQuery(GL_TIME_ELAPSED);
}

void ml::BenchMarker::log()
{
    results.emplace_back();
    glGetQueryObjectui64v(time_query, GL_QUERY_RESULT, &results.back());
    std::cout << results.back() << std::endl;
}

void ml::BenchMarker::log_all_to_file()
{
    if (!std::filesystem::exists("../logs/"))
    {
        std::filesystem::create_directories("../logs/");
    }

    auto now = std::chrono::system_clock::now();
    auto t_c = std::chrono::system_clock::to_time_t(now);
    auto local_time = std::localtime(&t_c);
    std::stringstream file_name;
    file_name << local_time->tm_hour << "_" << local_time->tm_min << "_" << local_time->tm_sec << "_" << local_time->tm_mday << "_" << local_time->tm_mon + 1 << "_" << 1900 + local_time->tm_year;

    std::cout << file_name.str() << std::endl;

    std::ofstream file_stream("../logs/" + file_name.str() + ".log", std::ios_base::app | std::ios_base::out);
    for (auto const& log : results)
    {
        file_stream << log << "\n";
    }
    file_stream.close();
}

void ml::BenchMarker::end_and_log()
{
    end();
    log();
}

void ml::BenchMarker::prime()
{
    results.clear();
    benchmarking = true;
    begin_if_primed = std::bind(&BenchMarker::begin, this);
    end_if_primed = std::bind(&BenchMarker::end_and_log, this);
    std::cout << "Logging primed" << std::endl;
}

void ml::BenchMarker::diffuse()
{
    benchmarking = false;
    begin_if_primed = []() { return; };
    end_if_primed = []() { return; };
    std::cout << "Logging diffused" << std::endl;
}
