#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <string>
#include <functional>

namespace ml
{
    class BenchMarker
    {
    private:
        bool benchmarking = false;
        unsigned int time_query;

        std::vector<uint64_t> results;


    public:
        std::function<void()> begin_if_primed;
        std::function<void()> end_if_primed;
    
        BenchMarker() : begin_if_primed([]() { return; }), end_if_primed([]() { return; })
        {
            glGenQueries(1, &time_query);
        }

        ~BenchMarker()
        {
            glDeleteQueries(1, &time_query);
        }

        void begin()
        {
            glBeginQuery(GL_TIME_ELAPSED, time_query);
        }

        void end()
        {
            glEndQuery(GL_TIME_ELAPSED);
        }

        void log()
        {
            results.emplace_back();
            glGetQueryObjectui64v(time_query, GL_QUERY_RESULT, &results.back());
            std::cout << results.back() << std::endl;
        }

        void log_all_to_file()
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

        void end_and_log()
        {
            end();
            log();
        }

        void prime()
        {
            results.clear();
            benchmarking = true;
            begin_if_primed = std::bind(&BenchMarker::begin, this);
            end_if_primed = std::bind(&BenchMarker::end, this);
        }

        void diffuse()
        {
            benchmarking = false;
            begin_if_primed = []() { return; };
            end_if_primed = []() { return; };
        }

        explicit operator bool() const { return benchmarking; }
    };
}