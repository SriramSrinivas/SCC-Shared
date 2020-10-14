//
//  Tarjan.hpp
//  
//
//  Created by Sanjukta Bhowmick on 4/4/19.
//Modified from https://www.geeksforgeeks.org/tarjan-algorithm-find-strongly-connected-components/
//

#ifndef Tarjan_h
#define Tarjan_h


void tarjan(int node, A_Network *X, vector<int_int> *SCC_ID, vector<int> *newSCC_ID, vector<int> *disc, vector<int> *low, vector<bool> *stackmember, stack<int>* st, int *iter )
{
 int sd=0;
#pragma omp atomic write
  *iter=*iter+1;	
 disc->at(node)=*iter;

    low->at(node)=*iter+1;
    st->push(node);
    stackmember->at(node)=true;
    
    //Get neighbors of node
    for(int i=0;i<X->at(node).ListW.size();i++)
    {
        int neigh=X->at(node).ListW[i].first;
        
        //ignore if SCC is different
        if(SCC_ID->at(node).first!=SCC_ID->at(neigh).first){continue;}
        
        
        if(disc->at(neigh)==-1)
        {
            tarjan(neigh,X,SCC_ID,newSCC_ID, disc,low, stackmember,st,iter);
            if( low->at(node) > low->at(neigh))
            {low->at(node) = low->at(neigh);}
        }
        else if (stackmember->at(neigh)==true)
        {
            if( low->at(node) > disc->at(neigh))
            {low->at(node) = disc->at(neigh);}
        }
    } //end of for
    
    //Popping th stack
    int w=0;
    vector<int> mySCC;
   
    if(low->at(node)==disc->at(node))
    {
         mySCC.clear();
        while(st->top()!=node)
        {
            w=st->top();
            stackmember->at(w)=false;
            mySCC.push_back(w);
            st->pop();
        }//end of while
        
        w=st->top();
        stackmember->at(w)=false;
        mySCC.push_back(w);
        st->pop();
        
        //Get new ID
        int id=*min_element(mySCC.begin(),mySCC.end());
        for(int i=0; i<mySCC.size();i++)
        {newSCC_ID->at(mySCC[i])=id;}

    }//end of if
    
}

/*** End of Function **/

void multiple_tarjan(A_Network *X, vector<int_int> *SCC_ID, vector<int> *SCCx, vector<int> *delete_nodes,int p )
{
    vector<int> newSCC_ID; //stores the SCC ids of the vertices
    newSCC_ID.resize(X->size(),-1);
    
    //Variables for Tarjan's algorithm
    vector<int> disc;
    disc.resize(X->size(),-1);
    
    vector<int> low;
    low.resize(X->size(), -1);
    
    vector<bool> stackmember;
    stackmember.resize(X->size(),false);
    
    #pragma omp parallel for num_threads(p) schedule(dynamic)
    for(int k=0;k<delete_nodes->size();k++)
    {
        stack<int> mystack;
         
        int i=delete_nodes->at(k);
        int iter=0;
         if(disc[i]==-1)
         {tarjan(i,X,SCC_ID,&newSCC_ID, &disc,&low, &stackmember, &mystack, &iter);}
    }
    
    disc.clear();
    low.clear();
    stackmember.clear();
    
//Add to map therefore sequential
    for(int k=0;k<SCC_ID->size();k++)
    {
        if(newSCC_ID[k]!=-1)
        {
             SCC_ID->at(k).first=newSCC_ID[k];
             SCCx->at(k)=newSCC_ID[k];
        }//end of if
    }//end of for
    
}
/*** End of Function **/

#endif /* Tarjan_h */
