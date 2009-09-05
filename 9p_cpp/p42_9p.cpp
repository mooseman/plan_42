

//  p42_9p.cpp 
//  A port of Tom Newsham's excellent Python 9p 
//  implementation to C++ 

//  Acknowledgements - This code would not have been 
//  possible without Tom Newsham's implementation being 
//  available.  Many thanks, Tom!  

//  This code is released to the public domain. 
//  "Share and enjoy......"   ;)  

//  NOTE! - This code uses some C++0x features, so you will need 
//  to run g++ with the "-std=c++0x" compile option.    


#include <vector> 
#include <map> 
#include <string> 
#include <iostream> 
#include <iomanip> 
#include <valarray> 
#include <cmath> 

using namespace std ; 


//  A function to pad strings 
std::string pad(std::string str, int l, char padch='\0') 
{
   int i;  
   for (i=0; i<=l; i++) 
      str += padch * (l - str.length() ) ; 
   return str.substr(0, l); 
} 


//  This Python function prints a space-separated hex version 
//  of a string. 
//  def XXXdump(buf) :
//	   print " ".join(["%02x" % ord(ch) for ch in buf]) 

//  Note - look at using a vector here as the argument, instead of 
//  a string.  Alternatively, the string can be passed as the argument
//  and then put in the vector. Need to apply the "ord" function to 
//  all elements in the vector. Then we convert them to their hex value.
void hexdump(std::string buf) 
{ 
   vector<int> v;  
   
   for (int i=0; i<v.size(); i++)  
//    std::cout << std::setfill(" ") << setbase(16) << *my_iter << endl ;  
      std::cout << v[i] << endl ;  
    
}     




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

//  Define some constants 
string version = "9P2000" ; 

int notag = 0xffff ; 
int nofid = 0xffffffff; 
int DIR = 020000000000 ;
int QDIR = 0x80 ; 
int OREAD = 0; 
int OWRITE = 1; 
int ORDWR = 2; 
int OEXEC = 3; 
int OTRUNC = 0x10; 
int ORCLOSE = 0x40; 
int PORT = 564; 

//  This is just here to stop warnings about 
//  unused variables. 
cout << notag << nofid << DIR << QDIR 
  << OREAD << OWRITE << ORDWR << OEXEC 
  << OTRUNC << ORCLOSE << PORT << "\n" ; 


return 0 ; 

}   


