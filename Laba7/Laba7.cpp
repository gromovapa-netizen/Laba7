#include <iostream>
#include <algorithm>
#include <cmath>
#include <string>

using namespace std;

class Figure {
protected:
    double x, y;
public:
    Figure(double x = 0, double y = 0) : x(x), y(y) {}
    virtual ~Figure() {}

    virtual string getName() const = 0;
    virtual double getSize() const = 0;
    virtual Figure* clone() const = 0;
    virtual void increaseSize(double delta) = 0;

    void print() const {
        cout << getName() << "(" << x << ";" << y << ") S=" << getSize();
    }
};

class Point : public Figure {
public:
    Point(double x, double y) : Figure(x, y) {}
    string getName() const override { return "Точка"; }
    double getSize() const override { return 0; }
    Figure* clone() const override { return new Point(*this); }
    void increaseSize(double delta) override {}
};

class Line : public Figure {
protected:
    double length;
public:
    Line(double x, double y, double len) : Figure(x, y), length(len) {}
    string getName() const override { return "Линия"; }
    double getSize() const override { return length; }
    Figure* clone() const override { return new Line(*this); }
    void increaseSize(double delta) override { length = max(0.0, length + delta); }
};

class Square : public Line {
public:
    Square(double x, double y, double side) : Line(x, y, side) {}
    string getName() const override { return "Квадрат"; }
    double getSize() const override { return length * length; }
    Figure* clone() const override { return new Square(*this); }
    void increaseSize(double delta) override {
        double newArea = max(0.0, getSize() + delta);
        length = sqrt(newArea);
    }
};

class Cube : public Square {
public:
    Cube(double x, double y, double edge) : Square(x, y, edge) {}
    string getName() const override { return "Куб"; }
    double getSize() const override { return pow(length, 3); }
    Figure* clone() const override { return new Cube(*this); }
    void increaseSize(double delta) override {
        double newVol = max(0.0, getSize() + delta);
        length = cbrt(newVol);
    }
};

template <typename T>
double getMetric(const T& key) { return (double)key; }

template <typename T>
T copyAndAdd(const T& key, int delta) { return key + delta; }

template <typename T>
T copyKey(const T& key) { return key; }

template <typename T>
void deleteKey(T& key) {}

double getMetric(const string& key) { return (double)key.length(); }

string copyAndAdd(const string& key, int delta) {
    if (delta >= 0) return key + string(delta, '_');
    int len = max(0, (int)key.length() + delta);
    return key.substr(0, len);
}

double getMetric(Figure* const& fig) { return fig->getSize(); }

Figure* copyAndAdd(Figure* const& fig, int delta) {
    Figure* c = fig->clone();
    c->increaseSize(delta);
    return c;
}

Figure* copyKey(Figure* const& fig) { return fig->clone(); }

void deleteKey(Figure*& fig) {
    delete fig;
    fig = nullptr;
}

template <typename T>
void printKey(const T& key) { cout << key << " "; }

void printKey(Figure* const& fig) {
    fig->print();
    cout << " | ";
}

template <typename T>
class AVLTree {
private:
    struct Node {
        T key;
        Node* left;
        Node* right;
        int height;

        Node(const T& k) : key(k), left(nullptr), right(nullptr), height(1) {}
    };

    Node* root;

    int getHeight(Node* n) const { return n ? n->height : 0; }
    int getBalance(Node* n) const { return n ? getHeight(n->left) - getHeight(n->right) : 0; }

    void updateHeight(Node* n) {
        if (n) n->height = 1 + max(getHeight(n->left), getHeight(n->right));
    }

    Node* rotateRight(Node* y) {
        Node* x = y->left;
        Node* T2 = x->right;
        x->right = y;
        y->left = T2;
        updateHeight(y);
        updateHeight(x);
        return x;
    }

    Node* rotateLeft(Node* x) {
        Node* y = x->right;
        Node* T2 = y->left;
        y->left = x;
        x->right = T2;
        updateHeight(x);
        updateHeight(y);
        return y;
    }

