#include <iostream>

#include <set>
#include <memory>
#include <assert.h>

#include <list>
#include <vector>
#include <cmath>

#define IN_INTERVAL(x, a, b) (((x) <= (b)) && ((x) >= (a)))

template <typename T>
class Graph
{
public:
    Graph(int count_vertex)
    {
        graph = new T*[count_vertex];
        for (int i = 0; i < count_vertex; ++i)
        {
            graph[i] = new T[count_vertex];
        }
        
        size = count_vertex;
    }
    
    Graph(const Graph<T> &g)
    {
        size = g.size;
        graph = new T*[size];
        for (int i = 0; i < size; ++i)
        {
            graph[i] = new T[size];
            for (int j = 0; j < size; ++j)
                graph[i][j] = g.graph[i][j];
        }
    }
    
    ~Graph()
    {
        for (int i = 0; i < size; ++i)
            delete[] graph[i];
        delete[] graph;
    }
    
    void set_path(int i, int j, T v)
    {
        graph[i][j] = v;
        graph[j][i] = v;
    }
    
    T get_path(int i, int j) const
    {
        return graph[i][j];
    }
    
    T *get_paths(int i) const
    {
        return graph[i];
    }
    
    int get_count_vertex() const
    {
        return size;
    }
    
    void set_all(T v)
    {
        for (int i = 0; i < size; ++i)
            for (int j = 0; j < size; ++j)
                graph[i][j] = v;
    }
private:
    T **graph;
    int size;
};

class Pheramon
{
public:
    Pheramon() : use(false), pheramons(0)
    {}
    
    Pheramon(int value) : use(true), value(value), pheramons(0)
    {}
    
    Pheramon(const Pheramon &pheramon)
    {
        this->use = pheramon.use;
        
        this->value = pheramon.value;
        this->pheramons = pheramon.pheramons;
    }

    bool is_use()
    {
        return use;
    }
    
    void set_value(int value)
    {
        this->use = true;
        this->value = value;
    }
    
    int get_value()
    {
        return value;
    }
    
    double pheramons;
private:
    bool use;
    int value;
};

typedef Graph<Pheramon> GraphWithPheramon;

class GraphAnt
{
public:
    GraphAnt(int begin_vertex)
    {
        bvertex = begin_vertex;
        my_vertex = begin_vertex;
        visited_vertex.insert(begin_vertex);
        
        my_path_value = 0;
    }
    
    GraphAnt(const GraphAnt &ga)
    {
        bvertex = ga.bvertex;
        my_vertex = ga.my_vertex;
        visited_vertex = ga.visited_vertex;
        
        my_path_value = ga.my_path_value;
    }

    bool next(std::weak_ptr<GraphWithPheramon> graph, Graph<int> *delta_pheramon)
    {
        auto time_graph = graph.lock();
        assert(time_graph);
        
        Pheramon *paths = time_graph->get_paths(my_vertex);
        int paths_len = time_graph->get_count_vertex();
        
        std::vector<int> distribution;
        int distribution_sum = 0;
        
        bool stop = true;
        int best_vertex = -1; // , max_sweet_path = -1;
        for (int i = 0; i < paths_len; ++i)
        {
            bool is_in = visited_vertex.find(i) != visited_vertex.end();
            if (!paths[i].is_use() || is_in)
            {
                distribution.push_back(0);
                continue;
            }
            
            
            double path_koef = 100 / paths[i].get_value();
            
            // TODO ???
            int sweet = abs(int(path_koef) + paths[i].pheramons);
            
            distribution.push_back(sweet);
            distribution_sum += sweet;
            
            stop = false;
        }
    
        if (stop || distribution_sum <= 0)
            return false;
        
        for (int i = 0; i < paths_len; ++i)
            distribution[i] = (int) (round((double) distribution[i] / distribution_sum * 100));
        
        best_vertex = get_number_rand(distribution);
        
        Pheramon *pheramon = &paths[best_vertex];
        my_path_value += pheramon->get_value();
        
        double leave_pheramons = (double) 1 / my_path_value;

        int last_pheramons = delta_pheramon->get_path(my_vertex, best_vertex);
        delta_pheramon->set_path(my_vertex, best_vertex, (int) (last_pheramons + leave_pheramons));
        
        visited_vertex.insert(best_vertex);
        my_vertex = best_vertex;
        
        std::cout << "Pass to " << my_vertex << " with pheramons " << leave_pheramons << std::endl;
        return true;
    }
    
    int get_sum_path()
    {
        return my_path_value;
    }
    
    void clear_path()
    {
        my_path_value = 0;
        my_vertex = bvertex;
        
        visited_vertex.clear();
    }
private:
    int get_number_rand(std::vector<int> distribution)
    {
        int rand_value = rand() % 99 + 1;
        
        int val = 0;
        for (int i = 0; i < distribution.size(); ++i)
        {
            int new_val = val + distribution[i];
            if (IN_INTERVAL(rand_value, val, new_val))
                return i;
            
            val = new_val;
        }
        
        assert(0);
        return -1;
    }
    
    int bvertex;
    int my_vertex, my_path_value;
    
    std::set<int> visited_vertex;
};

int main()
{
    srand(time(0));
    
    const unsigned int COUNT_VERTEX = 4;
    std::shared_ptr<GraphWithPheramon> graph(new GraphWithPheramon(COUNT_VERTEX));
    graph->set_path(0, 1, 10);
    graph->set_path(0, 2, 6);
    
    graph->set_path(1, 3, 20);
    graph->set_path(2, 3, 40);
    
    graph->set_path(0, 3, 3);
    graph->set_path(1, 2, 10);
    
    const unsigned int COUNT_ANT = 100;
    
    std::vector<GraphAnt> ants;
    for (int i = 0; i < COUNT_ANT; ++i)
        ants.push_back(GraphAnt(rand() % COUNT_VERTEX));
    
    int min_path = 10000;
    Graph<int> delta_pheramon(COUNT_VERTEX);
    for (int timer = 0; timer < 100; ++timer)
    {
        delta_pheramon.set_all(0);
        
        for (int i = 0; i < COUNT_ANT; ++i)
        {
            while (ants[i].next(graph, &delta_pheramon)) {}
            
            int result = ants[i].get_sum_path();
            if (result < min_path)
                min_path = result;
        }
        
        std::cout << "find min " << min_path << " in " << timer << std::endl;
    
        for (int i = 0; i < graph->get_count_vertex(); ++i)
            for (int j = 0; j < graph->get_count_vertex(); ++j)
            {
                Pheramon pheramon = graph->get_path(i, j);
                if (!pheramon.is_use())
                    continue;
                
                Pheramon new_pheramon(pheramon.get_value());
                new_pheramon.pheramons = 0.6 * pheramon.pheramons + delta_pheramon.get_path(i, j);
                graph->set_path(i, j, new_pheramon);
            }
        
        for (int i = 0; i < COUNT_ANT; ++i)
            ants[i].clear_path();
    }
    
    return 0;
}