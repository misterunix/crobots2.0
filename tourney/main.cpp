using namespace std;

//c++
#include<iostream>
#include<fstream>
#include<map>
#include<string>
#include<stdexcept>
#include<vector>
#include<sstream>

//c
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

//mysql
#include<mysql/mysql.h>


map<string,string> options;

// Keep a connection to mysql open
MYSQL *mysqlCon;

vector<string> &split(const string &s, char delim, vector<string> &elems) 
{
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) 
	{
        elems.push_back(item);
    }
    return elems;
}


vector<string> split(const string &s, char delim) 
{
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

// vector<string> x = split("one:two::three", ':');

void MysqlErrorReport( void )
{
	cerr << "Error: " << mysql_error(mysqlCon) << endl;
	exit(1);
}

  



void SaveBotToTMP( int botID )
{
	MYSQL_RES *result;
	MYSQL_ROW row;
	char sql[256];      // Dont like this! Want to use string
	int numFields;
	string r;
	
	sprintf(sql,"SELECT code FROM bots WHERE id='%i'",botID);
        if(mysql_query(mysqlCon,sql))
        {
                MysqlErrorReport();
        }

        result = mysql_store_result(mysqlCon);
        if( result == NULL )
        {
                MysqlErrorReport();
        }

        // Will this cause a memory leak?
        // Not calling free if there is no fields
        numFields = mysql_num_fields(result);
        if( numFields != 1 )
        {
                cerr << "Something wrong in SaveBotToTMP. numFields\n";
                cerr << "sql: '" << sql << "'" << endl;
                exit(1);
        }
        row = mysql_fetch_row(result);
        r = row[0];
        mysql_free_result(result);

	char filename[1024];
	sprintf(filename,"%s/%i",options["tmpfolder"].c_str(),botID);
        ofstream outfile;
	outfile.open(filename);
	outfile << r.c_str() << endl;
	outfile.close();
}

int RunMatch( int count, int *bots, int numbots )
{
  int i,j;
  // save bots to tmp
  for(i=0;i<numbots;i++)
  {
    SaveBotToTMP(bots[i]);
  }
  char temp[256];
  char results[1024];
  string cmd;
  FILE *pp;
  cmd.append( options["crobots"] );
  sprintf(temp," -m%i",count);
  cmd.append( temp );
  for(i=0;i<numbots;i++)
  {
    sprintf(temp," %s/%i",options["tmpfolder"].c_str(),bots[i]);
    cmd.append( temp );
  }

  cout << "cmd : '" << cmd.c_str() << "'\n" << flush;

  pp = popen(cmd.c_str(),"r");
  if( pp == NULL ) 
  {
    perror( "popen: ");
    exit(1);
  }

  memset(results,0,1024);
  fread(results,1,255,pp);
  j = strlen(results);
  for(i=0;i<j;i++) if( (results[i] == 13) || (results[i] == 10) ) results[i] = 0;
  cout << "Results: '" << results << "'" << endl << flush;
  pclose(pp);
  return(1);
}            

string GetUserName( int x )
{
	MYSQL_RES *result;
	MYSQL_ROW row;
    char sql[256];	// Dont like this! Want to use string
	int numFields;
	string r;
	
	sprintf(sql,"SELECT realname FROM users WHERE id='%i'",x);
	if(mysql_query(mysqlCon,sql))
	{
		MysqlErrorReport();
	}

	result = mysql_store_result(mysqlCon);
	if( result == NULL )
	{
		MysqlErrorReport();
	}
	
	// Will this cause a memory leak?
	// Not calling free if there is no fields
	numFields = mysql_num_fields(result);
	if( numFields != 1 )
	{
		cerr << "Something wrong in GetUserName numFields.\n";
		cerr << "sql: '" << sql << "'" << endl;
		exit(1);
	}
	row = mysql_fetch_row(result);
	r = row[0];
	mysql_free_result(result);
	return( r );
}
	
void PrintBotName( void )
{
	string sql;
	MYSQL_RES *result;
	MYSQL_ROW row;
	string temp;
	
	int numFields;

	sql = "SELECT owner,name FROM bots";
	if(mysql_query(mysqlCon,sql.c_str()))
	{
		MysqlErrorReport();
	}

	result = mysql_store_result(mysqlCon);
	if( result == NULL )
	{
		MysqlErrorReport();
	}
	
	// Will this cause a memory leak?
	// Not calling free if there is no fields
	numFields = mysql_num_fields(result);
	if( numFields == 0 )
	{
		cout << "No Bots to list.\n";
		return;
	}
	
	while ((row = mysql_fetch_row(result))) 
	{ 
		temp = GetUserName(atoi(row[0]));
		cout << temp.c_str() << ": " << row[1] << endl;
	}
	mysql_free_result(result);
}



void parseINI( ifstream & cfgfile )
{
	string id,eq,val;
	while(cfgfile >> id >> eq >> val)
	{
		if(id[0] == '#') continue;
		if(eq[0] != '=' ) runtime_error("Parse error");
		options[id] = val;
	}
}

void printOptions( void )
{
  cout << "-h -help --help" << endl;
  cout << "-u username" << endl;
  cout << "-p password" << endl;
  cout << "-b botname" << endl;
  cout << "-f filename" << endl;
  cout << "-a (Add bot to user db.)" << endl;
  cout << "-t hill (Add bot to hill.)" << endl;
  cout << endl;
  exit(0);
}

int main( int argc, char **argv )
{

	ifstream infile;

	infile.open("tourney.ini");
	parseINI( infile );
	infile.close();

	int i;
	vector <string> cla;
	for(i=1;i<argc;i++)
	{
		string arg;
		arg = argv[i];
		if( (arg=="-h") || (arg=="-help") || (arg=="--help") )
		{
			printOptions();
		}
		if( arg=="-u" )
		{
			i++;
			cout << "username: " << argv[i] << endl;
		}
		if( arg=="-p" )
		{
			i++;
			cout << "password: "<< argv[i] << endl;
		}
		if( arg=="-b" )
		{
			i++;
			cout << "botname: " << argv[i] << endl;
		}
		if( arg=="-t" )
		{
			i++;
			cout << "hill: " << argv[i] << endl;
		}
		if( arg=="-f" )
		{
			i++;
			cout << "filename: " << argv[i] << endl;
		}
		if( arg=="-a" )
		{
			cout << "Add bot to user." << endl;
		}
		if( arg=="-d" )
		{
			cout << "Remove bot from user." << endl;
		}
	}


	for (map<string,string>::iterator it=options.begin(); it!=options.end(); ++it)
	cout << it->first << " => " << it->second << '\n';

	cout << "mysql client version: " << mysql_get_client_info() << endl << endl;

	mysqlCon = mysql_init(NULL);
	if( mysqlCon == NULL )
	{
		cerr << "Error: " << mysql_error(mysqlCon) << endl;
		exit(1);	
	}
	
	if(mysql_real_connect(mysqlCon, "localhost", options["dbuser"].c_str(), options["dbpass"].c_str(),options["dbdb"].c_str(), 0, NULL, 0) == NULL) 
	{
		cerr << "Error: " << mysql_error(mysqlCon) << endl;
		mysql_close(mysqlCon);
		exit(1);
	}  
	PrintBotName();
	cout << endl;

	int b[2];
        b[0] = 1;
        b[1] = 2;
	RunMatch(101,b,2);
        cout << "End of program\n" << flush;
}

