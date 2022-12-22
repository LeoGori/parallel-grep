#include <mpi.h>
#include "grep.h"
#include <filesystem>
#include <iostream>

int main (int argc, char * argv[])
{
    std::string file_name;
    std::string search_string;

    if (argc != 3) {
        std::cout << "Expected 2 inputs, got " << argc - 1 << std::endl;
        return 0;
    }
    else {
        file_name = argv[2];
        search_string = argv[1];
    }

    std::cout << file_name << std::endl;

    grep::lines_found local_filtered_lines;
    unsigned local_lines_number;
    std::vector<std::string> input_lines;

    MPI_Init(&argc, &argv);

    grep::get_lines(input_lines, file_name);
    grep::search_string(input_lines, search_string, local_filtered_lines, local_lines_number);
    grep::print_result(local_filtered_lines);

    MPI_Finalize();

    return 0;
}
