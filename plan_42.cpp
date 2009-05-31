
//
//  A very simple "toy OS-like" parser. 
//  
//  NOTE - as of now (29th May 2009) this parser requires 
//  Boost::Spirit version 2.1. At present you will need to 
//  get that version via svn. The following command will check out 
//  Boost-trunk - that will get you this version - 
//  
//  svn co http://svn.boost.org/svn/boost/trunk boost-trunk
//
//  Written by:  Andy Elvey
// 
//  Acknowledgements: I wanmt to acknowledge all of the developers who
//  have worked on Spirit, pretty much all of whom have helped me 
//  at one time or another on the Spirit mailing-list.     
//  
//  I also want to acknowledge an *excellent* resource for Plan 9 
//  fans - this site - 
//  http://plan9.escet.urjc.es/plan9.html 
//  
//  In the "Local Documents" section there, there is a PDF named 
//  9.intro.pdf.  It is the most outstanding in-depth intro to Plan 9 
//  that I have seen!  Essential reading for Plan 9 newcomers (and that 
//  includes me... :)  ) 
//  
//  This parser is released to the public domain.  
//  
//  This parser parses a few commands used in Plan 9. 
//  ( Yes, that's the real OS, not my toy one... :)  )  
//  However, the parser is so simple that I do not want to offend anyone 
//  associated with that OS, so I've called it Plan 42 instead (after the 
//  eventually-vaguely-Plan-9-like toy OS that I'm weorking on.  
//  
//  Only a few commands to start with. More will be added, so please 
//  be patient... :)  
//  
//  Anyway...  "share and enjoy......"  :)  
//    
//  "Plan 9" is a trademark of Bell Labs.  
//
//  #define BOOST_SPIRIT_DEBUG  ///$$$ DEFINE THIS WHEN DEBUGGING $$$


#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace boost::spirit;
using namespace boost::spirit::qi;
using namespace boost::spirit::ascii;
using namespace boost::spirit::qi::labels;


///////////////////////////////////////////////////////////////////////////////
//  Our grammar
///////////////////////////////////////////////////////////////////////////////

//  NOTE - I have included the semicolon as part of the grammar. 
//  The semicolon is the command prompt in Plan 9. It is also used to 
//  separate multiple commands.  


template <typename Iterator>
struct plan_42_grammar : grammar<Iterator, space_type> 
{
    plan_42_grammar() : plan_42_grammar::base_type(expression)
	
    {
		
    expression 					
	%=  prompt >> +( commands ) ;  
	
	prompt 
	%= char_(";") ; 
	
	commands 
	%= date_cmd 
	|  ls_cmd ;  
	
/*	
	|  lc_cmd 
	|  touch_cmd 
	|  cp_cmd 
	|  rm_cmd 
	|  mv_cmd 
	|  cd_cmd 
	|  pwd_cmd 
	|  path_name ; 

*/  
	
		
// 	The "date" command. 
    date_cmd 
	%=  lit("date") ;  	
		
//  The "ls" command and its options 
//  In Spirit 2, the "-" operator is used for optional tokens.  
    ls_cmd 
	%=  lit("ls") >> !(ls_options) >> path_name; 
	
	ls_options 
	%=  lit("-s") ; 
	
/*	
	 	
//  The "lc" command.  
    lc_cmd 
	%=  lit("lc") ;  		
	
//  The "touch" command. 
    touch_cmd 	
	%=  lit("touch") ; 
	
//  The "cp" command. 
    cp_cmd 
	%=  lit("cp") >> source >> target; 
		
//  The "rm" command. 
    rm_cmd 
	%=  lit("rm") >> target; 	 		
	
//  The "mv" command 
    mv_cmd 
	%=  lit("mv") >> source >> target; 
	
//  The "cd" command 
    cd_cmd 
	%=  lit("cd") >> -(path_name);  
	
//  The "pwd" command. 
    pwd_cmd 
	%=  lit("pwd");  			
	
*/  	
	
//  Path name.  I've just hardcoded a few paths here 
//  for simplicity.  
    path_name
	%=  (lit("/usr") | lit("/usr/zaphod") ); 
	
/*	
	 	
//  Source and target (for commands that use them, like mv). 				
	source 
	%=  identifier ; 
	
	target 
	%=  identifier ; 

//  Identifier 	                                           	
    identifier %= lexeme[ ( alpha >> *(alnum | '_' | '.' ) ) ] ;    
	
	* 
	* 
	*/
	
			   		                                                                                                                                                                                                                                                                             
   }

      rule<Iterator, space_type> expression, 
	     prompt, commands, date_cmd, ls_cmd, 
		 ls_options, path_name; 
	/*	 
		 lc_cmd, touch_cmd, cp_cmd, rm_cmd, 
		 mv_cmd, cd_cmd, pwd_cmd, path_name, 
		 source, target, identifier;    */  
		 		                  
};



int main()  
{
    std::cout << "/////////////////////////////////////////////////////////\n\n";
    std::cout << "\t\t A toy OS-like parser...\n\n";
    std::cout << "/////////////////////////////////////////////////////////\n\n";
    std::cout << "Type a SEMICOLON then a Plan 9 command  \n" ; 
    std::cout << " (e.g. ; cp foo bar,  ; cd /usr   \n" ;
    std::cout << " Type [q or Q] to quit\n\n" ;

    typedef std::string::const_iterator iterator_type;
    typedef plan_42_grammar<iterator_type> plan_42_grammar;

    plan_42_grammar mygrammar; 

    std::string str;

    while (std::getline(std::cin, str))
    {
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;

        std::string::const_iterator iter = str.begin();
        std::string::const_iterator end = str.end();
        bool r = phrase_parse(iter, end, mygrammar, space);

        if (r && iter == end)
        {
            std::cout << "-------------------------\n";
            std::cout << "Parsing succeeded\n";
            std::cout << "-------------------------\n";
        }
        else
        {
            std::string rest(iter, end);
            std::cout << "-------------------------\n";
            std::cout << "Parsing failed\n";
            std::cout << "stopped at: \": " << rest << "\"\n";
            std::cout << "-------------------------\n";
        }
    }

    std::cout << "Bye... :-) \n\n";
    return 0;
}


