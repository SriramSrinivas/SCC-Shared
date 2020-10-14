#ifndef READIN_DATA_HPP
#define READIN_DATA_HPP

using namespace std;




//Read in the SCC Ids
void readin_SCC(vector<int_int> *SCC_ID, vector<int> *SCCx, char *file)
{
    //File reading parameters
    FILE *graph_file;
    char line[128];
    
    graph_file=fopen(file, "r");
    int l=0; //used a counter to detect first line whih is not read
    int ID;
    std::pair<std::map<int,int>::iterator,bool> retE;
    std::map<int,int>::iterator itE;
    
    while(fgets(line,128,graph_file) != NULL)
    {
        //Read line
        sscanf(line,"%d ",&ID);
        
        SCC_ID->at(l).first=ID;
        SCC_ID->at(l).second=l;
        SCCx->at(l)=ID;
        
        
        
        //Add to map
       /* itE=IDmap->find(ID);
        if(itE==IDmap->end()) //id already exists find location
        { retE=IDmap->insert(std::pair<int,int>(ID,IDmap->size()));}*/

        l++;
    }//end of while
    
    
    
return;
}
    


//Information about set of changed edges
void readin_changes(vector<int_int>* inserts, vector<int_int>* deletes, char *myfile)
{
    //File reading parameters
    FILE *graph_file;
    char line[128];
    int type;
    int_int ID_edge;
    
    graph_file=fopen(myfile, "r");
    while(fgets(line,128,graph_file) != NULL)
    {
        sscanf(line,"%d  %d %d",&ID_edge.first, &ID_edge.second, &type);
        
        if(type==0) //Delete from X
        { deletes->push_back(ID_edge);}
        
        if(type==1) //Insert to X
        { inserts->push_back(ID_edge);}
        
    }//end of while
    
    fclose(graph_file);
}
/*** End of Function ***/


#endif
