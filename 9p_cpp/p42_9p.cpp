

//  p42_9p.cpp 
//  A port of Tom Newsham's excellent Python 9p 
//  implementation to C++ 

//  Acknowledgements - This code would not have been 
//  possible without Tom Newsham's implementation being 
//  available.  Many thanks, Tom!  

//  This code is released to the public domain. 
//  "Share and enjoy......"   ;)  


#include <vector> 
#include <string> 
#include <iostream> 

using namespace std ; 

//  Main 
int main() 
{ 
//  Define the command enum.   
//  First, we create the numeric array for the codes (this is the 
//  cmdName array in Tom's code).  
vector <int> cmdName;  

for (int i=0; i<28; i++) 
{     
    cmdName[i] = i + 100; 
    cout << cmdName[i] << "\n" ; 
} 

return 0 ; 

}   











    






