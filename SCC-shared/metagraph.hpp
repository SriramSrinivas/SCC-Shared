


#ifndef metagraph_hpp
#define metagraph_hpp

#include "printout_others.hpp"
#include "translate_from_input.hpp"
#include "input_to_network.hpp"
#include"ADJ/create_network.hpp"

struct int_four {
    int id;
    int fork;
    int branch;
    int level;
};


void create_metagraph(A_Network *X,A_Network *M,vector<int_int> *SCC_ID,  vector<int> *SCCx, vector<bool> *is_source,  int p)
{
  
    sort(SCC_ID); //SCC_ID is sorted as per ID number
    //Get starting index of same value SCC
    vector<int> indices;
    indices.clear();
    
    int cur_id=-1;
    int i=0;
    while(i<SCC_ID->size())
    {
       while(cur_id==SCC_ID->at(i).first)
       {i++;
        if(i==SCC_ID->size()){break;}
       }
        
        if(i==SCC_ID->size()){break;}
        indices.push_back(i);
        cur_id=SCC_ID->at(i).first;
        i++;
        
    }
    indices.push_back(SCC_ID->size());
   // print_vector(indices);
    
    int maxID=SCC_ID->at(SCC_ID->size()-1).first+1;
    create_Network(maxID,0,M);
    
   // printf("%d --\n", maxID);
    //Initialize sources
    is_source->resize(maxID,true); //initally mark all vertices as sources
    
    //Loop over the values in the indices to create meta graph
#pragma omp parallel for schedule(dynamic) num_threads(p)
    for(int i=0;i<indices.size()-1;i++)
    {
        for(int j=indices[i];j<indices[i+1];j++)
        {
            
          int node=SCC_ID->at(j).second;
            int node_id=SCC_ID->at(j).first;
        //go through neighbors of node
        for(int k=0;k<X->at(node).ListW.size();k++)
        {
            //get neighbor
            int neigh=X->at(node).ListW[k].first;
            
            //Ignore if edge was deleted
            if(X->at(node).ListW[k].second==-1)
            {continue;}
            
            //get SCC ID of neighbors
            int neigh_id=SCCx->at(neigh);
            
            
            if(node_id!=neigh_id) //Avoid self loops
            {
            //add the value to M
            int_int dummy;
            dummy.first=neigh_id;
            dummy.second=1;
            M->at(node_id).ListW.push_back(dummy);
                
            is_source->at(neigh_id)=false; //If a neighbor then its not source
            }
            
        }//end of k
        }//end of for j
        
    }//end of for i
    
    //Sort the ListW to get unique values
    vector<int_double> temp;
    #pragma omp parallel for schedule(dynamic) num_threads(p)
    for(int i=0;i<M->size();i++)
    {
        //mark source as false if no indegree
        if(M->at(i).ListW.size()==0)
        {is_source->at(i)=false;}
        
        if(M->at(i).ListW.size()>1)
        {
        temp.clear();
        temp=M->at(i).ListW;
        sort(&temp);
        M->at(i).ListW=unique(temp);
        }//end of if
       
    }//end of for
    
    
    indices.clear();
    temp.clear();
    return;
}
/** End of Function ****/



