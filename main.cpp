#define NDEBUG

#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <windows.h>

#include "SparseSet.h"
#include "Registry.h"
#include "Component.h"

void SparseTest();
void ViewBenchmark(const int ecount, const int rep_num);
void RegistryBasicTest();

struct Tag {};

struct Position
{
    int x, y;
};

struct Health
{
    Health(int x) : hp(x)
    {

    }

    int hp;
};

void setColor(int color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

/*
    ZOPTYMALIZUJ SWOJ KOD
*/

using namespace aecs;

int main()
{
    /*Registry world;
    auto log_info = [&](Entity ent)
    {
        auto pos = world.try_get<Position>(ent);
        auto hel = world.try_get<Health>(ent);
        auto tag = world.try_get<Tag>(ent);

        printf("Entity %i.%i: ", ent.index, ent.version);
        if(pos)
        {
            setColor(10);
            printf(" Position %i %i, ", pos->x, pos->y);
        }
        if(hel) 
        {
            setColor(12);
            printf(" Health %i, ", hel->hp);
        }
        if(tag) 
        {
            setColor(14);
            printf(" Tag ");
        }
        setColor(7);
        printf("\n");
    };

    auto roll_prc = [](int percentage)
    {
        return (rand() % 100 + 1) < percentage;
    };

    std::cout << "\n\nEntire world's view: \n";
    for(int i = 0; i < 20; i++)
    {
        Entity ent = world.create();
        if(roll_prc(50)) world.add<Position>(ent, i * 2, i * 3);
        if(roll_prc(30)) world.add<Tag>(ent);
        if(roll_prc(40)) world.add<Health>(ent, i+20);

        log_info(ent);
    }

    std::cout << "\n\nWorld's position view: \n";
    for(const auto& entity : world.view<Position>())
    {
        log_info(entity);
    }

    std::cout << "\n\nWorld's health view: \n";
    for(const auto& entity : world.view<Health>())
    {
        log_info(entity);
    }

    std::cout << "\n\nWorld's position and health view: \n";
    for(const auto& entity : world.view<Position, Health>())
    {
        log_info(entity);
    }

    std::cout << "\n\nWorld's tag and health view: \n";
    for(const auto& entity : world.view<Tag, Health>())
    {
        log_info(entity);
    }*/
    
    //SparseTest();
    //RegistryBasicTest();
    ViewBenchmark(1000000, 1);

    std::cin.get();
}

void SparseTest()
{
    SparseSet<int> mySet(nullptr);

    for(size_t i = 0; i < 10; i++)
        mySet.insert(i, {i,0});

    std::cout << "SparseSet contains: \n";
    for(size_t i = 0; i < mySet.get_entities().size(); i++)
    {
        Entity ent = mySet.get_entities()[i];
        if(ent.isValid())
        {
            std::cout << mySet.get(ent) << ", ";
        }
    }
    std::cout << std::endl;

    for(int j = 0; j < 10; j++)
    {
        const size_t index = j;
        std::cout << "\nRemoving entity " << index << std::endl;

        mySet.remove({index,0});

        std::cout << "SparseSet contains: \n";
        for(size_t i = 0; i < mySet.get_entities().size(); i++)
        {
            Entity ent = mySet.get_entities()[i];
            if(ent.isValid())
            {
                std::cout << mySet.get(ent) << ", ";
            }
        }
    }

    std::cout << "\nAllocated pages: " << mySet.count_allocated_pages();
}

void ViewBenchmark(const int ecount, const int rep_num)
{
    srand(time(NULL));
    Registry world;

    for(int i = 0; i < ecount; i++)
    {
        Entity ent = world.create();
        world.add<Position>(ent, i+1, i*3+1);
        world.add<Tag>(ent);
    }

    float sumc = 0;
    std::chrono::microseconds sum{0};
    for(int i = 0; i < rep_num; i++)
    {
        auto start = std::chrono::steady_clock::now(); 
        auto view  = world.view<Position>();

        view.each([&](Position& position)
        {
            float calculations = float(position.x) / float(position.y);
            sumc += calculations;
        });

        auto end = std::chrono::steady_clock::now();
        sum += std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    }

    std::cout << sumc << std::endl;
    std::cout << "Avg Time taken to iterate over " << ecount << " entities: " 
                  << float(sum.count()) / float(rep_num) 
                  << " microseconds.\nAverage taken from " << rep_num << " iterations.\n";
}

void RegistryBasicTest()
{
    Registry world;
    auto log_info = [&](Entity ent)
    {
        auto pos = world.try_get<Position>(ent);
        auto hel = world.try_get<Health>(ent);
        auto tag = world.try_get<Tag>(ent);

        printf("Entity %i.%i: ", ent.index, ent.version);
        if(pos) printf(" Position %i %i, ", pos->x, pos->y);
        if(hel) printf(" Health %i, ", hel->hp);
        if(tag) printf(" Tag ");
        printf("\n");
    };

    std::cout << "Testing inserting components: \n";

    std::vector<Entity> entities;
    for(int i = 0;   i < 10; i++)
        entities.push_back(world.create());
    

    for(const auto& entity : entities)
    {
        int i = entity.index;
        world.add<Position>(entity, i, i);
        if(rand() % 2 == 0) world.add<Health>(entity, i * 2);
        if(rand() % 3 == 0) world.add<Tag>(entity);

        log_info(entity);
    }

    std::cout << "\n\nTesting removing components: \n";
    for(int i = 0; i < 5; i++)
    {
        Entity ent(rand() % entities.size(), 0);
        const int r = rand() % 3;
        std::string s = (r == 0 ? "Position" : (r == 1 ? "Health" : "Tag"));
        printf("Removing %s from entity %i:\n", s.c_str(), ent.index);

        log_info(ent);
        if(r == 0) world.remove<Position>(ent);
        else if(r == 1) world.remove<Health>(ent);
        else if(r == 2) world.remove<Tag>(ent);
        log_info(ent);
        printf("\n");
    }

    std::cout << "\n\nTesting removing entities: \n";
    for(int i = 0; i < 3; i++)
    {
        Entity ent(rand() % entities.size(), 0);
        printf("Removing entity %i.\n", ent.index);
        world.remove(ent);
    }

    std::cout << "\n\nTesting adding new entities: \n";
    for(int j = 0; j < 4; j++)
    {
        Entity entity = world.create();
        entities.push_back(entity);

        int i = entity.index;

        world.add<Position>(entity, i, i);
        if(rand() % 2 == 0) world.add<Health>(entity, i * 2);
        if(rand() % 3 == 0) world.add<Tag>(entity);

        log_info(entity);
    }

    std::cout << "\n\nLogging info: \n";
    for(const auto& entity : entities)
    {
        log_info(entity);
    }

    std::cout << "\n\nPosition view: \n";
    for(const auto& entity : world.view<Position>())
    {
        auto& pos = world.get<Position>(entity);
        printf("Entity %i position is: x = %i, y = %i\n", entity.index, pos.x, pos.y);
    }

    std::cout << "\n\nHealth view: \n";
    for(const auto& entity : world.view<Health>())
    {
        auto& hp = world.get<Health>(entity);
        printf("Entity %i health is: %i\n", entity.index, hp.hp);
    }

    std::cout << "\n\nTag view: \n";
    for(const auto& entity : world.view<Tag>())
    {
        printf("Entity %i has a tag.\n", entity.index);
    }

    std::cout << "\n\nTag, Position and Health view: \n";
    for(const auto& entity : world.view<Position, Tag, Health>())
    {
        auto& pos = world.get<Position>(entity);
        printf("Entity %i has a tag and its position is: x = %i, y = %i\n", entity.index, pos.x, pos.y);
    }

    std::cout << "\n\nPosition and health view using .each(): \n";
    world.view<Position, Health>().each([](Position& pos, Health& hp)
    {
        printf("Entity x: hp = %i, x = %i, y = %i\n", hp.hp, pos.x, pos.y);
    });
}