#include "Routing.h"
#include<cmath>

void Routing::createBoard(double u, double d, double l, double r){
    x_width = 2 * (r - l) + 101; //leftest point is at the 51th point on the board
    y_width = 2 * (u - d) + 101;
    x_shift = 50 - l * 2;
    y_shift = 50 - d * 2;

    circuit.resize(x_width);
    for(int i = 0; i < x_width; i++){
        for(int j = 0; j < y_width; j++){
            Node node(i, j);
            circuit[i].push_back(node);
        }
    }
/*
    //test if circuit set
    for(int i = y_width - 1; i >= 0; i--){
        for(int j = 0; j < x_width; j++){
            cout << "(" << circuit[j][i].getX() << ", " << circuit[j][i].getY() << ") ";
        }
        cout << endl;
    }
*/

    for( int i = 0; i < 4; i++ )
    {
        for( int j = 0; j < 2; j++ )
            color[ i ][ j ] = 0.0;
    }
    for( int i = 0; i < 2; i++ )
    {
        colorHor[ i ] = 0.0;
        colorVer[ i ] = 0.0;
    }
    horColorDiff = verColorDiff = 0.0;
}

int Routing::virtX(double x) const{
    return x * 2 + x_shift;
}
int Routing::virtY(double y) const{
    return y * 2 + y_shift;
}
double Routing::realX(int x) const{
    return ((double)(x - x_shift)) / 2;
}
double Routing::realY(int y) const{
    return ((double)(y - y_shift)) / 2;
}


vector< vector< vector<double> > > &Routing::ConnectNets(vector< vector<double> > &pins, vector< vector<double> > &blockages,
                                                          vector<int> &netOrder, vector< vector<int> > &nets){
    setBlock(pins, blockages);
    BFS(pins, netOrder, nets);
    cout << unconnected << " pins unconnected\n";
    coloring();
    /*
    cout << "in" << endl;
    cout << "colorHor_1 : " << colorHor[ 0 ] << endl;
    cout << "colorHor_2 : " << colorHor[ 1 ] << endl;
    cout << "colorVer_1 : " << colorVer[ 0 ] << endl;
    cout << "colorVer_2 : " << colorVer[ 1 ] << endl;
    cout << "H : " << abs( colorHor[ 0 ] - colorHor[ 1 ] ) << endl;
    cout << "V : " << abs( colorVer[ 0 ] - colorVer[ 1 ] ) << endl;
    cout << '\n';
    */
    recordColorConflict();
//    cout << "conflict solved\n";

    return wires;
}

void Routing::setBlock(vector< vector<double> > &pins, vector< vector<double> > &blockages){
    vector< vector<double> >::iterator it;
	for(it = blockages.begin(); it != blockages.end(); it++){
		for(int i = virtX((*it)[0]); i <= virtX((*it)[2]); i++){
			for(int j = virtY((*it)[1]); j <= virtY((*it)[3]); j++)
				circuit[i][j].setStatus('b');
		}
	}
    for(it = pins.begin() + 1; it != pins.end(); it++){
        circuit[virtX((*it)[0])][virtY((*it)[1])].setStatus('b');
        circuit[virtX((*it)[0])][virtY((*it)[1])].setLayer((*it)[2]);
        lockPin(circuit[virtX((*it)[0])][virtY((*it)[1])]);
    }
/*
    //test
    for(int i = y_width - 1; i >= 0; i--){
        for(int j = 0; j < x_width; j++){
            cout << circuit[j][i].getStatus() << " ";
        }
        cout << endl;
    }
*/
}

void Routing::lockPin(Node &pin){
    if(pin.getX() + 1 < x_width && circuit[pin.getX() + 1][pin.getY()].getStatus() != 'b')
        circuit[pin.getX() + 1][pin.getY()].setStatus('l');
    if(pin.getY() + 1 < y_width && circuit[pin.getX()][pin.getY() + 1].getStatus() != 'b')
        circuit[pin.getX()][pin.getY() + 1].setStatus('l');
    if(pin.getX() - 1 >= 0 && circuit[pin.getX() - 1][pin.getY()].getStatus() != 'b')
        circuit[pin.getX() - 1][pin.getY()].setStatus('l');
    if(pin.getY() - 1 >= 0 && circuit[pin.getX()][pin.getY() - 1].getStatus() != 'b')
        circuit[pin.getX()][pin.getY() - 1].setStatus('l');
}

