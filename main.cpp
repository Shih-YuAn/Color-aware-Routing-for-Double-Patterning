// **************************************************************************
//  File       [main.cpp]
//  Author     [Jheng-Ying Yu]
//  Synopsis   [The main program of 2018 Spring Algorithm PA3]
//  Modify     [2018/04/08 Jheng-Ying Yu]
// **************************************************************************

#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
//#include "../lib/tm_usage.h"
#include "Routing.h"
#include <map>
using namespace std;

void help_message() {
    cout << "usage: Routing <input_file_pin> <input_file_block> <input_file_net> <output_file>" << endl;
}
/*for alpha test
void help_message() {
    cout << "usage: Routing <input_file_pin> <input_file_net> <input_file_block>  <output_file>" << endl;
}
*/

int main(int argc, char* argv[])
{
    if(argc != 5) {
       help_message();
       return 0;
    }
//    CommonNs::TmUsage tmusg;
//    CommonNs::TmStat stat;

    //////////// read the input file /////////////

	double num;
	vector<double> position(3);
	int vias=0;
	vector<float> mapping;
	map<vector<float>,int> viasmap;

    fstream fin;
	fin.open(argv[1]);
	vector< vector<double> > pins;

	pins.push_back(position);
	double minx,maxx,miny,maxy;
	int check2=0,check3=0;
	while(fin >> num){
		fin >> num;
		position[2] = num;
		fin >> num;
		position[0] = num;
		if (position[0]<minx || check2==0){
			minx=position[0];
			if (check2==0){
				maxx=position[0];
			}
			check2=1;
		}
	   	else if (position[0]>maxx){
			maxx=position[0];
		}
		fin >> num;
		position[1] = num;
		if (position[1]<miny || check3==0){
			miny=position[1];
			if (check3==0){
				maxy=position[1];
			}
			check3=1;
		}
		else if (position[1]>maxy){
			maxy=position[1];
		}
		mapping.push_back(position[0]);
		mapping.push_back(position[1]);
		viasmap[mapping]=position[2];
		mapping.clear();
		pins.push_back(position);
		//[x,y,layer]
	}
	fin.close();

	position.resize(4);

	fin.open(argv[2]);//cad argv[3]
	vector< vector<double> > blockages;
	while(fin >> num){
		position[0] = num;
		fin >> num;
		position[1] = num;
		fin >> num;
		position[2] = num;
		fin >> num;
		position[3] = num;
		blockages.push_back(position);
	}
	fin.close();

	fin.open(argv[3]);//cad argv[2]
	vector< vector<int> > nets;
	vector<string> isCritical;

	string a,b;
	int number,k=0;
	string str1("Y");
    string str2("N");
	vector<int> pinsToConnect;
	int numberOfCriticalNets = 0;

	isCritical.push_back("0");
    nets.push_back(pinsToConnect);
    while(getline(fin,a,'\n')){
        stringstream iss(a);
        while(getline(iss,b,' ')){
            if (b.find(str1)!=string::npos || b.find(str2)!=string::npos){
                isCritical.push_back(b);
                if( b.find( str1 )!=string::npos )
                    numberOfCriticalNets++;
            }
            else{
                stringstream iss2(b);
                iss2>>number;
                if (k!=0)
                    pinsToConnect.push_back(number);
                k=1;
            }
        }
        k=0;
        nets.push_back(pinsToConnect);
        pinsToConnect.clear();
    }
    fin.close();

    vector< vector<int> > rearrangeNets( nets.size() );
    vector<int> arrangement( nets.size() ); // record original number of nets : critical nets - noncritical nets
    int flag = 1;
    for( int i = 1; i < nets.size(); i++ )
    {
        if( isCritical[ i ][ 0 ] == 'Y' )
        {
            rearrangeNets[ flag ] = nets[ i ];
            arrangement[ flag ] = i;
            flag++;
        }
        else
        {
            rearrangeNets[ numberOfCriticalNets + 1 ] = nets[ i ];
            arrangement[ numberOfCriticalNets + 1 ] = i;
            numberOfCriticalNets++;
        }
    }

    int board = 0; // metal2 & metal3 |board| metal1 & metal4
    int metal;
    int temp;
    for( int i = 1; i < rearrangeNets.size(); i++ )
    {
        //cout << "metal " ;
        //cout << i << ' ';
        for( int j = 0; j < rearrangeNets[ i ].size(); j++ )
        {
            metal = pins[ rearrangeNets[ i ][ j ] ][ 2 ];
            if( metal == 2 || metal == 3 )
            {
                temp = rearrangeNets[ i ][ board ];
                rearrangeNets[ i ][ board ] = rearrangeNets[ i ][ j ];
                rearrangeNets[ i ][ j ] = temp;
                board++;
                //cout << "board " << board << ' ';
            }
        }
        //cout << endl;
        board = 0;
    }


    //////////// perform routing on the nets ////
//    tmusg.periodStart();

	Routing routing;
    routing.createBoard(maxy, miny, minx, maxx);
	vector< vector< vector<double> > > wires = routing.ConnectNets(pins, blockages, arrangement, rearrangeNets);

//	cout << "ConnectNets ended" << endl;

//    tmusg.getPeriodUsage(stat);
//Check output
/*
for(int i = 1; i < rearrangeNets.size(); i++){
		for(int j = 0; j < rearrangeNets[i].size(); j++)
			cout << rearrangeNets[i][j] << " ";
		cout << endl;
	}
	cout << endl;
	for( int i = 1; i < arrangement.size(); i++ )
    {
        cout << arrangement[ i ] << ' ';
    }
    cout << endl;
    cout << endl;

    for(int i = 1; i < rearrangeNets.size(); i++){
		for(int j = 0; j < rearrangeNets[i].size(); j++)
			cout << rearrangeNets[i][j] << " ";
		cout << endl;
	}
	cout << endl;

for(int i = 1; i < pins.size(); i++){
		for(int j = 0; j < pins[i].size(); j++)
			cout << pins[i][j] << " ";
		cout << endl;
	}
	cout << endl;
	for(int i = 0; i < blockages.size(); i++){
		for(int j = 0; j < blockages[i].size(); j++)
			cout << blockages[i][j] << " ";
    cout << endl;
	}
	cout << endl;
	for(int i = 1; i < nets.size(); i++){
		for(int j = 0; j < nets[i].size(); j++)
			cout << nets[i][j] << " ";
		cout << endl;
	}
	cout << endl;
*/

    //////////// write the output file ///////////

    fstream fout;
    fout.open(argv[4],ios::out);
    for(int i = 1; i < wires.size(); i++){
        fout << "Net " << i << endl;
//        if(wires[i].empty()){
//            fout << "FAIL" << endl;
//        }
        for(int j = 0; j < wires[i].size(); j += 1){
        	for(int k = 0; k < wires[i][j].size(); k += 6){
        		fout <<"+ ";
            	fout << wires[i][j][k] << " " << wires[i][j][k+1] << " ";
            	fout << wires[i][j][k+2] << " " << wires[i][j][k+3] <<" ";
            	fout << wires[i][j][k+4] << " " << wires[i][j][k+5] <<endl;
            }
        }
    }
    fout.close();

    double wiresLength = 0;
    double hor=0, ver=0;
    int numberOfVias = 0;
    double color[ 4 ][ 2 ] = {};
    for(int i = 1; i < wires.size(); i++ )
    {
        for(int j = 0; j < wires[i].size(); j++ )
        {
            if( j > 0 )
               numberOfVias += abs( wires[ i ][ j ][ 4 ] - wires[ i ][ j - 1 ][ 4 ] );
        	for(int k = 0; k < wires[i][j].size(); k+=6 )
        	{
        	    hor += abs( wires[ i ][ j ][ k ] - wires[ i ][ j ][ k + 2 ] );
        	    wiresLength += hor;
        	    ver += abs( wires[ i ][ j ][ k + 1 ] - wires[ i ][ j ][ k + 3 ] );
        	    wiresLength += ver;
        	    //cout << hor << ' ' << ver << endl;
        	    color[ (int)wires[ i ][ j ][ 4 ] - 1 ][ (int)wires[ i ][ j ][ 5 ] - 1 ] += hor;
        	    color[ (int)wires[ i ][ j ][ 4 ] - 1 ][ (int)wires[ i ][ j ][ 5 ] - 1 ] += ver;
        	    hor = 0;
        	    ver = 0;
            }
        }
    }


    cout << "wiresLength : " << wiresLength << endl;
    double colorHor[ 2 ] = {};
    double colorVer[ 2 ] = {};
    colorHor[ 0 ] = color[ 0 ][ 0 ] + color[ 2 ][ 0 ];
    colorHor[ 1 ] = color[ 0 ][ 1 ] + color[ 2 ][ 1 ];
    colorVer[ 0 ] = color[ 1 ][ 0 ] + color[ 3 ][ 0 ];
    colorVer[ 1 ] = color[ 1 ][ 1 ] + color[ 3 ][ 1 ];

/*
    for( int i = 0; i < 4; i++ )
    {
        for( int j = 0; j < 2; j++ )
        {
            cout << "color[ " << i + 1 << " ][ " << j + 1 << " ] : " << color[ i ][ j ] << endl;
        }
    }

    //cout << color[ 0 ][ 0 ] << ' ' << color[ 2 ][ 0 ] << endl;
    cout << "colorHor_1 : " << colorHor[ 0 ] << endl;
    cout << "colorHor_2 : " << colorHor[ 1 ] << endl;
    cout << "colorVer_1 : " << colorVer[ 0 ] << endl;
    cout << "colorVer_2 : " << colorVer[ 1 ] << endl;
    cout << "H : " << abs( colorHor[ 0 ] - colorHor[ 1 ] ) << endl;
    cout << "V : " << abs( colorVer[ 0 ] - colorVer[ 1 ] ) << endl;
    cout << endl;
*/
    cout << "color_1 at metal 1 : " << color[0][0] << endl;
    cout << "color_2 at metal 1 : " << color[0][1] << endl;
    cout << "color_1 at metal 2 : " << color[1][0] << endl;
    cout << "color_2 at metal 2 : " << color[1][1] << endl;
    cout << "color_1 at metal 3 : " << color[2][0] << endl;
    cout << "color_2 at metal 3 : " << color[2][1] << endl;
    cout << "color_1 at metal 4 : " << color[3][0] << endl;
    cout << "color_2 at metal 4 : " << color[3][1] << endl;
    vector<float> checkit;
    for(int i = 1; i < wires.size(); i++){
        //fout << "Net " << i << endl;
//        if(wires[i].empty()){
//            fout << "FAIL" << endl;
//        }
        for(int j = 0; j < wires[i].size(); j += 1){
        	for(int k = 0; k < wires[i][j].size(); k += 6){
        		checkit.push_back(wires[i][j][k]);
        		checkit.push_back(wires[i][j][k+1]);
        		if (wires[i][j][k]-wires[i][j][k+2]==0){
					if (viasmap.find(checkit)==viasmap.end()){//2、判斷是否已被佔據，並回傳層數，若無，則佔住
						viasmap[checkit]=wires[i][j][k+4];
					}
					else{
					vias=vias+abs(wires[i][j][k+4]-viasmap[checkit]);//若有，則代表有vias
					}
					if (wires[i][j][k+1]-wires[i][j][k+3]<0){/*3、將net經過之所有點佔據(不判斷是否已經被佔據之原因
					為在此段中所有線段並非起點或終點，而在交叉過後，雖原本的metal數據被覆蓋，然而由於不可能再有其他
					線連接(4邊皆有連線)，因此不會影響*/
						checkit[1]=checkit[1]+0.5;
						while(checkit[1]<wires[i][j][k+3]){
							viasmap[checkit]=wires[i][j][k+4];
							checkit[1]+=0.5;
				        }
					}
					else{
						checkit[1]-=0.5;
						while(checkit[1]>wires[i][j][k+3]){
							viasmap[checkit]=wires[i][j][k+4];
							checkit[1]-=0.5;
						}
					}
					//4、判斷終點是否有vias
				if (viasmap.find(checkit)==viasmap.end()){
					viasmap[checkit]=wires[i][j][k+4];
				}
				else{
				vias=vias+abs(wires[i][j][k+4]-viasmap[checkit]);
				}
				checkit.clear();
			}
			else {
				if (viasmap.find(checkit)==viasmap.end()){
					viasmap[checkit]=wires[i][j][k+4];
				}
				else{
				vias=vias+abs(wires[i][j][k+4]-viasmap[checkit]);
				}
				if (wires[i][j][k]-wires[i][j][k+2]<0){
					checkit[0]+=0.5;
					while(checkit[0]<wires[i][j][k+2]){
						viasmap[checkit]=wires[i][j][k+4];
						checkit[0]+=0.5;
			        }
				}
				else{
					checkit[0]-=0.5;
					while(checkit[0]>wires[i][j][k+2]){
						viasmap[checkit]=wires[i][j][k+4];
						checkit[0]-=0.5;
					}
				}
				if (viasmap.find(checkit)==viasmap.end()){
					viasmap[checkit]=wires[i][j][k+4];
				}
				else{
				vias=vias+abs(wires[i][j][k+4]-viasmap[checkit]);
				}
				checkit.clear();
			}
        }

        }
    }
    cout << "numberOfVias : " << vias << endl;

//    cout <<"# run time = " << (stat.uTime + stat.sTime) / 1000000.0 << "sec" << endl;
//    cout <<"# memory =" << stat.vmPeak / 1000.0 << "MB" << endl;

    return 0;
}
