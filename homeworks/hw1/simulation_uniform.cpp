#include <vector>
#include <iostream>
#include <cfloat>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

#define DEBUG 0
int wait_time_limit = 7;
const int max_wait = 3;

// run the simulation for given number of iterations
double run_simulation( vector< int > &processors, vector< int > &mmodules, int num_iterations, double iterations_threshold, int num_processors, int num_modules ) {
    int min_iter = 100;
    int max_iter = 10000;
    double tot_proc = 0;
    double tot_time = 0;
    double prev_avg = 10;
    int iter = 0;
    int wait_times_count = 0;
    double value_change = DBL_MAX;
    double curr_avg = 0;
    random_device rd; // Will be used to obtain a seed for the random number engine
    mt19937 gen( rd() ); // Standard mersenne_twister_engine seeded with rd()
    uniform_int_distribution<> dis( 0, num_modules - 1 );

    // processor wait times
    unordered_map< int, int > processor_wait_time;

    // waiting processors
    unordered_set< int > waiting_processors;

    // map of processors that asked for each memory module in an iteration
    unordered_map< int, vector< int > > module_processors;

    for ( int i = 0; i < num_processors; i++ ) {
        processor_wait_time[ i ] = max_wait;
    }

    while ( iter++ < max_iter ) {
        for ( int i = 0; i < num_processors; i++ ) { // iterate over all processors
            if ( waiting_processors.find( i ) == waiting_processors.end() ) {
                // Pick a memory module for this processor
                int mmodule_picked = dis( gen );
                if ( DEBUG ) cout << "Memory module picked is: " << mmodule_picked << endl;
                processor_wait_time[ i ] = max_wait;
                module_processors[ mmodule_picked ].push_back( i );
            }
        }

        // Go over module requests and assign them the processors
        for ( int i = 0; i < num_modules; i++ ) {
            if ( module_processors[ i ].size() == 0 ) continue;
            vector< int > &proc_list = module_processors[ i ];
            int best_proc = -1;
            for ( int curr_proc : proc_list ) {
                if ( best_proc == -1 || curr_proc < best_proc ) {
                    best_proc = curr_proc;
                }
                if ( processor_wait_time[ curr_proc ] <= 0 ) {
                    best_proc = curr_proc;
                    break;
                }
            }

            module_processors[ i ].erase( remove( module_processors[ i ].begin(), module_processors[ i ].end(), best_proc ) );
            tot_proc += 1;
            tot_time += max_wait - processor_wait_time[ best_proc ] + 1;
            if ( waiting_processors.find( best_proc ) != waiting_processors.end() ) {
                waiting_processors.erase( best_proc );
            }

            for ( int rem_proc : module_processors[ i ] ) {
                processor_wait_time[ rem_proc ] -= 1;
                waiting_processors.insert( rem_proc );
            }
        }

        curr_avg = tot_time/tot_proc;
        if ( iter > min_iter && abs( ( curr_avg - prev_avg )/prev_avg ) < 0.0002 ) {
            if ( DEBUG ) cout << "Converged!" << endl;
            prev_avg = curr_avg;
            break;
        }
        prev_avg = curr_avg;
    }

    return curr_avg;
}

int main() {
    vector< int > processor_nums = { 2, 4, 8, 16, 32, 64 };
    vector< int > module_nums = { 1, 2, 4, 8, 12, 20, 30 }; //, 50, 100, 200, 500, 1000, 2048 };
for ( int i = 0; i < processor_nums.size(); i++ ) {
    vector< double > results = {};
    for ( int j = 0; j < module_nums.size(); j++ ) {
        int num_processors = processor_nums[ i ];
        int num_mmodules = module_nums[ j ];
        vector< int > processors( num_processors, 0 );
        vector< int > mmodules( num_mmodules, 0 );
        int num_iterations = 100;
        double iterations_threshold = 0.0002;
        double res = run_simulation( processors, mmodules, num_iterations, iterations_threshold, num_processors, num_mmodules );
        results.push_back( res );
    }
    for ( int k = 0; k < module_nums.size(); k++ ) {
        cout << results[ k ] << " ";
    }
    cout << endl;
}
}