void Routing::unlockPin(Node &pin){
    if(pin.getX() + 1 < x_width && circuit[pin.getX() + 1][pin.getY()].getStatus() == 'l')
        circuit[pin.getX() + 1][pin.getY()].setStatus('a');
    if(pin.getY() + 1 < y_width && circuit[pin.getX()][pin.getY() + 1].getStatus() == 'l')
        circuit[pin.getX()][pin.getY() + 1].setStatus('a');
    if(pin.getX() - 1 >= 0 && circuit[pin.getX() - 1][pin.getY()].getStatus() == 'l')
        circuit[pin.getX() - 1][pin.getY()].setStatus('a');
    if(pin.getY() - 1 >= 0 && circuit[pin.getX()][pin.getY() - 1].getStatus() == 'l')
        circuit[pin.getX()][pin.getY() - 1].setStatus('a');
}

void Routing::BFS(vector< vector<double> > &pins, vector<int> &netOrder, vector< vector<int> > &nets){
    wires.resize(netOrder.size());
    int order = 0, currentNet;

    vector< vector<int> >::iterator it;
	vector<int>::iterator it2;
	for(it = nets.begin() + 1; it != nets.end(); it++){
        currentNet = netOrder[++order];

        //determine the first end node (first pin) for BST
        it2 = (*it).begin();
        //circuit[virtX(pins[*it2][0])][virtY(pins[*it2][1])].setLayer(pins[*it2][2]);
        circuit[virtX(pins[*it2][0])][virtY(pins[*it2][1])].setStatus('d');
        unlockPin(circuit[virtX(pins[*it2][0])][virtY(pins[*it2][1])]);

        for(it2 = it2 + 1; it2 != (*it).end(); it2++){
            priority_queue< Node, vector<Node>, greater<Node> > storage;

            circuit[virtX(pins[*it2][0])][virtY(pins[*it2][1])].setLayer(pins[*it2][2]);
            circuit[virtX(pins[*it2][0])][virtY(pins[*it2][1])].setStatus('f');
            unlockPin(circuit[virtX(pins[*it2][0])][virtY(pins[*it2][1])]);

            Node beg(circuit[virtX(pins[*it2][0])][virtY(pins[*it2][1])]);
            storage.push(beg);

            vector<int> layer_n(5, 0);
            int quad = quadrant(circuit[virtX(pins[*it2][0])][virtY(pins[*it2][1])]);
            Node fin(0, 0);
            while(!storage.empty()){
                if(findNext(storage, layer_n, quad, fin)) break;
            }

//            if(!storage.empty()) cout << "wire found\n";
            if(storage.empty()){ //routing failed
                //cerr << "pins unconnected" << endl;
                unconnected++;
                clearFound();
                break;
            }
            traceBack(currentNet, fin, beg); //note that fin and beg switched in the function
            clearFound();
        }
        occupyDest();
	}
}

int Routing::quadrant(Node &beg){
    int del_x = 0, del_y = 0;
    for(int i = 0; i < x_width; i++){
        for(int j = 0; j < y_width; j++){
            if(circuit[i][j].getStatus() == 'd'){
                del_x += circuit[i][j].getX() - beg.getX();
                del_y += circuit[i][j].getY() - beg.getY();
            }
        }
    }
    if(del_x > 0){
        if(del_y > 0) return 1;
        return 4;
    }
    if(del_y > 0) return 2;
    return 3;
}

