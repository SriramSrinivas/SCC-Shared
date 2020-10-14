


#ifndef insert_hpp
#define insert_hpp




void find_valid_inserts(A_Network *M, vector<int> *SCCx, vector<int_int> *inserts, vector<int> *valid_inserts,vector< vector<int> > *source, vector<vector<int> > *fork,  vector<vector<int> > *branch,  vector<vector<int> > *level,int p)
{
#pragma omp parallel for schedule(dynamic) num_threads(p)
    for(int i=0;i<inserts->size();i++)
    {
        
        int src=inserts->at(i).first;
        int dest=inserts->at(i).second;
        
        //Find the SCC_IDs for the meta graph
        int src_id=SCCx->at(src);
        int dest_id=SCCx->at(dest);
        
       // printf("%d %d::: %d %d \n",src, dest, src_id, dest_id);
       
        //If they are in the same SCC then continue
        if(src_id==dest_id)
        {continue;}
        
        //Otherwise
        //compare to see if they have common source nodes (called root).
        //since source nodes are ordered as per their id, we just have to compre the source vectors
        //even if multiple common sources are possible it is sufficient to break at the first one
        bool found_root=false;
        int index=0;
        for(int j=0;j<source->at(src_id).size();j++)
        {
            if(source->at(src_id)[j]==source->at(dest_id)[j])
            {
                if(source->at(src_id)[j]!=-1)
                {found_root=true;
                    index=j;
                    break;
                }
            }
        }//end of for
    
        //If common root not found then continue;
         if(found_root==false){continue;}
        
        //Otherwise
        //Compare their levels at the common source
        //if level of src is less than level of destination then it will not be a back edge that completes the SCC
        if(level->at(src_id)[index]<level->at(dest_id)[index]){continue;}
        
        //Otherwise
        //Check that they have common fork and common branch
        int src_frk=fork->at(src_id)[index];
        int next_src_frk=-1;
       int dest_frk=fork->at(dest_id)[index];
        
        int src_branch=branch->at(src_id)[index];
        int next_src_branch=-1;
         int dest_branch=branch->at(dest_id)[index];
        
        while(src_frk!=dest_frk) //has to be equal at some point since earliest fork is source (root)
          {
              next_src_frk=fork->at(src_frk)[index];
              next_src_branch=branch->at(src_frk)[index];
              src_frk=next_src_frk;
              src_branch=next_src_branch;
          }//end of while
        
        
       //If dest_id not a root node then check if branches match
        //If the branch is different then continue
        if(dest_frk!=dest_branch)
        { if(src_branch!=dest_branch) {continue;}}
        
        //Otherwise--it is a valid insert
        valid_inserts->at(i)=index;
        
        
    }//end of for
    
    return;
}
/*** End of Function ***/

void connect_SCCs(A_Network *M, vector<int> *SCCx,vector<int> *newSCC, vector<int_int> *inserts, vector<int> *valid_inserts, vector< vector<int> > *fork, vector< vector<int> >*branch,int p)
{
  
#pragma omp parallel for schedule(dynamic) num_threads(p)
    for(int i=0;i<inserts->size();i++)
    {
        //Continue if inserts are not valid
        if(valid_inserts->at(i)==-1){continue;}
        
        //get index of source
        int index=valid_inserts->at(i);
        
        int src=inserts->at(i).first;
        int dest=inserts->at(i).second;
        
        //Find the SCC_IDs for the meta graph
        int src_id=SCCx->at(src);
        int dest_id=SCCx->at(dest);
        
       // printf("%d  %d \n",src_id, dest_id);
        
        //List all the branches until common frk reached
        vector<int> listfrks;
        
        int src_frk=fork->at(src_id)[index];
        int dest_frk=fork->at(dest_id)[index];
        int src_branch=branch->at(src_id)[index];
        int dest_branch=branch->at(dest_id)[index];
        int next_src_frk=-1;
        int next_src_branch=-1;
        
        {listfrks.push_back(src_branch);}
        
        while(src_frk!=dest_frk) //has to be equal at some point since earliest fork is source (root)
        {
            next_src_frk=fork->at(src_frk)[index];
            next_src_branch=branch->at(src_frk)[index];
            src_frk=next_src_frk;
            src_branch=next_src_branch;
            listfrks.push_back(src_branch);
        }//end of while
       // listfrks.push_back(dest_id);
        //print_vector(listfrks);

        int b; //from where to read listfrks
        if((src_frk==dest_id) &&(dest_id!=dest_branch)) //if destination is a fork, but not the root include final branch
        { b=listfrks.size()-1;}
        else //dont include it
         { b=listfrks.size()-2;}
        
        
        //traverse move to the correct branch
        vector<int> visited_nodes;
        visited_nodes.clear();
        int cur_node=dest_id;
        int next_node;
        visited_nodes.push_back(dest_id);
       
        
        while(cur_node!=src_id)
        {
            //If single path continue;
            while(M->at(cur_node).ListW.size()==1)
            {
                next_node=M->at(cur_node).ListW[0].first;
                visited_nodes.push_back(next_node);
                cur_node=next_node;
            }
            
            //Now arrive at fork
            for(int j=0;j<M->at(cur_node).ListW.size();j++)
            {
                if(M->at(cur_node).ListW[j].first==listfrks[b])
                {
                    next_node=M->at(cur_node).ListW[j].first;
                    visited_nodes.push_back(next_node);
                    cur_node=next_node;
                    b=b-1;
                }
            }
        }//end of while
        
       // print_vector(visited_nodes);
        
        //get minimum node id from visited
        int min_id=*min_element(visited_nodes.begin(), visited_nodes.end());
        
        int myvisit=1;
        for(int k=0;k<visited_nodes.size();k++)
        {
            if(newSCC->at(visited_nodes[k])>min_id)
            {
               #pragma omp atomic write
                newSCC->at(visited_nodes[k])=min_id;
                myvisit++;
                           // change=true;
            }
        }//end of for
        
        //If all the nodes updated, then this path no longer needed
        if(myvisit==visited_nodes.size())
          {valid_inserts->at(i)=-1;}
       // printf("%d ...==\n",myvisit);
    
    }//end of for
                            
    
    
    //Update the SCC to the lowest value iteratively
    bool change=true;
    while(change)
    {
    change=false;
        
#pragma omp parallel for schedule(dynamic) num_threads(p)
        for(int i=0;i<newSCC->size();i++)
        {
            int id=newSCC->at(i);
            
            if(newSCC->at(id)<id)
            {newSCC->at(i)=newSCC->at(id);
                change=true;}
            
        }//end of for
        
        
    }//end of while
    
    return;
}
/*** End of Function ***/

#endif /* insert_across_h */
