#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <filesystem>
#include <functional>

namespace ml
{
    class BenchMarker
    {
    private:
        bool benchmarking = false;
        unsigned int time_query = 0;

        std::vector<uint64_t> results;


    public:
        std::function<void()> begin_if_primed;
        std::function<void()> end_if_primed;
    
        BenchMarker();
        BenchMarker(BenchMarker const& other) = delete;
        BenchMarker& operator=(BenchMarker const& other) = delete;
        BenchMarker(BenchMarker&& other) = delete;
        BenchMarker& operator=(BenchMarker&& other) = delete;
        ~BenchMarker();

        void begin();

        void end();

        void log();

        void log_all_to_file();

        void end_and_log();

        void prime();

        void diffuse();

        explicit operator bool() const { return benchmarking; }
    };
}