bool Routing::findNext(priority_queue< Node, vector<Node>, greater<Node> > &storage, vector<int> &layer_n, int &quad, Node &fin){
    Node cur(storage.top());
    storage.pop();
    //find rightward
    if(cur.getX() + 1 < x_width){
        if(circuit[cur.getX() + 1][cur.getY()].getStatus() == 'a' || circuit[cur.getX() + 1][cur.getY()].getStatus() == 'v'){
            if(circuit[cur.getX() + 1][cur.getY()].getStatus() == 'v') circuit[cur.getX() + 1][cur.getY()].setBlock(true);

            circuit[cur.getX() + 1][cur.getY()].setParent('l');
            if(quad == 2 || quad == 3) circuit[cur.getX() + 1][cur.getY()].addDistance(2);
            //determine layer
            if(cur.getLayer() == 1 || cur.getLayer() == 3) circuit[cur.getX() + 1][cur.getY()].setLayer(cur.getLayer());
            else if(cur.getLayer() == 4){
                circuit[cur.getX() + 1][cur.getY()].setLayer(3);
                layer_n[3]++;
                circuit[cur.getX() + 1][cur.getY()].addDistance(1);
            }
            else{ //layer 2
                if(layer_n[1] < layer_n[3]){
                    circuit[cur.getX() + 1][cur.getY()].setLayer(1);
                    layer_n[1]++;
                    circuit[cur.getX() + 1][cur.getY()].addDistance(1);
                }
                else{
                    circuit[cur.getX() + 1][cur.getY()].setLayer(3);
                    layer_n[3]++;
                    circuit[cur.getX() + 1][cur.getY()].addDistance(1);
                }
            }

            circuit[cur.getX() + 1][cur.getY()].setStatus('f');
            storage.push(circuit[cur.getX() + 1][cur.getY()]);
        }
        else if(circuit[cur.getX() + 1][cur.getY()].getStatus() == 'd'){
            circuit[cur.getX() + 1][cur.getY()].setParent('l');
            storage.push(circuit[cur.getX() + 1][cur.getY()]);
            fin = circuit[cur.getX() + 1][cur.getY()];
            return true;
        }
    }
    //find upward
    if(cur.getY() + 1 < y_width){
        if(circuit[cur.getX()][cur.getY() + 1].getStatus() == 'a' || circuit[cur.getX()][cur.getY() + 1].getStatus() == 'h'){
            if(circuit[cur.getX()][cur.getY() + 1].getStatus() == 'h') circuit[cur.getX()][cur.getY() + 1].setBlock(true);

            circuit[cur.getX()][cur.getY() + 1].setParent('d');
            if(quad == 3 || quad == 4) circuit[cur.getX()][cur.getY() + 1].addDistance(2);
            //determine layer
            if(cur.getLayer() == 2 || cur.getLayer() == 4) circuit[cur.getX()][cur.getY() + 1].setLayer(cur.getLayer());
            else if(cur.getLayer() == 1){
                circuit[cur.getX()][cur.getY() + 1].setLayer(2);
                layer_n[2]++;
                circuit[cur.getX()][cur.getY() + 1].addDistance(1);
            }
            else{ //layer 3
                if(layer_n[2] <= layer_n[4]){
                    circuit[cur.getX()][cur.getY() + 1].setLayer(2);
                    layer_n[2]++;
                    circuit[cur.getX()][cur.getY() + 1].addDistance(1);
                }
                else{
                    circuit[cur.getX()][cur.getY() + 1].setLayer(4);
                    layer_n[4]++;
                    circuit[cur.getX()][cur.getY() + 1].addDistance(1);
                }
            }

            circuit[cur.getX()][cur.getY() + 1].setStatus('f');
            storage.push(circuit[cur.getX()][cur.getY() + 1]);
        }
        else if(circuit[cur.getX()][cur.getY() + 1].getStatus() == 'd'){
            circuit[cur.getX()][cur.getY() + 1].setParent('d');
            storage.push(circuit[cur.getX()][cur.getY() + 1]);
            fin = circuit[cur.getX()][cur.getY() + 1];
            return true;
        }
    }
    //find leftward
    if(cur.getX() - 1 >= 0){
        if(circuit[cur.getX() - 1][cur.getY()].getStatus() == 'a' || circuit[cur.getX() - 1][cur.getY()].getStatus() == 'v'){
            if(circuit[cur.getX() - 1][cur.getY()].getStatus() == 'v') circuit[cur.getX() - 1][cur.getY()].setBlock(true);

            circuit[cur.getX() - 1][cur.getY()].setParent('r');
            if(quad == 1 || quad == 4) circuit[cur.getX() - 1][cur.getY()].addDistance(2);
            //determine layer
            if(cur.getLayer() == 1 || cur.getLayer() == 3) circuit[cur.getX() - 1][cur.getY()].setLayer(cur.getLayer());
            else if(cur.getLayer() == 4){
                circuit[cur.getX() - 1][cur.getY()].setLayer(3);
                layer_n[3]++;
                circuit[cur.getX() - 1][cur.getY()].addDistance(1);
            }
            else{ //layer 2
                if(layer_n[1] < layer_n[3]){
                    circuit[cur.getX() - 1][cur.getY()].setLayer(1);
                    layer_n[1]++;
                    circuit[cur.getX() - 1][cur.getY()].addDistance(1);
                }
                else{
                    circuit[cur.getX() - 1][cur.getY()].setLayer(3);
                    layer_n[3]++;
                    circuit[cur.getX() - 1][cur.getY()].addDistance(1);
                }
            }

            circuit[cur.getX() - 1][cur.getY()].setStatus('f');
            storage.push(circuit[cur.getX() - 1][cur.getY()]);
        }
        else if(circuit[cur.getX() - 1][cur.getY()].getStatus() == 'd'){
            circuit[cur.getX() - 1][cur.getY()].setParent('r');
            storage.push(circuit[cur.getX() - 1][cur.getY()]);
            fin = circuit[cur.getX() - 1][cur.getY()];
            return true;
        }
    }
    //find downward
    if(cur.getY() - 1 >= 0){
        if(circuit[cur.getX()][cur.getY() - 1].getStatus() == 'a' || circuit[cur.getX()][cur.getY() - 1].getStatus() == 'h'){
            if(circuit[cur.getX()][cur.getY() - 1].getStatus() == 'h') circuit[cur.getX()][cur.getY() - 1].setBlock(true);

            circuit[cur.getX()][cur.getY() - 1].setParent('u');
            if(quad == 1 || quad == 2) circuit[cur.getX()][cur.getY() - 1].addDistance(2);
            //determine layer
            if(cur.getLayer() == 2 || cur.getLayer() == 4) circuit[cur.getX()][cur.getY() - 1].setLayer(cur.getLayer());
            else if(cur.getLayer() == 1){
                circuit[cur.getX()][cur.getY() - 1].setLayer(2);
                layer_n[2]++;
                circuit[cur.getX()][cur.getY() - 1].addDistance(1);
            }
            else{ //layer 3
                if(layer_n[2] <= layer_n[4]){
                    circuit[cur.getX()][cur.getY() - 1].setLayer(2);
                    layer_n[2]++;
                    circuit[cur.getX()][cur.getY() - 1].addDistance(1);
                }
                else{
                    circuit[cur.getX()][cur.getY() - 1].setLayer(4);
                    layer_n[4]++;
                    circuit[cur.getX()][cur.getY() - 1].addDistance(1);
                }
            }

            circuit[cur.getX()][cur.getY() - 1].setStatus('f');
            storage.push(circuit[cur.getX()][cur.getY() - 1]);
        }
        else if(circuit[cur.getX()][cur.getY() - 1].getStatus() == 'd'){
            circuit[cur.getX()][cur.getY() - 1].setParent('u');
            storage.push(circuit[cur.getX()][cur.getY() - 1]);
            fin = circuit[cur.getX()][cur.getY() - 1];
            return true;
        }
    }

    return false;
}

