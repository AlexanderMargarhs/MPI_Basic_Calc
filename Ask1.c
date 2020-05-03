#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int main(int argc, char** argv)
{
	int My_Rank, processes, i, n, num, load, chunk, leftover, target, source, ans = 1;
	int tag1 = 50, tag2 = 60, tag3 = 70, tag4 = 80;
	int sum, min, max, min_chunk, max_chunk;
	double Fin_Sum, avg, val, var, Fin_Var, *D;
	int *X;
	//Initialize MPI.
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &My_Rank);
	MPI_Comm_size(MPI_COMM_WORLD, &processes);

	while(ans == 1)
	{
		sum = 0, Fin_Sum = 0, avg = 0, var = 0, Fin_Var = 0; //Reset the values to 0 after every .
		if (My_Rank == 0) //If My_Rank is 0 it means that it is the Parent process.
		{
    		printf("Please type the size of the array:\n"); //Read the size of the array.
    		scanf("%d", &n);
			X = (int *) malloc (n * sizeof(int ));
			for(i = 0; i < n; i++)
			{
				printf("Please type the number:"); //Read the array.
				scanf("%d", &num);
				X[i] = num;
			}
			load = (n / processes); //Divide the 'array' to equal parts for each process.
			leftover = (n % processes); //Find the leftover of the array.
			chunk = load; //Chunk of the array for each process.
			for(target = 1; target < processes; target++ )  //Send the equal chunks of the array to each of the processes.
			{
				MPI_Send(&load, 1, MPI_INT, target, tag1, MPI_COMM_WORLD); //Send the load to Child process.
				MPI_Send(&X[chunk], load, MPI_INT, target, tag1, MPI_COMM_WORLD); //Send the chunk of the array to Child processes.
				chunk += load; //Counter to send the correct chunk of the array.
			}
		}
		else
		{
			MPI_Recv(&load, 1, MPI_INT, 0, tag1, MPI_COMM_WORLD, &status);  //Receive load from Parent process.
			X = (int *) malloc (load * sizeof(int )); //Malloc the load of the array.
			MPI_Recv(&X[0], load, MPI_INT, 0, tag1, MPI_COMM_WORLD, &status); //Receive the chunk of the array from Parent process.  
		}
	
		if (My_Rank == 0)
		{
			for(i = 0; i < load; i ++) 
			{
				sum += X[i]; //Sum of the Parent process.
			}
			Fin_Sum = sum; //Add to the final sum.
			for(source = 1; source < processes; source ++ )  
			{
	    		MPI_Recv(&sum, 1, MPI_INT, source, tag2, MPI_COMM_WORLD, &status); //Receive sum from Child processes.
        		Fin_Sum += sum; //Add to final sum.
			}
			sum = 0;
			for(i = (n - leftover); i < n; i++)
			{
				sum += X[i]; //Sum of the leftover chunk.	
			}
			Fin_Sum += sum; //Add leftover sum to final sum.
			avg = Fin_Sum / (double) n; //Find the average.
			printf("Average: %.2lf\n", avg); //Print average.
    	}
		else
		{
			sum = 0; 
			for(i = 0; i < load; i ++)
			{								
				sum += X[i]; //Sum of Child processes.
			}
			MPI_Send(&sum, 1, MPI_INT, 0, tag2, MPI_COMM_WORLD); //Send the sum to Parent processes.
		}

		if (My_Rank == 0)  
		{
			for(target = 1; target < processes; target ++)
			{
				MPI_Send(&avg, 1, MPI_DOUBLE, target, tag3, MPI_COMM_WORLD); //Send average to Child processes.
			}
			var = 0;
			for(i = 0; i < load; i++)
			{
				val = (X[i] - avg); 
				var += (val * val); //Add to variance of current chunk.
			}
			Fin_Var += var; //Add to final Variance.
			for(source = 1; source < processes; source ++)
			{
				MPI_Recv(&var, 1, MPI_DOUBLE, source, tag4, MPI_COMM_WORLD, &status); //Receive chunk variance from Child processes.
				Fin_Var += var; //Add to final Variance.	
			}
			var = 0;
			for(i = (n - leftover); i < n; i++)
			{
				val = (X[i] - avg); 
				var += (val * val); //Add to variance of leftover chunk.
			}
			Fin_Var += var; //Add the leftover variance.
			Fin_Var = Fin_Var / n; //Calculate the final Variance.
			printf("Variance: %.2lf\n", Fin_Var); //Print the Final Variance.	
		}
		else
		{
			MPI_Recv(&avg, 1, MPI_DOUBLE, 0, tag3, MPI_COMM_WORLD, &status); //Receive average from Parent process.
			var = 0;
			for(i = 0; i < load; i++)
			{
				val = (X[i] - avg); 
				var += (val * val); //Add to variance of current chunk.
			}
			MPI_Send(&var, 1, MPI_DOUBLE, 0, tag4, MPI_COMM_WORLD); //Send the variance of the chunk to the Parent process.
		}

		if (My_Rank == 0)  
		{
			min = X[0];
			max = X[0];
			for(i = 1; i < load; i ++)
			{
				if(min > X[i])
				{
					min = X[i]; //Min of Parent process.
				}
				if(max < X[i])
				{
					max = X[i]; //Max of Parent process.
				}
			}
			for(i = (n - leftover); i < n; i++)
			{
				if(min > X[i])
				{
					min = X[i]; //Compare current min of leftover chunk.
				}
				if(max < X[i])
				{
					max = X[i]; //Compare current max of leftover chunk.
				}
			
			}
			for(source = 1; source < processes; source ++)
			{
				MPI_Recv(&min_chunk, 1, MPI_INT, source, tag3, MPI_COMM_WORLD, &status);  //Receive min from Child processes.  
				MPI_Recv(&max_chunk, 1, MPI_INT, source, tag3, MPI_COMM_WORLD, &status);  //Receive max from Child processes.
				if(min > min_chunk)
				{
					min = min_chunk;
				}
				if(max < max_chunk)
				{
					max = max_chunk;
				}
			}
			for(target = 1; target < processes; target ++)
			{
				MPI_Send(&min, 1, MPI_INT, target, tag3, MPI_COMM_WORLD); //Send min to Child processes.
				MPI_Send(&max, 1, MPI_INT, target, tag3, MPI_COMM_WORLD); //Send max to Child processes.
			}
		}
		else
		{
			min_chunk = X[0];
			max_chunk = X[0];
			for(i = 1; i < load; i ++)
			{
				if(min_chunk > X[i])
				{
					min_chunk = X[i];
				}
				if(max_chunk < X[i])
				{
					max_chunk = X[i];
				}
			}
			MPI_Send(&min_chunk, 1, MPI_INT, 0, tag3, MPI_COMM_WORLD); //Send min to Parent process.
			MPI_Send(&max_chunk, 1, MPI_INT, 0, tag3, MPI_COMM_WORLD);	//Send min to Parent process.
			MPI_Recv(&min, 1, MPI_INT, 0, tag3, MPI_COMM_WORLD, &status); //Receive min from Parent process. 
			MPI_Recv(&max, 1, MPI_INT, 0, tag3, MPI_COMM_WORLD, &status); //Receive max from Parent process. 
		}
	
		if(My_Rank == 0)
		{
			D = (double *) malloc (n * sizeof(double )); //Create array D.
			for( i = 0; i < load; i ++)
			{
				D[i] = ((X[i] - min) / ((double)max - (double)min)) * 100 ; //Calculate and save to array D the result for the Parent process.
			}
			chunk = load; //Counter for the array D.
			for(source = 1; source < processes; source ++)
			{
				for( i = chunk; i < (load + chunk); i ++)
				{
					MPI_Recv(&val, 1, MPI_DOUBLE, source, tag4, MPI_COMM_WORLD, &status); //Receive values from the Child processes.
					D[i] = val; //Save them to the array D.
				}
				chunk += load; //Up-count the counter for the next process.
			}
			
			for(i = (n - leftover); i < n; i++)
			{
				D[i] = ((X[i] - min) / ((double)max - (double)min)) * 100 ; //Calculate and save to array D the result for the leftover chunk.
			}
			for( i = 0; i < n; i ++)
			{
				printf("d[%d]:%.2lf\n", i, D[i]); //Display the array D.
			}
		}
		else
		{
			for( i = 0; i < load; i ++)
			{
				val = ((X[i] - min) / ((double) max - (double) min)) * 100 ; //Calculate the value for each chunk.
				MPI_Send(&val, 1, MPI_DOUBLE, 0, tag4, MPI_COMM_WORLD); //Send the value to the Parent process. 
			}
		}
		
		if(My_Rank == 0)
		{
			printf("1.Continue\n"); //Ask user if he would like to end the program.
			printf("2.Exit\n");
			scanf("%d", &ans);
			for(target = 1; target < processes; target ++)
			{
				MPI_Send(&ans, 1, MPI_INT, target, tag4, MPI_COMM_WORLD); //Send the answer to the Child processes. 
			}
			free(X); //Free the array from the parent process.
			free(D); //Free the array(It only exist in the parent process).
		}
		else
		{
			MPI_Recv(&ans, 1, MPI_INT, 0, tag4, MPI_COMM_WORLD, &status); //Receive the answer from the Parent process.
			free(X); //Free the array for the child processes.
		}
	}
	MPI_Finalize(); //End all processes.
    return 0;
}

    
