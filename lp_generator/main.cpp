#include <iostream>
#include <string>
#include "Parser.h"

using namespace std;


float budget = 0.;
string inputXML = "";
string outputLPDir = "../example_output/";
string appName = "";

int main(int argc, const char **argv){

    if (argc >= 7) {
        for (int i = 1; i < argc; i++) {
            if (!strcmp(argv[i], "--xml"))
                inputXML = argv[++i];
            if (!strcmp(argv[i], "--outdir")){
                outputLPDir = argv[++i];
            }
            if (!strcmp(argv[i], "--budget"))
                budget = stof(argv[++i]);
            if (!strcmp(argv[i], "--app"))
                appName = argv[++i];
        }
    } else{
        cout << "argument missing: needs at least --app <application_name> --xml <xml_file_path> --budget <budget>";
        exit(1);
    }

    Parser* parser = new Parser(appName);
    parser->genKDGwithXML(inputXML);
    parser->setBudget(budget);
    parser->writeLp(outputLPDir);

}
