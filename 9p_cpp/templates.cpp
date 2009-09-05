

//  templates.cpp  
//  Becoming more familiar with the STL.  


#include <vector> 
#include <map> 
#include <string> 
#include <iostream> 
#include <valarray> 
#include <cmath> 

using namespace std ; 


//  A template for a list of functions and args 
template <typename... funcs> class myfuncs ; 
template <typename... args> class myargs ; 
 
 
void andysfunction(string *args) 
{  
    cout << *args << "\n" ;     
}  
     
 
void andy_func(int val) 
{ 
    cout << val << "\n" ;       
}      
 
 
string printstr(string a, string b) 
{
  
   cout << a << " " << b << "\n" ;
   return "foo" ;    
    
}       
 

string concat(string a, string b) 
{
  
   return a + b ; 
    
}       
 
 
 
 
 
//  Create an instance of the myfuncs class. 


//  std::vector<(andysfuncs *)> functions ; 

//  class myclass<float sqrt(), float square(), std::vector<float> >; 



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

//  Call andy_func 
andy_func(235); 

//  Call the printstr function 
printstr("foo", "bar"); 

//  Call the concat function 
concat("foo", "bar"); 


return 0 ; 

}   


