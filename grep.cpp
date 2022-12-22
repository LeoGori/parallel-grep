#include "grep.h"
#include <fstream>
#include "iostream"
#include "mpi.h"
#include <cstring>
#include <algorithm>

void grep::get_lines(std::vector<std::string> &input_string, const std::string &file_name) {

    int rank, size;

    // store the rank and the size of the communicator inside the variables rank and size
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // declare displacement and sendcounts arrays
    int *displacements = new int[size];
    int *sendcounts = new int[size];

    // declare the number of lines of the file
    int num_lines;
    // if the rank of the process is 0, then it is the master process,
    // and it will read the file and send the lines to the other processes
    if (rank == MASTER) {

        // read the file line by line into file_content, padding each line with spaces to make it
        // 80 characters long, and then adding a newline character at the end of each line
        std::ifstream file(file_name);

        std::string line;
        while (std::getline(file, line)) {
            line.resize(LINELENGTH + 1, ' ');
            input_string.push_back(line);
        }
        // send the number of lines to the other processes
        num_lines = input_string.size();

        // compute the array sendcounts and displacements
        int lines_per_process = num_lines / size;
        int remainder = num_lines % size;
        int sum = 0;
        for (int i = 0; i < size; i++) {
            sendcounts[i] = i < remainder ? (lines_per_process + 1) * (LINELENGTH + 1) : lines_per_process * (LINELENGTH + 1);
            displacements[i] = sum;
            sum += sendcounts[i];
        }
    }

    MPI_Bcast(&num_lines, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

    // determine the number of lines that each process will receive
    int lines_per_process = num_lines / size;
    int remainder = num_lines % size;

    // compute the chunk size for each process and the relative receive buffer
    int chunk_size = lines_per_process * (LINELENGTH + 1);
    if (rank < remainder) {
        chunk_size += (LINELENGTH + 1);
    }

    // allocate the receive buffer
    char *recv_buffer = new char[chunk_size];

    // convert input_string to char*
    char *input_string_char = new char[input_string.size() * (LINELENGTH + 1)];
    for (int i = 0; i < input_string.size(); i++) {
        std::strcpy(input_string_char + i * (LINELENGTH + 1), input_string[i].c_str());
    }

    // scatter the lines to the other processes
    MPI_Scatterv(input_string_char, sendcounts, displacements, MPI_CHAR,
                 recv_buffer, chunk_size, MPI_CHAR, MASTER, MPI_COMM_WORLD);

    input_string.clear();

    // convert the receive buffer into a vector of strings
    for (int i = 0; i < chunk_size; i += (LINELENGTH + 1)) {
        input_string.push_back(std::string(recv_buffer + i, (LINELENGTH + 1)));
    }


    // free the memory
    delete[] displacements;
    delete[] sendcounts;
    delete[] recv_buffer;
    delete[] input_string_char;

}

//implementation of search_string defined in grep.h
void grep::search_string(const std::vector<std::string> & input_strings,
                         const std::string & search_string,
                         lines_found &lines, unsigned &local_lines_number) {

    // get the rank of the process
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // find the lines that contain the search string
    for (unsigned i = 0; i < input_strings.size(); i++)
    {
        if (input_strings[i].find(search_string) != std::string::npos)
        {
            lines.push_back(std::make_pair(i, input_strings[i]));
        }
    }

    // store the number of lines that were found
    local_lines_number = lines.size();
    int send_local_found_chars_number = local_lines_number * (LINELENGTH + 1);

    // compose a vector of indices of the lines found
    int *local_lines_indices = new int[local_lines_number];
    char *lines_char = new char[local_lines_number * (LINELENGTH + 1)];

    for (int i = 0; i < local_lines_number; i++)
    {
        local_lines_indices[i] = (int) lines[i].first;
        std::strcpy(lines_char + i * (LINELENGTH + 1), lines[i].second.c_str());
    }

    int *recv_number_of_lines_found = new int[size];
    int *recv_number_of_chars_found = new int[size];
    int *displacements = new int[size];
    int *chars_displacements = new int[size];
    int recv_total_found_number;


    MPI_Gather(&local_lines_number, 1, MPI_INT,
               recv_number_of_lines_found, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

    if (rank == MASTER) {
        displacements[0] = 0;
        chars_displacements[0] = 0;
        recv_total_found_number = recv_number_of_lines_found[0];
        recv_number_of_chars_found[0] = recv_number_of_lines_found[0] * (LINELENGTH + 1);
        for (int i=1; i < size; i++) {
            recv_number_of_chars_found[i] = recv_number_of_lines_found[i] * (LINELENGTH + 1);
            displacements[i] = displacements[i-1] + recv_number_of_lines_found[i-1];
            chars_displacements[i] = chars_displacements[i - 1] + recv_number_of_chars_found[i - 1];
            recv_total_found_number = recv_total_found_number + recv_number_of_lines_found[i];
        }
    }

    int *receive_indices_buffer = new int[recv_total_found_number];

    // gather the indices of the lines found by all processes
    MPI_Gatherv(local_lines_indices, (int) local_lines_number, MPI_INT,
                receive_indices_buffer, recv_number_of_lines_found, displacements, MPI_INT,
                MASTER, MPI_COMM_WORLD);

    char *recv_string_buffer = new char[recv_total_found_number * (LINELENGTH + 1)];

    MPI_Gatherv(lines_char, send_local_found_chars_number, MPI_CHAR,
                recv_string_buffer, recv_number_of_chars_found, chars_displacements, MPI_CHAR,
                MASTER, MPI_COMM_WORLD);

    if (rank == MASTER) {
        // compose the vector of lines found
        lines.clear();
        for (int i = 0; i < recv_total_found_number; i++)
        {
            lines.push_back(std::make_pair(receive_indices_buffer[i], std::string(recv_string_buffer + i * (LINELENGTH + 1), (LINELENGTH + 1))));
        }
    }

    int chunk_size = input_strings.size();
    int *receive_chunk_buffer = new int[size];

    // gather the number of lines of each process
    MPI_Gather(&chunk_size, 1, MPI_INT,
               receive_chunk_buffer, 1, MPI_INT,
               MASTER, MPI_COMM_WORLD);


    if (rank == MASTER) {
        // print the indices of the lines found by all processes
        int global_chunk_size = 0;
        for (int i = 0; i < size; i++) {
            for (int j=0; j < recv_number_of_lines_found[i]; j ++) {
                lines[j + displacements[i]].first = lines[j + displacements[i]].first + global_chunk_size + 1;
            }
            global_chunk_size += receive_chunk_buffer[i];
        }
    }

    // free the memory
    delete[] local_lines_indices;
    delete[] lines_char;
    delete[] receive_indices_buffer;
    delete[] recv_string_buffer;
    delete[] recv_number_of_lines_found;
    delete[] recv_number_of_chars_found;
    delete[] displacements;
    delete[] chars_displacements;

}

// trim from end (in place)
std::string grep::rtrim(const std::string& str)
{
    size_t first = 0;
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

//implementation of print_result defined in grep.h
void grep::print_result(const lines_found &lines) {
//     get the rank of the process and the size of the communicator
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == MASTER) {
        // print the content of the vector of lines found
        for (int i = 0; i < lines.size(); i++) {
            std::cout << lines[i].first << ":" << rtrim(lines[i].second) << std::endl;
        }
    }
}