void Routing::traceBack(int n, Node &cur, Node &fin){
    vector<double> emp;
    wires[n].push_back(emp);

    int l = wires[n].size() - 1; //l == 0
    wires[n][l].push_back(realX(cur.getX()));
    wires[n][l].push_back(realY(cur.getY()));

    circuit[cur.getX()][cur.getY()].setBlock(true); //note that cur does not equal circuit[][]

    while((cur.getX() != fin.getX()) || (cur.getY() != fin.getY())){
        //trace rightward
        if(cur.getParent() == 'r'){
            cur = circuit[cur.getX() + 1][cur.getY()];
            //record when net turns
            if(cur.getParent() != 'r'){
                wires[n][l].push_back(realX(cur.getX()));
                wires[n][l].push_back(realY(cur.getY()));
                wires[n][l].push_back(circuit[cur.getX() - 1][cur.getY()].getLayer());

                wires[n].push_back(emp);
                wires[n][++l].push_back(realX(cur.getX()));
                wires[n][l].push_back(realY(cur.getY()));

                circuit[cur.getX()][cur.getY()].setBlock(true);
            }
        }
        //trace upward
        else if(cur.getParent() == 'u'){
            cur = circuit[cur.getX()][cur.getY() + 1];
            if(cur.getParent() != 'u'){
                wires[n][l].push_back(realX(cur.getX()));
                wires[n][l].push_back(realY(cur.getY()));
                wires[n][l].push_back(circuit[cur.getX()][cur.getY() - 1].getLayer());

                wires[n].push_back(emp);
                wires[n][++l].push_back(realX(cur.getX()));
                wires[n][l].push_back(realY(cur.getY()));

                circuit[cur.getX()][cur.getY()].setBlock(true);
            }
        }
        //trace leftward
        else if(cur.getParent() == 'l'){
            cur = circuit[cur.getX() - 1][cur.getY()];
            if(cur.getParent() != 'l'){
               wires[n][l].push_back(realX(cur.getX()));
               wires[n][l].push_back(realY(cur.getY()));
               wires[n][l].push_back(circuit[cur.getX() + 1][cur.getY()].getLayer());

               wires[n].push_back(emp);
               wires[n][++l].push_back(realX(cur.getX()));
               wires[n][l].push_back(realY(cur.getY()));

               circuit[cur.getX()][cur.getY()].setBlock(true);
            }
        }
        //trace downward
        else if(cur.getParent() == 'd'){
            cur = circuit[cur.getX()][cur.getY() - 1];
            if(cur.getParent() != 'd'){
                wires[n][l].push_back(realX(cur.getX()));
                wires[n][l].push_back(realY(cur.getY()));
                wires[n][l].push_back(circuit[cur.getX()][cur.getY() + 1].getLayer());

                wires[n].push_back(emp);
                wires[n][++l].push_back(realX(cur.getX()));
                wires[n][l].push_back(realY(cur.getY()));

                circuit[cur.getX()][cur.getY()].setBlock(true);
            }
        }
        circuit[cur.getX()][cur.getY()].setStatus('d');
    }
    wires[n].pop_back(); //no need to find new wire
}

