
//INPUT HEADERS
#include "translate_from_input.hpp"
#include "input_to_network.hpp"
#include"structure_defs.hpp"

//OUTPUT HEADERS
#include "printout_network.hpp"
#include "printout_others.hpp"


#include "readin_data.hpp"
#include "deletes.hpp"
#include "Tarjan.hpp"
#include "metagraph.hpp"
#include "insert.hpp"




using namespace std;

int main(int argc, char *argv[])
{       
	clock_t q, t;
    
    
    //Check if valid input is given
    if ( argc < 3) { cout << "INPUT ERROR:: 5 inputs required. First:  filename \n Second: file with component id \n  Third: Set of changed edges \n Fourth: Number of threads \n";}
    //Check to see if file opening succeeded
     ifstream the_file1 ( argv[1] ); if (!the_file1.is_open() ) { cout<<"INPUT ERROR:: Could not open  graph file\n";}
      ifstream the_file2 ( argv[2] ); if (!the_file2.is_open() ) { cout<<"INPUT ERROR:: Could not open SCC file\n";}
     ifstream the_file3 ( argv[3] ); if (!the_file3.is_open() ) { cout<<"INPUT ERROR:: Could not open changed file\n";}
    // ifstream the_file4 ( argv[4] ); if (!the_file3.is_open() ) { cout<<"INPUT ERROR:: Could not openchanged  file\n";}
    int nodes=atoi(argv[4]);
    int p = atoi(argv[5]);  //total number of threads per core
  
  
   //===Read in Network to the Processors
    //Back edges are not explicitly stored--we will convert as needed
     q=clock();
    A_Network X;
    readin_network(&X,argv[1],nodes);
    
    vector<int_int> SCC_ID; //stores the SCC ids of the vertices
    int_int dummy;
    dummy.first=-1;
    dummy.second=-1;
    SCC_ID.resize(nodes,dummy); //this is required for rodering as per the sccids
    vector<int> SCCx;
    SCCx.resize(nodes,-1); //this is to look up the ids of the nodes as per the index
    readin_SCC(&SCC_ID, &SCCx, argv[2]);
   
   //===Read in changed edges
    //Reads the set of changed edges and then keeps the ones with at least one neighbor in its processor
  vector <int_int> inserts;
    inserts.clear();
    vector<int_int> deletes;
    deletes.clear();
    
    //We assume that the inserts and deletes are sorted
    readin_changes(&inserts, &deletes, argv[3]);
    q=clock()-q;
    printf("Total Time for Reading  %f \n", ((float)q)/CLOCKS_PER_SEC);
    //========= Reading Done ===//
  
    
    t=clock();
    //==Apply Deletes ===//
    //Find valid deletes
    clock_t q3;
      q3=clock();
    vector<int> delete_nodes; //these are vertices associated with valid deletes
    delete_nodes.clear();
    find_valid_deletes(&X, &SCC_ID, &deletes,&delete_nodes,p);
    
  //  print_vector(delete_nodes);
    
    //Obtain new SCCs by running Tarjan from multiple sources
   // if(delete_nodes.size()>1)
    { multiple_tarjan(&X, &SCC_ID, &SCCx, &delete_nodes,p);}
    q3=clock()-q3;
    printf("Total Time for Deletion %f \n", ((float)q3)/CLOCKS_PER_SEC);
 //   print_vector(SCC_ID);
  //  print_vector(SCCx);
    
    //==Apply Deletes Done ===//
    
    //printf("Apply deletes doen \n");
    //==Create Metagraph and Fork Based Numbering ===//
    //Create meta graph of unique SCC_IDs
    clock_t q2;
      q2=clock();
    A_Network M;
    vector<bool> is_source; //markes if a node is a source: has no in_degree
    create_metagraph(&X,&M,&SCC_ID, &SCCx, &is_source, p);
  //  print_network(M);
   // print_vector(is_source);
  //  print_vector(is_fork);
    
    //first pass at finding inserts
    //find_valid_inserts1(&M, &SCC_ID, &inserts, &valid_insert,p);
    
    //Number the vertices based on their forks
    vector< vector<int> > fork; //stores the nearest fork(s) nodes with  indgree >1
    vector< vector<int> > branch; //the next node after the fork that indictes the branch
    vector< vector<int> > level; //distance from fork where fork node is distance 1
    vector< vector<int> > source;//gives the soruce vertex of the path
    forkbased_numbering(&M, &is_source, &source, &fork, &branch, &level,p);
    
    q2=clock()-q2;
    printf("Total Time for Metagraph Creation %f \n", ((float)q2)/CLOCKS_PER_SEC);
   /* for(int i=0;i<M.size();i++)
    {
        printf("%d :: \n",i);
        
        for(int j=0;j<fork[i].size();j++)
        {
            printf("%d  %d %d %d..\n", source[i][j], fork[i][j], branch[i][j],level[i][j]);
        }
    }*/
     //==Create Metagraph and Fork Based Numbering Done ===//

    //==Apply Inserts ===//
    clock_t q1;
    q1=clock();
    vector<int> valid_inserts; //these are vertices associated with valid deletes
    valid_inserts.resize(inserts.size(),-1); //initialized to false
    vector<int> newSCC;
    newSCC=SCCx;
    find_valid_inserts(&M,&SCCx,&inserts,&valid_inserts, &source, &fork, &branch, &level,p);
    
  /*  for(int i=0;i<inserts.size();i++)
    { printf("%d %d %d \n", inserts[i].first, inserts[i].second, valid_inserts[i]);}*/
    
    connect_SCCs(&M,&SCCx,&newSCC,&inserts,&valid_inserts,&fork, &branch,p);
    q1=clock()-q1;
    printf("Total Time for Insert  %f \n", ((float)q1)/CLOCKS_PER_SEC);
    //==Apply Inserts Done===//
    
    t=clock()-t;
    printf("Total Time for Update  %f \n", ((float)t)/CLOCKS_PER_SEC);
    
    //print_vector(newSCC);

	return 0;
}//end of main
	
