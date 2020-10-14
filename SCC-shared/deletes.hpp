//
//  delete_within.hpp
//  
//
//  Created by Bhowmick, Sanjukta on 8/18/20.
//

#ifndef deletes_hpp
#define deletes_hpp

//Check if there is a path from n1 to n2
bool get_path(A_Network *X, vector<int_int> *SCC_ID, int n1, int n2)
{
    queue<int> myQ;
    vector<bool> visited;
    visited.resize(X->size(), false);
    
    myQ.push(n1);
    visited[n1]=true;
    bool found=false;

    while(!myQ.empty())
    {
        
        int node=myQ.front();
        myQ.pop();
        
        for(int i=0;i<X->at(node).ListW.size();i++)
        {
            //Ignore if edge is deleted
            if(X->at(node).ListW[i].second==-1){continue;}
            
            
              int neigh=X->at(node).ListW[i].first;
            //Ignore if different SCC
            if(SCC_ID->at(node).first!=SCC_ID->at(neigh).first){continue;}
            
            //If neighbir found then done
            if(neigh==n2){found=true; break;}
            
            //Ignore if already visited
            if(visited[neigh]){continue;}
            
            myQ.push(neigh);
        }
        
    }
    
    visited.clear();
    return found;
}
/*** End of Function ***/

 void find_valid_deletes(A_Network *X, vector<int_int> *SCC_ID, vector<int_int> *deletes, vector<int> *delete_nodes, int p)
{
    vector<bool> valid_deletes;
    valid_deletes.resize(deletes->size(),true);
    
#pragma omp parallel for num_threads(p) schedule(static)
    for(int i=0;i<deletes->size();i++)
    {
        //mark edges as -1
        int n1=deletes->at(i).first;
        int n2=deletes->at(i).second;
        
        //First delete from network
        for(int j=0;j<X->at(n1).ListW.size();j++)
        {
            int n=X->at(n1).ListW[j].first;
            if(n==n2)
            {X->at(n1).ListW[j].second=-1;}
        }//end of for
        
        
        //Check if delete affects the SCC
        //If the end points have different SCC then delete does not matter
        if(SCC_ID->at(n1).first!= SCC_ID->at(n2).first)
        {valid_deletes[i]=false;}
    
    }//end of for

    //Find whether  deletes are valid based on paths
    //Do in two stages to ensure all -1 are marked.
#pragma omp parallel for num_threads(p) schedule(dynamic)
    for(int i=0;i<deletes->size();i++)
    {
        //Only for valid deletes
        if(valid_deletes[i]==false) {continue;}
        
        int n1=deletes->at(i).first;
        int n2=deletes->at(i).second;
    
        //If there exists an alternate path from n1 to n2, then this delete does not affect SCC
       bool found=get_path(X,SCC_ID,n1,n2);
        //If path found then delete is not valid
        valid_deletes[i]=(!found);
     }//end of for
  
    //Sequential
    vector<int> dnodes;
    dnodes.clear();
    for(int i=0;i<valid_deletes.size();i++)
    {
        if(valid_deletes[i])
        {dnodes.push_back(deletes->at(i).first);
         dnodes.push_back(deletes->at(i).second);
        }
    }//end of if
    
    sort(&dnodes);
    *delete_nodes=unique(dnodes);
    
    dnodes.clear();
    valid_deletes.clear();
    
}
/*** End of Function ***/

#endif /* delete_within_h */