void Routing::clearFound(){
    for(int i = 0; i < x_width; i++){
        for(int j = 0; j < y_width; j++){
            if(circuit[i][j].getStatus() == 'f'){
                circuit[i][j].setStatus('a');
                circuit[i][j].setLayer(0);
                circuit[i][j].setParent('n');
                circuit[i][j].resetDistance();
                circuit[i][j].setBlock(false);
            }
        }
    }
}
void Routing::occupyDest(){
    for(int i = 0; i < x_width; i++){
        for(int j = 0; j < y_width; j++){
            if(circuit[i][j].getStatus() == 'd'){
                if(circuit[i][j].isBlocked()) circuit[i][j].setStatus('b');
                else if(circuit[i][j].getLayer() == 1 || circuit[i][j].getLayer() == 3) circuit[i][j].setStatus('h');
                else circuit[i][j].setStatus('v');

                circuit[i][j].resetDistance();
            }
        }
    }
}

void Routing::coloring()
{
    for( int i = 1; i < wires.size(); i++ )
    {
        for( int j = 0; j < wires[ i ].size(); j++ )
        {
            if( ( ceil( wires[ i ][ j ][ 0 ] ) == floor( wires[ i ][ j ][ 0 ] ) )
                && ( ceil( wires[ i ][ j ][ 1 ] ) == floor( wires[ i ][ j ][ 1 ] ) ) )
            {
                wires[ i ][ j ].push_back( 1 ); // "integer lines" with color1
            }
            else
                wires[ i ][ j ].push_back( 2 );// "non-integer lines with color2
        }
    }
    double hor, ver;
    hor = ver = 0.0;
    for(int i = 1; i < wires.size(); i++ )
    {
        for(int j = 0; j < wires[i].size(); j++ )
        {
        	for(int k = 0; k < wires[i][j].size(); k+=6 )
        	{
        	    hor += abs( wires[ i ][ j ][ k ] - wires[ i ][ j ][ k + 2 ] );
        	    ver += abs( wires[ i ][ j ][ k + 1 ] - wires[ i ][ j ][ k + 3 ] );
        	    color[ (int)wires[ i ][ j ][ 4 ] - 1 ][ (int)wires[ i ][ j ][ 5 ] - 1 ] += hor;
        	    color[ (int)wires[ i ][ j ][ 4 ] - 1 ][ (int)wires[ i ][ j ][ 5 ] - 1 ] += ver;
        	    hor = ver = 0.0;
            }
        }
    }
    colorHor[ 0 ] = color[ 0 ][ 0 ] + color[ 2 ][ 0 ];
    colorHor[ 1 ] = color[ 0 ][ 1 ] + color[ 2 ][ 1 ];
    colorVer[ 0 ] = color[ 1 ][ 0 ] + color[ 3 ][ 0 ];
    colorVer[ 1 ] = color[ 1 ][ 1 ] + color[ 3 ][ 1 ];
    horColorDiff = countColorDiff( colorHor[ 0 ], colorHor[ 1 ] );
    verColorDiff = countColorDiff( colorVer[ 0 ], colorVer[ 1 ] );
    //check initial
    /*
    cout << "H : " << colorHor[ 0 ] << ' ' << colorHor[ 1 ] << endl;
    cout << "V : " << colorVer[ 0 ] << ' ' << colorVer[ 1 ] << endl;
    cout << horColorDiff << ' ' << verColorDiff << endl;
    cout << endl;
    */
    /*cout << "color_1 at metal 1 : " << color[0][0] << endl;
    cout << "color_2 at metal 1 : " << color[0][1] << endl;
    cout << "color_1 at metal 2 : " << color[1][0] << endl;
    cout << "color_2 at metal 2 : " << color[1][1] << endl;
    cout << "color_1 at metal 3 : " << color[2][0] << endl;
    cout << "color_2 at metal 3 : " << color[2][1] << endl;
    cout << "color_1 at metal 4 : " << color[3][0] << endl;
    cout << "color_1 at metal 4 : " << color[3][1] << endl;*/
    horizontalBalancing();
    verticalBalancing();
}

