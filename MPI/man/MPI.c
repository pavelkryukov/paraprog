#error File has not been written to be compiled

/// 29.02.2012

/*

Мы будем изучать системы с распределённой памятью.

Технология MPI (mpi-forum.org)

Кластер — набор вычислительных узлов, который может решать некоторую задачу.
MPI распределяет задачи между вычузлами.
Все процессы, над котороми запущен MPI, называется коммуникатором.

Коммуникатор по умолчанию: MPI_COMM_WORLD.

rank — номер процесса (0,1,..), который можно узнать изнутри программы.
*/

#include <mpi.h>

int main() (int argc, char** argv) {
    MPI_Init(...);             // first MPI function ever
    // printf("Hello world\n");   // please use only one process, common rank 0)
                               // message input-output code    
    MPI_Finalize();            // last MPI function
    return 0;
}

MPI_Common_size(&size); // size of communicator
MPI_Common_rank(&rank); // rank of process

// Send/Recv
MPI_SSend // дожидается завершения приёма и только после отпускает
MPI_Send  // дожидается начала приёма 
MPI_Recv  // дожидается получения сообщения
MPI_ASend // ничего не дожидается
MPI_ARecv // ничего не дожидается

MPI_Send(const void*, // pointer to data
         size_t,      // length
         MPI_type,    // type
         int dst,     // destination 
         int tag,     // tag to differ messages
         MPI_COMM_WORLD); // Communicator
         
MPI_Recv(void*,    // pointer to buffer (allocated)
         size_t,   // length
         MPI_type, // type
         int dst,  // source 
         int tag,  // tag to differ messages
         MPI_Status*,      // status of delivery
         MPI_COMM_WORLD);  // Communicator
         
MPI_Get_Count(status);
 
MPI_Barrier(); // блокировка до тех пор, пока все процессы не запустят Barrier

/// 14.03.2012 Коллективный обмен

MPI_Bcast // рассылка всем процессам \
должна быть запущена во всех процессах коммуникатора

MPI_Reduce // сбор данных со всех процессоров
MPI_Reduce(void*        from, 
           void*        to,
           unsigned     amount,
           MPI_DATATYPE type,
           MPI_OP       operation,
           unsigned     aggregator_rank,
           MPI_COMM     communicator);
               
MPI_All_Reduce // сбор данных во все процессоры.

MPI_Scatter // берёт буфер данных и распределяет равномерно по всем процессорам
/*
---------- => --------
0 | A B C     0 | A
1 |           1 | B
2 |           2 | C
*/

MPI_Gather // обратна MPI_Scatter

MPI_ScatterV // Если буфер не делится на все процессоры, можно использовать 

MPI_Allgather // рассылка на все процессоры
/*
---------- => --------
0 | A         0 | A B C D
1 | B         1 | A B C D
2 | C         2 | A B C D
3 | D         3 | A B C D
*/

/// 28.03.2012 Пользовательские типы.

MPI_Type_Contiguous(int count,              // Объявление типа-массива
                    MPI_Datatype  oldtype,      
                    MPI_Datatype* newtype);
                    
MPI_Type_Vector(int count,              // число блоков (число строк)
                int block,              // длина блока  (сколько столбцов)
                int stride,             // страйд       (число столбцов)
                MPI_Datatype  oldtype,  // старый тип
                MPI_Datatype* newtype); // новый тип