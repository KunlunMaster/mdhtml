#include <iostream>
#include <regex>
#include <fstream>
#include <cstdio>
#include "Block.h"
#include "config.h"

#include "LineParser.h"
#include "Blocker.h"

using namespace std;
using namespace mdht;

int main(int argc, char*argv[]) {
    string fname;
	if(argc == 1){
    cout<<"pls input md file name:";
    cin>>fname;
	}
	else 
		fname = argv[1];
	if(fname.size() == 1 && stoi(fname) < 10)
		fname="md/test" + fname + ".md";
    MarkdownParser mdp;
    if(!mdp.parse(fname))
        return -1;
    if(!mdp.output_html(fname+".html"))
        return -1;
    cout<<"result output:"<<fname<<".html\n"
			<<"do you wanna open it?(y/n)";
	char o = 'n';;
	cin >>o;
	if(o == 'y' || o == 'Y'){
		string cmd = "open " + fname + ".html";
		system(cmd.c_str());
		return 0;
	}
	return 0;
}