double Routing::countColorDiff( double colorOne, double colorTwo )
{
    return abs( colorOne - colorTwo );
}

void Routing::horizontalBalancing()
{
    bool pre_flag, next_flag;
    for( int j = 0; j < y_width - 1; j++ )
    {
        pre_flag = next_flag = false; // false : not a blank row
        for( int i = 0; i < x_width; i++ )
        {
            if( circuit[i][j].getLayer() == 1 || circuit[i][j].getLayer() == 3 )
                pre_flag = true; // is blank row
            if( circuit[i][j + 1].getLayer() == 1 || circuit[i][j + 1].getLayer() == 3 )
                next_flag = true; // is blank row
        }
        if( pre_flag == true && next_flag == false )
        {
            colorBalancing( realY( j ), 1, 3 );
            if( !colorDistributionCompare( 1 ) )
                colorBalancing( realY( j ), 1, 3 );
        }
    }
}

void Routing::verticalBalancing()
{
    bool pre_flag, next_flag;
    for( int i = 0; i < x_width - 1; i++ )
    {
        pre_flag = next_flag = false; // false : not a blank column
        for( int j = 0; j < y_width; j++ )
        {
            if( circuit[i][j].getLayer() == 2 || circuit[i][j].getLayer() == 4 )
                pre_flag = true; // is blank column
            if( circuit[i + 1][j].getLayer() == 2 || circuit[i + 1][j].getLayer() == 4 )
                next_flag = true; // is blank column
        }
        if( pre_flag == true && next_flag == false )
        {
            colorBalancing( realX( i ), 2, 4 );
            if( !colorDistributionCompare( 2 ) )
                colorBalancing( realX( i ), 2, 4 );
        }
    }
}