void forkbased_numbering(A_Network *M, vector<bool> *is_source, vector< vector<int> > *source, vector<vector<int> > *fork,  vector<vector<int> > *branch,  vector<vector<int> > *level,int p)
{
    
    //Get how many sources are there
    
    vector<int> current_sources;
    current_sources.clear();
    for(int i=0;i<is_source->size();i++)
    {
        if(is_source->at(i)==1)
        {current_sources.push_back(i);}
    }
    int num_source=current_sources.size();
    //Initialize the vectors to have information about at most two sources
    vector<int> dummyV;
    dummyV.resize(num_source,-1);
    fork->resize(M->size(),dummyV);
    branch->resize(M->size(),dummyV);
    level->resize(M->size(),dummyV);
    source->resize(M->size(), dummyV);

    
    
#pragma omp parallel for schedule(dynamic) num_threads(p)
    for(int i=0;i<current_sources.size();i++)
    {
        //do traversal if vertex is fork
        
            //Set value for mysrc
        int mysrc=current_sources[i];
        
            int_four node;
            node.id=mysrc;
            node.fork=mysrc;
            node.branch=mysrc;
            node.level=0;
            
            //Add values to fork, branch and level
            fork->at(node.id)[i]=node.fork;
            branch->at(node.id)[i]=node.branch;
            level->at(node.id)[i]=node.level;
            source->at(node.id)[i]=mysrc;
            
        
            queue<int_four> myqueue;
            myqueue.push(node);
            
            while(!myqueue.empty())
            {
                int_four this_node=myqueue.front();
                myqueue.pop();
                
                for(int j=0;j<M->at(this_node.id).ListW.size();j++)
                {
                    //Get values of neighbor
                    int_four neigh;
                    neigh.id=M->at(this_node.id).ListW[j].first;
                    neigh.fork=this_node.fork;
                    neigh.level=this_node.level+1;
                    if(M->at(this_node.id).ListW.size()>1 || (this_node.id==mysrc))
                    {
                        neigh.fork=this_node.id;
                        neigh.branch=neigh.id;
                    }
                    else
                    {   neigh.fork=this_node.fork;
                        neigh.branch=this_node.branch;}
                    
                    //Add values to fork, branch and level
                    fork->at(neigh.id)[i]=neigh.fork;
                    branch->at(neigh.id)[i]=neigh.branch;
                    level->at(neigh.id)[i]=neigh.level;
                    source->at(neigh.id)[i]=mysrc;
                    
                    myqueue.push(neigh);
                }//end of for
            }//end of while
        
    }//end of for
    
    
    //Sort the sources in decreasing order
/*#pragma omp parallel for schedule(dynamic) num_threads(p)
    for(int i=0;i<M->size();i++)
    {
        //sort in dec reasing order --highest first
        sort(&source->at(i), dec_first);
    }//end of for*/
    
    return;
}
/** End of Function **/

/*
void forkbased_numbering(A_Network *M, vector<bool> *is_source, vector<bool> *is_fork, vector< vector<int_int> > *source, vector<vector<int> > *fork,  vector<vector<int> > *branch,  vector<vector<int> > *level)
{
    
    //Initialize the vectors
    vector<int> dummyV;
    dummyV.clear();
    fork->resize(M->size(),dummyV);
    branch->resize(M->size(),dummyV);
    level->resize(M->size(),dummyV);
    
    vector<int_int> dummyVx;
    dummyVx.clear();
    source->resize(M->size(), dummyVx);
    
    #pragma omp parallel for schedule(dynamic) num_threads(p)
    for(int i=0;i<M->size();i++)
    {
        
     //do traversal if vertex is fork
        if(is_fork->at(i)==true || is_source->at(i)==true)
        {
            //Set value for i
            int_four node;
            node.id=i;
            node.fork=i;
            node.branch=i;
            node.level=0;
            
            //Add values to fork, branch and level
            fork->at(node.id).push_back(node.fork);
            branch->at(node.id).push_back(node.branch);
            level->at(node.id).push_back(node.level);
            
            //Add source
            int_int mysrc;
            mysrc.first=-1;
            mysrc.second=source->at(node.id).size();
            
            if(is_source->at(i)==true)
            { mysrc.first=i;}
            source->at(node.id).push_back(mysrc);
            
            queue<int_four> myqueue;
            myqueue.push(node);
          
            while(!myqueue.empty())
            {
                int_four this_node=myqueue.front();
                myqueue.pop();
               
                for(int j=0;j<M->at(this_node.id).ListW.size();j++)
                {
                    //Get values of neighbor
                    int_four neigh;
                    neigh.id=M->at(this_node.id).ListW[0].first;
                    neigh.fork=this_node.fork;
                    neigh.level=this_node.level+1;
                    if(neigh.level==1)
                    {neigh.branch=neigh.id;}
                    else
                    {neigh.branch=this_node.branch;}
                    
                    //Add values to fork, branch and level
                    fork->at(neigh.id).push_back(neigh.fork);
                    branch->at(neigh.id).push_back(neigh.branch);
                    level->at(neigh.id).push_back(neigh.level);
                    
                    //Add source
                    int_int mysrc;
                    mysrc.first=-1;
                    mysrc.second=source->at(neigh.id).size();
                    
                    if(is_source->at(i)==true)
                    { mysrc.first=i;}
                    source->at(neigh.id).push_back(mysrc);
                    
                    //Continue to traverse if negh not a fork
                    if(is_fork->at(neigh.id)==false)
                    {myqueue.push(neigh);} //no need for visited since it is DAG
            
                }//end of for
            }//end of while
        }//end of if
    }//end of for
    
    
    //Sort the sources in decreasing order
#pragma omp parallel for schedule(dynamic) num_threads(p)
    for(int i=0;i<M->size();i++)
    {
        //sort in dec reasing order --highest first
        sort(&source->at(i), dec_first);
    }//end of for

    return;
}
** End of Function **/
#endif /* insert_across_h */
