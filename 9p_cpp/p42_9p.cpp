

//  p42_9p.cpp 
//  A port of Tom Newsham's excellent Python 9p 
//  implementation to C++ 

//  Acknowledgements - This code would not have been 
//  possible without Tom Newsham's implementation being 
//  available.  Many thanks, Tom!  

//  This code is released to the public domain. 
//  "Share and enjoy......"   ;)  

//  N!TE! - This code uses some C++0x features, so you will need 
//  to run g++ with the "-std=c++0x" compile option.    


#include <vector> 
#include <map> 
#include <string> 
#include <iostream> 

using namespace std ; 

//  Main 
int main() 
{ 
    
//  Define two maps. One maps from the code to the text command, and 
//  the other does the reverse.  
typedef map <int, string>  num_to_name;  
typedef map <string, int>  name_to_num; 
        
//  Define the numbers and commands.   
//  First, we create the numeric vector for the codes (this is the 
//  cmdName array in Tom's code). We also create a string vector 
//  for the commands. 
 
vector <int> nums;  

// The vector for the commands  
vector <string> cmds { "version", "auth" , "attach", "error",  
    "flush", "walk", "open", "create", "read", "write", "clunk",  
    "remove", "stat", "wstat" };  
    
//  A vector to hold all of the "T" and "R" commands     
vector <string> trcmds ;          
                   
for (int i=0; i<14; i++) 
{  
   trcmds.push_back("T" + cmds[i] ) ; 
   trcmds.push_back("R" + cmds[i] ) ;   
   cout << trcmds[i] << "\n" ;           
}                
          
          
for (int j=0; j<28; j++) 
{     
   nums.push_back(j+100);     
   cout << nums[j] << "\n" ;      
} 


return 0 ; 

}   











    