    Node* balanceNode(Node* node) {
        if (!node) return nullptr;
        updateHeight(node);
        int b = getBalance(node);

        if (b > 1) {
            if (getBalance(node->left) < 0) node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        if (b < -1) {
            if (getBalance(node->right) > 0) node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
        return node;
    }

    Node* insert(Node* node, T key) {
        if (!node) return new Node(key);

        if (getMetric(key) < getMetric(node->key)) {
            node->left = insert(node->left, key);
        }
        else if (getMetric(key) > getMetric(node->key)) {
            node->right = insert(node->right, key);
        }
        else {
            deleteKey(key);
            return node;
        }
        return balanceNode(node);
    }

    Node* clone(Node* node) const {
        if (!node) return nullptr;
        Node* n = new Node(copyKey(node->key));
        n->height = node->height;
        n->left = clone(node->left);
        n->right = clone(node->right);
        return n;
    }

    void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        deleteKey(node->key);
        delete node;
    }

    Node* getMinNode(Node* node) const {
        while (node && node->left) node = node->left;
        return node;
    }

    Node* getMaxNode(Node* node) const {
        while (node && node->right) node = node->right;
        return node;
    }

    Node* addValueToNodes(Node* node, int val) const {
        if (!node) return nullptr;
        Node* n = new Node(copyAndAdd(node->key, val));
        n->height = node->height;
        n->left = addValueToNodes(node->left, val);
        n->right = addValueToNodes(node->right, val);
        return n;
    }

    void traverseAndInsert(Node* node, AVLTree<T>& target) const {
        if (!node) return;
        target.insert(copyKey(node->key));
        traverseAndInsert(node->left, target);
        traverseAndInsert(node->right, target);
    }

    Node* removeMax(Node* node, Node*& maxNode) {
        if (!node->right) {
            maxNode = node;
            Node* l = node->left;
            maxNode->left = nullptr;
            maxNode->height = 1;
            return l;
        }
        node->right = removeMax(node->right, maxNode);
        return balanceNode(node);
    }

    Node* removeMin(Node* node, Node*& minNode) {
        if (!node->left) {
            minNode = node;
            Node* r = node->right;
            minNode->right = nullptr;
            minNode->height = 1;
            return r;
        }
        node->left = removeMin(node->left, minNode);
        return balanceNode(node);
    }

    Node* attachLeft(Node* current, Node* t1, Node* x, int targetH) {
        if (getHeight(current) <= targetH) {
            x->left = t1;
            x->right = current;
            return balanceNode(x);
        }
        current->left = attachLeft(current->left, t1, x, targetH);
        return balanceNode(current);
    }

    Node* attachRight(Node* current, Node* t2, Node* x, int targetH) {
        if (getHeight(current) <= targetH) {
            x->left = current;
            x->right = t2;
            return balanceNode(x);
        }
        current->right = attachRight(current->right, t2, x, targetH);
        return balanceNode(current);
    }

    Node* mergeSortedTrees(Node* t1, Node* t2) {
        if (!t1) return t2;
        if (!t2) return t1;

        if (getHeight(t1) <= getHeight(t2)) {
            Node* x = nullptr;
            t1 = removeMax(t1, x);
            return attachLeft(t2, t1, x, getHeight(t1));
        }
        else {
            Node* x = nullptr;
            t2 = removeMin(t2, x);
            return attachRight(t1, t2, x, getHeight(t2));
        }
    }

    void printInOrder(Node* node) const {
        if (!node) return;
        printInOrder(node->left);
        printKey(node->key);
        printInOrder(node->right);
    }

public:
    AVLTree() : root(nullptr) {}
    AVLTree(const AVLTree& other) { root = clone(other.root); }

    AVLTree& operator=(const AVLTree& other) {
        if (this != &other) {
            destroy(root);
            root = clone(other.root);
        }
        return *this;
    }

    ~AVLTree() { destroy(root); }

    void insert(T key) { root = insert(root, key); }

    void print() const {
        printInOrder(root);
        cout << endl;
    }

    AVLTree<T> operator+(int value) const {
        AVLTree<T> res;
        res.root = addValueToNodes(root, value);
        return res;
    }

    AVLTree<T> operator+(const AVLTree<T>& other) const {
        if (!root) return AVLTree<T>(other);
        if (!other.root) return AVLTree<T>(*this);

        Node* maxL = getMaxNode(root);
        Node* minR = getMinNode(other.root);

        if (getMetric(maxL->key) <= getMetric(minR->key)) {
            AVLTree<T> res;
            Node* t1 = clone(root);
            Node* t2 = clone(other.root);

            if (getMetric(maxL->key) == getMetric(minR->key)) {
                Node* dup = nullptr;
                t1 = res.removeMax(t1, dup);
                deleteKey(dup->key);
                delete dup;
            }

            res.root = res.mergeSortedTrees(t1, t2);
            return res;
        }
        else {
            AVLTree<T> res(*this);
            traverseAndInsert(other.root, res);
            return res;
        }
    }
};

int main() {
    setlocale(LC_ALL, "Russian");

    AVLTree<int> a, b, d;
    a.insert(10);
    a.insert(5);
    a.insert(15);

    b.insert(20);
    b.insert(30);

    d.insert(12);
    d.insert(15);
    d.insert(40);

    cout << "a: "; a.print();
    cout << "b: "; b.print();
    cout << "d: "; d.print();

    AVLTree<int> c1 = a + 100;
    cout << "a + 100: "; c1.print();

    AVLTree<int> c2 = a + b;
    cout << "a + b (Л+Р): "; c2.print();

}