bool Routing::colorDistributionCompare( int direction )
{
    double recolor[ 4 ][ 2 ] = {};
    double recolorHor[ 2 ] = {};
    double recolorVer[ 2 ] = {};
    double hor, ver;
    hor = ver = 0.0;
    double recolorDiff = 0.0;
    if( direction == 1 )
    {
         for(int i = 1; i < wires.size(); i++ )
         {
             for(int j = 0; j < wires[i].size(); j++ )
             {
        	     for(int k = 0; k < wires[i][j].size(); k+=6 )
        	     {
        	         hor += abs( wires[ i ][ j ][ k ] - wires[ i ][ j ][ k + 2 ] );
             	     recolor[ (int)wires[ i ][ j ][ 4 ] - 1 ][ (int)wires[ i ][ j ][ 5 ] - 1 ] += hor;
             	     //cout << hor << ' ' << ver << endl;
        	         hor = 0.0;
                 }
             }
         }
         recolorHor[ 0 ] = recolor[ 0 ][ 0 ] + recolor[ 2 ][ 0 ];
         recolorHor[ 1 ] = recolor[ 0 ][ 1 ] + recolor[ 2 ][ 1 ];
         recolorDiff = countColorDiff( recolorHor[ 0 ], recolorHor[ 1 ] );
         //cout << "recolorDiff H: " << recolorDiff << ' ' << "horColorDiff : " << horColorDiff << endl;
         if( recolorDiff > horColorDiff )
         {
             recolorDiff = 0.0;
             recolorHor[ 0 ] = 0.0;
             recolorHor[ 1 ] = 0.0;
             return false;
         }
         else
         {
             horColorDiff = recolorDiff;
             colorHor[ 0 ] = recolorHor[ 0 ];
             colorHor[ 1 ] = recolorHor[ 1 ];
             recolorDiff = 0.0;
             recolorHor[ 0 ] = 0.0;
             recolorHor[ 1 ] = 0.0;
         }
         //cout << "H after : " << horColorDiff << endl;
    }

    else if( direction == 2 )
    {
        for(int i = 1; i < wires.size(); i++ )
        {
            for(int j = 0; j < wires[i].size(); j++ )
            {
        	    for(int k = 0; k < wires[i][j].size(); k+=6 )
        	    {
        	        ver += abs( wires[ i ][ j ][ k + 1 ] - wires[ i ][ j ][ k + 3 ] );
        	        recolor[ (int)wires[ i ][ j ][ 4 ] - 1 ][ (int)wires[ i ][ j ][ 5 ] - 1 ] += ver;
        	        //cout << hor << ' ' << ver << endl;
        	        ver = 0.0;
                }
            }
        }
        recolorVer[ 0 ] = recolor[ 1 ][ 0 ] + recolor[ 3 ][ 0 ];
        recolorVer[ 1 ] = recolor[ 1 ][ 1 ] + recolor[ 3 ][ 1 ];
        recolorDiff = countColorDiff( recolorVer[ 0 ], recolorVer[ 1 ] );
        //cout << "recolorDiff V: " << recolorDiff << ' ' << "verColorDiff : " << verColorDiff << endl;
        if( recolorDiff > verColorDiff )
        {
            recolorDiff = 0.0;
            recolorVer[ 0 ] = 0.0;
            recolorVer[ 1 ] = 0.0;
            return false;
        }
        else
        {
            verColorDiff = recolorDiff;
            colorVer[ 0 ] = recolorVer[ 0 ];
            colorVer[ 1 ] = recolorVer[ 1 ];
            recolorVer[ 0 ] = 0.0;
            recolorVer[ 1 ] = 0.0;
            recolorDiff = 0.0;
        }
        //cout << "V after : " << verColorDiff << endl;
    }
}

void Routing::colorBalancing( double startBalancingLine, int metalOne, int metalTwo )
{
    for(int i = 1; i < wires.size(); i++ )
    {
        for(int j = 0; j < wires[i].size(); j++ )
        {
        	for(int k = 0; k < wires[i][j].size(); k+=6 )
        	{
        	    if( wires[ i ][ j ][ 4 ] != metalOne && wires[ i ][ j ][ 4 ] != metalTwo ) continue;

        	    if( metalOne == 1 )
                {
                    //cout << "check" << endl;
                    if( wires[ i ][ j ][ 1 ] > startBalancingLine )
                    {
                        if( wires[ i ][ j ][ 5 ] == 1 )
                           wires[ i ][ j ][ 5 ] = 2;
                        else if( wires[ i ][ j ][ 5 ] == 2 )
                           wires[ i ][ j ][ 5 ] = 1;
                    }
                }
                else if( metalOne == 2 )
                {
                    if( wires[ i ][ j ][ 0 ] > startBalancingLine )
                    {
                        if( wires[ i ][ j ][ 5 ] == 1 )
                           wires[ i ][ j ][ 5 ] = 2;
                        else if( wires[ i ][ j ][ 5 ] == 2 )
                           wires[ i ][ j ][ 5 ] = 1;
                    }
                }
            }
        }
    }
}

