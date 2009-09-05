

//  templates2.cpp  
//  Becoming more familiar with the STL.  


#include <vector> 
#include <map> 
#include <string> 
#include <iostream> 
#include <valarray> 
#include <cmath> 

using namespace std ; 

//  The following code is from here - 
//  http://www.daniweb.com/forums/thread139414.html 

class Movie
{   
    //  Create a type for our class. This type is a vector 
    //  (defined below).   
    typedef void (Movie::*MovieFunc)();
    
public:
    Movie()
    {
        //  The functions to push onto the vector 
        f.push_back(&Movie::heHe);
        f.push_back(&Movie::haHa);
    }
    
    //  A method for the class. This takes an int (the index of 
    //  the vector) and executes the function in that index.  
    void show(int part) { (this->*f[part])(); }
    
private:
    //  The functions used are defined here 
    void heHe() { cout << "He he" << endl; }
    void haHa() { cout << "Ha ha" << endl; }
    
    //  Create the MovieFunc vector 
    vector<MovieFunc> f;
};


/*
void Serial()
{
    Movie m;
    m.show(0);
    m.show(1);
}
*/


int main()  
{ 

//  Create a movie instance and call the methods on it. 
  Movie a;
  a.show(0);
  a.show(1);


return 0; 
} 






