#ifndef ROUTING_H
#define ROUTING_H

#include <vector>
#include <queue>
#include <iostream>
using namespace std;

class Node
{
public:
    Node(int x, int y)
    :x_coordinate(x), y_coordinate(y)
    {
        status = 'a';
        parent = 'n';
        layer = 0;
        distance = 0;
        blocked = false;
    }

    int getX() const{
        return x_coordinate;
    }
    int getY() const{
        return y_coordinate;
    }

    void setStatus(char s){
        status = s;
    }
    char getStatus() const{
        return status;
    }
    void setParent(char p){
        parent = p;
    }
    char getParent() const{
        return parent;
    }

    void setLayer(int l){
        layer = l;
    }
    int getLayer(){
        return layer;
    }

    void addDistance(int d){
        distance += d;
    }
    void resetDistance(){
        distance = 0;
    }
    int getDistance(){
        return distance;
    }

    bool isBlocked(){
        return blocked;
    }
    void setBlock(bool b){
        blocked = b;
    }

    bool operator==(const Node &right) const{
        return distance == right.distance;
    }
    bool operator<(const Node &right) const{
        return distance < right.distance;
    }
    bool operator<=(const Node &right) const{
        return distance <= right.distance;
    }
    bool operator>(const Node &right) const{
        return distance > right.distance;
    }
    bool operator>=(const Node &right) const{
        return distance >= right.distance;
    }

private:
    int x_coordinate;
    int y_coordinate;

    char status; //a:available, f:found, d:destination
                 //cases of occupations: h:horizontal, v:vertical, b:both, l:locked
    char parent; //n:null, r:rightward, u:upward, l:leftward, d:downward
    int layer; //0:none, 1, 2, 3, 4
    int distance;

    bool blocked;
};


class Routing
{
public:
    Routing(){
        unconnected = 0;
    }
    void createBoard(double, double, double, double);
    vector< vector< vector<double> > > &ConnectNets(vector< vector<double> > &, vector< vector<double> > &,
                                                    vector<int> &, vector< vector<int> > &);

private:
    int virtX(double) const; //transform real position onto virtual board
    int virtY(double) const;
    double realX(int) const; //transform position on virtual board back to real one
    double realY(int) const;

    void setBlock(vector< vector<double> > &, vector< vector<double> > &); //put blockages and pins on the board
    void lockPin(Node &);
    void unlockPin(Node &);

    void BFS(vector< vector<double> > &, vector<int> &, vector< vector<int> > &); //connect pins in each net
    int quadrant(Node &); //1:r&u 2:l&u 3:l&d 4:r&d
    bool findNext(priority_queue< Node, vector<Node>, greater<Node> > &, vector<int> &, int &, Node &);
    void traceBack(int, Node &, Node &);
    void clearFound();
    void occupyDest();

    void coloring();
    void colorBalancing( double, int, int );
    void horizontalBalancing();
    void verticalBalancing();
    bool colorDistributionCompare( int );
    double countColorDiff( double, double );
    void recordColorConflict();
    void fixColoring( vector<double> &, int, int );

    int x_width; //horizontal width of the board
    int y_width;
    int x_shift; //distance needed to move leftmost point to 11th point on the board
    int y_shift;
    double color[ 4 ][ 2 ];
    double colorHor[ 2 ];
    double colorVer[ 2 ];
    double horColorDiff;
    double verColorDiff;
/*
    vector< vector<int> > layer; //0:none, 1, 2, 3, 4
    vector< vector<char> > status; //a:available, f:found, d:destination
                                   //cases of occupations: h:horizontal, v:vertical, b:both
    vector< vector<char> > parent; //n:none, r:rightward, u:upward, l:leftward, d:downward
*/
    vector< vector<Node> > circuit;

    vector< vector< vector<double> > > wires;

    int unconnected; //#of unconnected pins
};

#endif
