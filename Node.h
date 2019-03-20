#ifndef NODE_H
#define NODE_H

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

#endif // NODE_H