void Routing::recordColorConflict()
{
    vector<double> horizontal_conflict;
    vector<double> vertical_conflict;
    //record horizontal conflicts
    int k = 0;
    for( int j = 0; j < y_width; j++ )
    {
        int flag = 0;
        for( int i = 0; i < x_width; i++ )
        {
            if( circuit[i][j].getLayer() == 1 || circuit[i][j].getLayer() == 3 )
            {
                if( flag == 0)
                   flag++;
                else if( flag == 2 )
                {
                    //record this position - conflict
                    horizontal_conflict.push_back( realX( i - 2 ) );
                    horizontal_conflict.push_back( realY( j ) );
                    horizontal_conflict.push_back( realX( i ) );
                    horizontal_conflict.push_back( realY( j ) );
                    flag = 0;
                }
                else
                {
                    if( circuit[i + 1][j].getLayer() == 1 || circuit[i + 1][j].getLayer() == 3 )
                        flag = 0;
                    else
                        continue;
                }
            }
            else if( circuit[i][j].getLayer() != 1 && circuit[i][j].getLayer() != 3 && flag == 1 )
            {
                flag++;
            }
            else
                flag = 0;
        }

        k++;
    }
    //record vertical conflicts
    for( int i = 0; i < x_width; i++ )
    {
        int flag = 0;
        for( int j = 0; j < y_width; j++ )
        {
            if( circuit[i][j].getLayer() == 2 || circuit[i][j].getLayer() == 4 )
            {
                if( flag == 0)
                   flag++;
                else if( flag == 2 )
                {
                    //record this position - conflict
                    vertical_conflict.push_back( realX( i ) );
                    vertical_conflict.push_back( realY( j ) );
                    vertical_conflict.push_back( realX( i ) );
                    vertical_conflict.push_back( realY( j - 2 ) );
                    flag = 0;
                }
                else
                {
                    if( circuit[i][j + 1].getLayer() == 1 || circuit[i][j + 1].getLayer() == 3 )
                        flag = 0;
                    else
                        continue;
                }
            }
            else if( circuit[i][j].getLayer() != 2 && circuit[i][j].getLayer() != 4 && flag == 1 )
            {
                flag++;
            }
            else
                flag = 0;
        }
    }
    if( !horizontal_conflict.empty() )
       fixColoring( horizontal_conflict, 1, 3 );
    if( !vertical_conflict.empty() )
       fixColoring( vertical_conflict, 2, 4 );
}

void Routing::fixColoring( vector<double> & conflict , int metalOne, int metalTwo )
{
    int metalOfLine1, metalOfLine2;
    int iChange, jChange;
    for( int t = 0; t < conflict.size() - 3; t += 4 )
    {
        for( int i = 1; i < wires.size(); i++ )
        {
            for( int j = 0; j < wires[ i ].size(); j++ )
            {
                if( wires[ i ][ j ][ 4 ] != metalOne && wires[ i ][ j ][ 4 ] != metalTwo ) continue;

                if( ( conflict[ t ] == wires[ i ][ j ][ 0 ] && conflict[ t + 1 ] == wires[ i ][ j ][ 1 ] )
                      || ( conflict[ t ] == wires[ i ][ j ][ 2 ] && conflict[ t + 1 ] == wires[ i ][ j ][ 3 ] ) )
                   metalOfLine1 = wires[ i ][ j ][ 4 ];

                if( ( conflict[ t + 2 ] == wires[ i ][ j ][ 0 ] && conflict[ t + 3 ] == wires[ i ][ j ][ 1 ] )
                      || ( conflict[ t + 2 ] == wires[ i ][ j ][ 2 ] && conflict[ t + 3 ] == wires[ i ][ j ][ 3 ] ) )
                   metalOfLine2 = wires[ i ][ j ][ 4 ];
                   iChange = i;
                   jChange = j;
            }
        }
        if( metalOfLine1 == metalOfLine2 )
        {
            switch( metalOfLine1 )
            {
                case 1:   wires[ iChange ][ jChange ][ 4 ] = 3; break;
                case 2:   wires[ iChange ][ jChange ][ 4 ] = 2; break;
                case 3:   wires[ iChange ][ jChange ][ 4 ] = 1; break;
                case 4:   wires[ iChange ][ jChange ][ 4 ] = 4; break;
            }
        }
    }
}
