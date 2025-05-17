#include "object_pool.hpp"

#include <chrono>
#include <cstdio>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

using namespace opo;

struct Object
{
    int a;
    int b;
    int c;
    int d;
    int e;
    int f;
    int g;
    double h;
};

int main(int argc, const char *const *argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <0: object_pool, 1: std::make_shared>\n" << std::endl;
        return 1;
    }

    auto choice = atoi(argv[1]);

    if (choice == 0)
    {
        std::cout << "Using object_pool" << std::endl;
    }
    else
    {
        std::cout << "Using std::make_shared" << std::endl;
    }

    ObjectPool<Object> pool(256);

    auto beforeTime = std::chrono::steady_clock::now();
    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i)
    {
        threads.emplace_back([&pool, choice]() {
            for (int i = 0; i < 512; ++i)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                std::shared_ptr<Object> obj;
                if (choice == 0)
                {
                    // std::cout << "Get obj from pool" << std::endl;
                    obj = get_shared_pointer_from(pool);
                }
                else
                {
                    // std::cout << "Create obj by make_shared" << std::endl;
                    obj = std::make_shared<Object>();
                }
                obj->a = i;
                obj->b = i;
                obj->c = i;
                obj->d = i;
                obj->e = i;
                obj->f = i;
                obj->g = i;
                obj->h = i;
            }
        });
    }

    for (auto &t : threads)
    {
        t.join();
    }

    auto afterTime = std::chrono::steady_clock::now();
    double durationMillsecond = std::chrono::duration<double, std::milli>(afterTime - beforeTime).count();
    std::cout << "Use " << durationMillsecond << " ms" << std::endl;

    return 0;
}
