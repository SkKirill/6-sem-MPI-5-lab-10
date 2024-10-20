#include<stdio.h>
#include<mpi.h>
#include<time.h>
#include <iostream>
#include <random>
#include <iomanip>
#include <limits>

using namespace std;

void createArr(int* row, int size)
{
    const int min = 0, max = 20;
    random_device r;
    default_random_engine e(r());
    uniform_int_distribution<int> digit(min, max);
    for (int i = 0; i < size; i++)
    {
        row[i] = digit(e);
    }
}

void printArr(int* row, int size, string letter, int rank, string rowOrCal)
{
    cout << rowOrCal << " " << rank << " of matrix " << letter << ":";
    for (int i = 0; i < size; i++)
    {
        cout << setw(4) << row[i];
    }
    cout << endl;
}

int findMaxElem(int* row, int size) {
    int max = row[0];
    for (int i = 1; i < size; i++)
    {
        if (row[i] > max)
        {
            max = row[i];
        }
    }
    return max;
}

int main(int argc, char** argv)
{
    const int dimentions = 1;
    const int tag = 5;

    int rank, size;
    int source, destination; // отправитель получатель

    int dims[dimentions]{}, periods[dimentions]{}, coords[dimentions];// массивы для хранения размерности, периодичности и координат.

    MPI_Comm comm; // создаем переменную под новый коммуникатор
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    for (int i = 0; i < dimentions; i++) // задание размерности и переодичности
    {
        dims[i] = 0;
        periods[i] = 1; // связь последнего и первого
    }

    MPI_Dims_create(size, dimentions, dims); // создание рамерности
    MPI_Cart_create(MPI_COMM_WORLD, dimentions, dims, periods, 0, &comm); // создаем коммуникатор

    MPI_Cart_coords(comm, rank, dimentions, coords); 
    MPI_Cart_shift(comm, 0, -1, &source, &destination); 
         // определение координаты соседних процессов и размером шага смещения(положительным или отрицательным).

    int* rowA = new int[size];
    int* colB = new int[size];
    int* masArr = new int[size];

    createArr(rowA, size);
    printArr(rowA, size, "A", rank, "Row");
    createArr(colB, size);
    printArr(colB, size, "B", rank, "Col");

    for (int i = 0; i < size; i++) {
        int* sumMas = new int[size];
        int* maxKMas = new int[size];
        for (int j = 0; j < size; j++) {
            for (int k = 0; k < size; k++) {
                int temp = rowA[k] + colB[k];
                sumMas[k] = temp;
            }
            maxKMas[j] = findMaxElem(sumMas, size);
        }
        masArr[i] = findMaxElem(sumMas, size);
        if (i < size - 1) {
            MPI_Sendrecv_replace(colB, size, MPI_INT, destination, tag, source, tag, comm, &status); 
                        // для смещения colB вдоль направления процессов 
                        // (обмен между всеми процессами сначала отправляет затем приничает каждый процесс это делает)
        }

        delete[] sumMas;
        delete[] maxKMas;
    }

    int res = findMaxElem(masArr, size);
    cout << "Result " << rank << " : " << res << endl;

    delete[] rowA;
    delete[] colB;
    delete[] masArr;
    MPI_Comm_free(&comm);
    MPI_Finalize();
    return 0;
}
