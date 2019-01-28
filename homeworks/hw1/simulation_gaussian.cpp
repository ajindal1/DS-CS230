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
double run_simulation( vector< int > &processors, vector< int > &mmodules, int num_iterations, double iterations_threshold, int num_processors, int num_modules, vector< int > &gaussian_mean_proc, vector< int > &gaussian_var_proc ) {
    int min_iter = 100;
    int max_iter = 10000;
    double tot_proc = 0;
    double tot_time = 0;
    double prev_avg = 10;
    int iter = 0;
    int wait_times_count = 0;
    double value_change = DBL_MAX;
    double curr_avg = 0;

    // processor wait times
    unordered_map< int, int > processor_wait_time;

    // waiting processors
    unordered_set< int > waiting_processors;

    // map of processors that asked for each memory module in an iteration
    unordered_map< int, vector< int > > module_processors;

    for ( int i = 0; i < num_processors; i++ ) {
        // Assign wait time of max_wait to each processor
        // If a processors reduces to wait time of zero i.e.
        // it has waited max_wait cycles it is given preference
        processor_wait_time[ i ] = max_wait;
    }

    while ( iter++ < max_iter ) {
        for ( int i = 0; i < num_processors; i++ ) { // iterate over all processors
            if ( waiting_processors.find( i ) == waiting_processors.end() ) {
                // Pick a memory modules for only those processors which are not waiting
                // The waiting processors are already in the queue for their requested
                // memory module
                //
                // Pick a memory module for this processor using uniform distribution
                int proc_mean = gaussian_mean_proc[ i ];
                int proc_var = gaussian_var_proc[ i ];
                random_device rd;
                mt19937 e2( rd() );
                normal_distribution<> dist( proc_mean, proc_var );
                int mmodule_picked = ( int )( round( dist( e2 ) ) + num_modules ) % num_modules;
                if ( DEBUG ) cout << "Memory module picked is: " << mmodule_picked << endl;
                processor_wait_time[ i ] = max_wait;
                module_processors[ mmodule_picked ].push_back( i );
            }
        }

        // Go over module requests and assign them the processors
        for ( int i = 0; i < num_modules; i++ ) {
            if ( module_processors[ i ].size() == 0 ) continue; // The memory module has not been chosen by any processor
            vector< int > &proc_list = module_processors[ i ]; // The wait queue of the memory module

            int best_proc = -1;
            for ( int curr_proc : proc_list ) {
                // Assign it the best processor
                // If there is a processor with wait time <= 0 then that is assigned automatically
                // Otherwise a processor with minimum index is assigned
                if ( best_proc == -1 || curr_proc < best_proc ) {
                    best_proc = curr_proc;
                }
                if ( processor_wait_time[ curr_proc ] <= 0 ) {
                    best_proc = curr_proc;
                    break;
                }
            }

            module_processors[ i ].erase( remove( module_processors[ i ].begin(), module_processors[ i ].end(), best_proc ) );
            // update the total request time and total processing time
            tot_proc += 1;
            tot_time += max_wait - processor_wait_time[ best_proc ] + 1;
            // if a waiting processor has been picked for a memory module
            // update the waiting processors list
            if ( waiting_processors.find( best_proc ) != waiting_processors.end() ) {
                waiting_processors.erase( best_proc );
            }
            // add the remaining processors which are not assigned the memory module
            // to the waiting processors list and update the wait time by reducing
            // it by 1
            for ( int rem_proc : module_processors[ i ] ) {
                processor_wait_time[ rem_proc ] -= 1;
                waiting_processors.insert( rem_proc );
            }
        }
        // compute the average wait time
        curr_avg = tot_time/tot_proc;
        // if the change from previous iteration is within threshold then
        // continue else break
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
        vector< int > gaussian_mean_proc( num_processors, 0 );
        vector< int > gaussian_var_proc( num_processors, 0 );

        random_device rd; // Will be used to obtain a seed for the random number engine
        mt19937 gen( rd() ); // Standard mersenne_twister_engine seeded with rd()
        uniform_int_distribution<> dis( 0, num_mmodules - 1 );
        uniform_int_distribution<> dis2( 1, num_mmodules );
        // Generate the mean and variance for the normal distrbutions of each of the processors
        // a uniform distribution foe the mean between 0 to number of memory modules - 1 and
        // similarly variance between 1 to number of memory modules
        for ( int i = 0; i < num_mmodules; i++ ) {
            gaussian_mean_proc[ i ] = dis( gen );
            gaussian_var_proc[ i ] = dis2( gen );
        }

        int num_iterations = 100000;
        double iterations_threshold = 0.0002;
        double res = run_simulation( processors, mmodules, num_iterations, iterations_threshold, num_processors, num_mmodules, gaussian_mean_proc, gaussian_var_proc );
        results.push_back( res );
    }
    for ( int k = 0; k < module_nums.size(); k++ ) {
        cout << results[ k ] << " ";
    }
    cout << endl;
}
}
